
#include <iostream>
using namespace std;

#include "nu.h"
#include <assert.h>



int luaopen_dkjson(lua_State* L);

int	lua_inf(lua_State*L);
int	lua_def(lua_State*L);
int	lua_gzip(lua_State*L);
int	lua_gunzip(lua_State*L);

const luaL_Reg zfuncs[] = {
    { "inflate", lua_inf },
	{ "deflate", lua_def },
	{ "gzinflate", lua_gzip },
	{ "gzdeflate", lua_gunzip },
	{ NULL, NULL }
};


int list_games(lua_State* L);
int openlib_zlib(lua_State*L)
{
	luaL_newlib(L,zfuncs);
	return 1;
}

extern "C" {
int luaopen_marshal(lua_State* L);
}
const luaL_Reg funcs[] = {
    { "GetTurn", get_turn_data },
	{ "GetList", list_games },
	{ NULL, NULL }
};
static void semidebug(lua_State*L,const char* msg) 
{
	int top = lua_gettop(L);
	int t = lua_type(L,-1);
	cout << "Top: " << top  << " ";
	switch(t) {
	case LUA_TSTRING: cout << "string: " << lua_tostring(L,-1); break;
	case LUA_TTABLE: cout << "table";
	case LUA_TNUMBER: cout << lua_tonumber(L,-1);
	default:
		cout << "Dont care: " << t;
	}
	cout << "  " << msg << endl;
}
static int lua_require(lua_State* L, const char* name)
{
	lua_pushglobaltable(L);
	lua_getfield(L, -1, "require");
	lua_pushstring(L, name);
	lua_call(L,1,1);
	lua_remove(L,-2); // GRRR global table
	return 0;
}
int inflate_json_totable(lua_State*L) 
{
	if(lua_gettop(L) != 1 && lua_type(L,1) != LUA_TSTRING)
	{ return luaL_error(L,"GZ to JSON error"); }

	//lua_pushvalue(L,lua_upvalueindex(2)); // index of the decompressor
	//lua_pushvalue(L,1); // copy
	//lua_call(L,1,1);
	lua_pushvalue(L,lua_upvalueindex(1)); // json table decode()
	lua_pushvalue(L,-2); // push the string again
	lua_call(L,1,1); // <-- takes a while, 
//	assert(lua_gettop(L) == 1);
	//assert(lua_type(L,-1) == LUA_TTABLE);
	return 1;
}

int lualoadlib(lua_State *L) {
	lua_newtable(L);
	luaL_requiref(L,"zlib",openlib_zlib,0);
	lua_setfield(L,-2,"zlib");
	luaL_requiref(L, "lpeg", luaopen_lpeg, 0);
	lua_setfield(L,-2,"lpeg");

	lua_require(L,"dkjson");
	lua_pushvalue(L,-1);
	lua_setfield(L,-2,"json");
	lua_getfield(L,-1,"decode");
	// make a gzjson to lua table function
	luaL_setfuncs(L,funcs,1);

	// Now we copy this closure to any function that needs to uncompress
	
	// testing.  rather have some kind of fast json serializer instead
	// but might just end up using json
	luaL_requiref(L,"marshal",luaopen_marshal,0);
	lua_setfield(L,-2,"marshal");
	
    return 1;
}
extern "C" {
#define DEBUG_STACK(msg) cout << msg << endl; stackdump_g(L);
int __declspec(dllexport) luaopen_PlanetNu(lua_State* L) {
	LoadLibrary("zlib1.dll");
	LoadLibrary("libcurl.dll");
	curl_global_init(CURL_GLOBAL_ALL);
	extract_dkjson(L); // if dkjson library dosn't exist, lets extract the version we do have
    luaL_requiref(L,"PlanetNu",lualoadlib,0);
    return 1;
}
}