//#include "nu.h"
#include <Windows.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <assert.h>

//#define ZLIB_DLL
//#define ZLIB_WINAPI
//#pragma comment(lib,"libz.a")
#include <lua52\lua.hpp>

#include <zlib\zlib.h>


using namespace std;
#define CHUNK (1024* 1024)

// hate windows.  HATE it

/* report a zlib or i/o error */
void zerr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
    case Z_ERRNO:
        if (ferror(stdin))
            fputs("error reading stdin\n", stderr);
        if (ferror(stdout))
            fputs("error writing stdout\n", stderr);
        break;
    case Z_STREAM_ERROR:
        fputs("invalid compression level\n", stderr);
        break;
    case Z_DATA_ERROR:
        fputs("invalid or incomplete deflate data\n", stderr);
        break;
    case Z_MEM_ERROR:
        fputs("out of memory\n", stderr);
        break;
    case Z_VERSION_ERROR:
        fputs("zlib version mismatch!\n", stderr);
    }
}

const char* zerrstr(int ret)
{
    fputs("zpipe: ", stderr);
    switch (ret) {
	case Z_OK:				return "Z_OK: No error ";
	case Z_ERRNO:			return "Z_ERRNO: Error Reading ";
	case Z_STREAM_ERROR:	return "Z_STREAM_ERROR: Invalid Compression Level ";
	case Z_DATA_ERROR:		return "Z_DATA_ERROR: Invalid or Incomplete Deflate Data ";
	case Z_MEM_ERROR:		return "Z_MEM_ERROR: Memory Allocation Error ";
	case Z_VERSION_ERROR:	return "Z_VERSION_ERROR: Zlib Version Mismatch ";
	default:				return "LUA_ZLIB: Can't find Z error";
    }
}



#define ON_FALSE_ERROR(exp,msg) if(!(exp)) { errmsg = msg ? msg : zerrstr(ret); goto end_func; }
#define ON_TRUE_ERROR(exp,msg) if(exp)  { errmsg = msg ? msg : zerrstr(ret);  goto end_func; }
int lua_infgz(lua_State*L) 
{

	const char* errmsg = NULL;
	int ret=0;
	ON_FALSE_ERROR(lua_isstring(L,1), "No Incoming Data\nFormat: gunzip(string)")
	const char* compressed_data = lua_tostring(L,1);
	int compress_len = lua_rawlen(L,1);
	luaL_Buffer B;
	luaL_buffinit(L,&B);

	
   
    z_stream strm;
	//gz_header header;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = compress_len;
    strm.next_in = (Bytef*)compressed_data;
    ret = inflateInit2(&strm,(16+MAX_WBITS));
	ON_FALSE_ERROR(ret == Z_OK,strm.msg)
	//ret = inflateGetHeader(&strm,&header);
	//ON_FALSE_ERROR(ret == Z_OK,strm.msg)
	//assert(header.done == 1); // just to see if this ticks or no

    do // Trust zlib to get us out of here
	{
		char *buffer = luaL_prepbuffsize(&B, CHUNK);
		ON_FALSE_ERROR(buffer, "LUA: Could not alloc more memory ");
		strm.avail_out = CHUNK;
		strm.next_out = (Bytef*)buffer;
		ret = inflate(&strm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		ON_TRUE_ERROR(ret == Z_NEED_DICT,strm.msg)
		ON_TRUE_ERROR(ret == Z_DATA_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		luaL_addsize(&B, CHUNK - strm.avail_out);
     } while(ret != Z_STREAM_END || ret == Z_OK);

    /* clean up and return */
end_func:
	if(errmsg)  { 
		if(ret == Z_OK || ret == Z_MEM_ERROR) ret=inflateEnd(&strm);
		// clean up memory and push error
		luaL_pushresult(&B);
		lua_pop(L,1);
		lua_pushstring(L,errmsg);
		lua_error(L);
		return 1;
	}
//	ret=inflateEnd(&strm);
    luaL_pushresult(&B);
	return 1;
}
int lua_gzip(lua_State*L) 
{
	int ret;
	if(!lua_isstring(L,1))  luaL_error(L,"No Incoming Data\nFormat: gunzip(string)");
	const char* uncompressed_data = lua_tostring(L,1);
	int uncompressed_len = lua_rawlen(L,1);

	// Might be a bit big, but I got freaking 16 gigs of ram, whats another 3 megs?
	char* compressed_data = (char*)lua_newuserdata(L,sizeof(char) * uncompressed_len);
	if(!compressed_data) luaL_error(L,"Could not allocate data");

    z_stream strm;
	//gz_header header;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
	ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);  
	if(ret != Z_OK) luaL_error(L,"Could not init deflate");

	strm.avail_in	= uncompressed_len;
	strm.next_in	= (Bytef*)uncompressed_data;
	strm.avail_out	= uncompressed_len;
	strm.next_out	= (Bytef*)compressed_data;
	// We are compressing eveything in one go, if something breaks need ot check it
	ret = deflate(&strm,Z_FINISH);
	if(ret !=Z_STREAM_END) luaL_error(L,"lua_def: Something wrong with zlib");
	ret = deflateEnd(&strm);

    // Ok lets convert the userdata to a string
	lua_pushlstring(L,compressed_data,uncompressed_len - strm.avail_out);
	lua_remove(L,-2); // remove userdata and poof easy

	return 1;
}
int lua_gunzip(lua_State*L) 
{

	const char* errmsg = NULL;
	int ret=0;
	ON_FALSE_ERROR(lua_isstring(L,1), "No Incoming Data\nFormat: gunzip(string)")
	const char* compressed_data = lua_tostring(L,1);
	int compress_len = lua_rawlen(L,1);
	luaL_Buffer B;
	luaL_buffinit(L,&B);

    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = compress_len;
    strm.next_in = (Bytef*)compressed_data;
    ret = inflateInit2(&strm,16+MAX_WBITS);
	ON_FALSE_ERROR(ret == Z_OK,strm.msg)


    do // Trust zlib to get us out of here
	{
		char *buffer = luaL_prepbuffsize(&B, CHUNK);
		ON_FALSE_ERROR(buffer, "LUA: Could not alloc more memory ");
		strm.avail_out = CHUNK;
		strm.next_out = (Bytef*)buffer;
		ret = inflate(&strm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		ON_TRUE_ERROR(ret == Z_NEED_DICT,strm.msg)
		ON_TRUE_ERROR(ret == Z_DATA_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		luaL_addsize(&B, CHUNK - strm.avail_out);
     } while(ret != Z_STREAM_END || ret == Z_OK);

    /* clean up and return */
end_func:
	if(ret == Z_OK || ret == Z_MEM_ERROR) ret=inflateEnd(&strm);
	if(errmsg)  { 
		
		// clean up memory and push error
		luaL_pushresult(&B);
		lua_pop(L,1);
		luaL_error(L,"ZERROR: %s",errmsg);
		return 1;
	}
    luaL_pushresult(&B);
	return 1;
}
int lua_inf(lua_State*L) 
{

	const char* errmsg = NULL;
	int ret=0;
	ON_FALSE_ERROR(lua_isstring(L,1), "No Incoming Data\nFormat: gunzip(string)")
	const char* compressed_data = lua_tostring(L,1);
	int compress_len = lua_rawlen(L,1);
	luaL_Buffer B;
	luaL_buffinit(L,&B);

 
    z_stream strm;

    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = compress_len;
    strm.next_in = (Bytef*)compressed_data;
	// Quicky test to see if its a gz stream or not
	if(compressed_data[0] == '\037' && compressed_data[1] == '\213')
		ret = inflateInit2(&strm,16+MAX_WBITS);
	else
		ret = inflateInit(&strm);

	
	ON_FALSE_ERROR(ret == Z_OK,strm.msg)


    do // Trust zlib to get us out of here
	{
		char *buffer = luaL_prepbuffsize(&B, CHUNK);
		ON_FALSE_ERROR(buffer, "LUA: Could not alloc more memory ");
		strm.avail_out = CHUNK;
		strm.next_out = (Bytef*)buffer;
		ret = inflate(&strm, Z_NO_FLUSH);
		assert(ret != Z_STREAM_ERROR);  /* state not clobbered */
		ON_TRUE_ERROR(ret == Z_NEED_DICT,strm.msg)
		ON_TRUE_ERROR(ret == Z_DATA_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		ON_TRUE_ERROR(ret == Z_MEM_ERROR,strm.msg)
		luaL_addsize(&B, CHUNK - strm.avail_out);
     } while(ret != Z_STREAM_END || ret == Z_OK);

    /* clean up and return */
end_func:
	if(ret == Z_OK || ret == Z_MEM_ERROR) ret=inflateEnd(&strm);
	if(errmsg)  { 
		
		// clean up memory and push error
		luaL_pushresult(&B);
		lua_pop(L,1);
		luaL_error(L,"ZERROR: %s",errmsg);
		return 1;
	}
    luaL_pushresult(&B);
	return 1;
}

int lua_def(lua_State*L) 
{
	int ret;
	if(!lua_isstring(L,1))  luaL_error(L,"No Incoming Data\nFormat: gunzip(string)\n Use:  deflate(string[,level])");
	int level = lua_isnumber(L,2) ? lua_tointeger(L,2) : Z_DEFAULT_COMPRESSION;
	if(level < 0 || level > 9) luaL_error(L,"Invalid compression range.\n Use:  deflate(string[,level])");
	const char* uncompressed_data = lua_tostring(L,1);
	int uncompressed_len = lua_rawlen(L,1);

	// Might be a bit big, but I got freaking 16 gigs of ram, whats another 3 megs?
	char* compressed_data = (char*)lua_newuserdata(L,sizeof(char) * uncompressed_len);
	if(!compressed_data) luaL_error(L,"Could not allocate data");

    z_stream strm;
	//gz_header header;
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm,level); // make this chanagable
	if(ret != Z_OK) luaL_error(L,"Could not init deflate");

	strm.avail_in	= uncompressed_len;
	strm.next_in	= (Bytef*)uncompressed_data;
	strm.avail_out	= uncompressed_len;
	strm.next_out	= (Bytef*)compressed_data;
	// We are compressing eveything in one go, if something breaks need ot check it
	ret = deflate(&strm,Z_FINISH);
	if(ret !=Z_STREAM_END) luaL_error(L,"lua_def: Something wrong with zlib");
	ret = deflateEnd(&strm);

    // Ok lets convert the userdata to a string
	lua_pushlstring(L,compressed_data,uncompressed_len - strm.avail_out);
	lua_remove(L,-2); // remove userdata and poof easy

	return 1;
}