#include "nu.h"



#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <exception>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>
#include <map>


#ifndef S_ISDIR
#define S_ISDIR(mode)  (((mode) & S_IFMT) == S_IFDIR)
#endif

#ifndef S_ISREG
#define S_ISREG(mode)  (((mode) & S_IFMT) == S_IFREG)
#endif

int lua_def(lua_State* L);
int lua_inf(lua_State* L);
int lua_gzip(lua_State*L);

using namespace std;


static const int BUFFER_SIZE = (256 * 1024);

int fexist(const string& filename) {
  struct stat fileAtt ;  
  if ( stat( filename.c_str(), &fileAtt ) !=0 ) return 0;
  return 1;
}
// Just to make sure we don't have any free roaming nulls in there
static string safe_null_string(const vector<char>& vec) {
	for(vector<char>::const_iterator it = vec.begin(); it != vec.end(); it++) 
		if(*it == '\0') return string(vec.begin(),it -1);
	return string(vec.begin(),vec.end());
}
static int isnewlinechar(int c) { return c == '\r' || c == '\n' ? 1 : 0; }  

static string safe_null_string(const char* str, int len) {
	int i=0; 
	for(i=0;str[i] != '\0' && i < len; i++); 
	if(i != len) len = i -1;
	return string(str,len);
}


extern "C" 
{
     static size_t HeaderCallback( void *ptr, size_t size, size_t nmemb, void *userdata) {
            size_t realsize = size*nmemb;
            vector<string>* data = static_cast<vector<string>*>(userdata);
			data->push_back(string((char*)ptr,realsize));
            return realsize;
        }
        static size_t BodyCallback( void *ptr, size_t size, size_t nmemb, void *userdata) {
            size_t realsize = size*nmemb;
            stringstream* data = static_cast<stringstream*>(userdata);
			data->write((char*)ptr,realsize);
            return realsize;
        }
}
const string& html_content_type_str = "text/html";
const string& gzip_content_type_str = "gzip";

#define ON_Z_RETURN(exp) if((ret = exp) != Z_OK) goto func_return;
class EasyBinaryGet
{
    private:
		static const int CHUNK = 128 * 1024;
		vector<string> _headers;
		stringstream _raw;
		string json;
		long _contentSize;
		string _contentType;
		bool _isjavascript;
		string _url;
    public:
		void PrintDebug(lua_State* L) 
		{
			cout << "URL: " << _url << endl;
			for(vector<string>::iterator it=_headers.begin();
				it != _headers.end(); it++)
				cout << *it ;
				cout << _raw.str();
			cout << endl << endl;
		}
		bool isJavaScript() { return _isjavascript; }
		int ContentSize() { return _contentSize; }
		void PushBody(lua_State *L) { 
			const string& s = _raw.str();
			lua_pushlstring(L,s.c_str(),s.length());
		}
        void HttpGet(const string& url)
        {
			_url = url;
			cout << "Downloading from planet.nu" << endl;
			int res;
            CURL *curl_handle;
            /* init the curl session */ 
          //  CURLE_OK
            curl_handle = curl_easy_init();
            /* set URL to get */ 
            curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
            /* no progress meter please */ 
            curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);
           // curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 1L);
            /* send all data to this function  */ 
            curl_easy_setopt(curl_handle, CURLOPT_ENCODING , "gzip");
            curl_easy_setopt(curl_handle,CURLOPT_HTTP_CONTENT_DECODING , 1);

            //curl_easy_setopt(curl_handle,CURLOPT_ENCODING, 1);
            curl_easy_setopt(curl_handle,   CURLOPT_WRITEDATA, &_raw);
            curl_easy_setopt(curl_handle,   CURLOPT_WRITEFUNCTION, BodyCallback);
            curl_easy_setopt(curl_handle,   CURLOPT_WRITEHEADER, &_headers);
            curl_easy_setopt(curl_handle,   CURLOPT_HEADERFUNCTION, HeaderCallback);
            
            
            cout << "Ok We have it setup, about to do a get" << endl;
            int result = curl_easy_perform(curl_handle); 
			char *ct; double _size;
			result = curl_easy_getinfo(curl_handle,CURLINFO_CONTENT_TYPE,&ct);
			if((CURLE_OK == result) && ct)
				_contentType = ct;

			result = curl_easy_getinfo(curl_handle,CURLINFO_CONTENT_LENGTH_DOWNLOAD,&_size);
			if((CURLE_OK == result) && _size != -1)
				_contentSize = (long)_size;
			_isjavascript = !_contentType.compare("text/javascript");
            /* cleanup curl stuff */ 
            curl_easy_cleanup(curl_handle);
        }

};

void lua_loadfile(lua_State* L, const char* filename)
{
	 ifstream f;
     f.open(filename,ios::in | ios::binary | ios::ate);
	 if(!f.is_open()) { luaL_error(L,"Cannot open turn file: %s");  }
	 long filesize = f.tellg();
	 f.seekg(0,ios::beg);
	 char* buffer = (char*)lua_newuserdata(L,sizeof(char) * filesize);
	 f.read(buffer,filesize);
	 lua_pushlstring(L,buffer,filesize);
	 lua_remove(L,-2);
}
void lua_savefile(lua_State* L, const char* filename)
{
	 ofstream f;
     f.open(filename,ios::out | ios::binary);
	 if(!f.is_open()) { luaL_error(L,"Cannot open turn file: %s");  }
	 int size = lua_rawlen(L,-1);
	 const char* data = lua_tostring(L,-1);
	 f.write(data,size);
}
#define CHECK_KEY(key) if(!strcmp(lua_tostring(L,-2),key) && lua_isnumber(L,-1)) ADD_URL(key,lua_tonumber(L,-1))
#define ADD_URL(key,value) \
	{ if(start_mark) { url << '?'; start_mark = false; } else url << '&'; \
	url << key << '=' << value; }

int list_games(lua_State* L)
{
	EasyBinaryGet ebg;
	bool start_mark= true;
	stringstream url;
	int t,i;
	int top = lua_gettop(L);
	url << URL_LISTGAMES; 
	switch(top)
	{
		case 0: break;
		case 1: // ok, string for username, or table
			switch(lua_type(L,1)) 
			{
				case LUA_TTABLE: 
					// Because we might not be in a protected enviroment, going to check all the keys
					// this can be REALLY slow if the user sends a bad table or to big of one
					lua_pushnil(L);
					while(lua_next(L,1) !=0) {
						if(lua_isstring(L,-2))
						{
							CHECK_KEY("scope")
							CHECK_KEY("username")
							CHECK_KEY("type")
							CHECK_KEY("status")
						}
						lua_pop(L,1);
					}
					lua_pop(L,1);
					break;
				case LUA_TSTRING: 
					url << URL_LISTGAMES << '?' << LISTGAME_FIELD_USERNAME(lua_tostring(L,1)); 
					break;
				}
			break;
		default:
			luaL_error(L,"Fix latter, don't support more than one or none args");
	}
    ebg.HttpGet(url.str());
	if(ebg.isJavaScript()) 
	{
		lua_pushvalue(L,lua_upvalueindex(1));
		ebg.PushBody(L);
		lua_call(L,1,1);
	} else 
	{ 
		cout << "-------------HTTP GET ERROR---------------" << endl;
		ebg.PrintDebug(L);
		luaL_error(L,"Expecting GZIP Data");
	}
	return 1;
}
int get_turn_data(lua_State* L) //, int gameid , int playerid)
{
	int top = lua_gettop(L);
    unsigned gameid =   luaL_checkunsigned(L,1);        
    unsigned playerid = luaL_checkunsigned(L,2);  
    unsigned turn = luaL_optunsigned(L,3,0);
    
    string url;
    string savefile;
	stringstream s;
    if(top == 3) {
        s << FILENAME_HISTORICAL(gameid,playerid,turn);
        savefile = s.str();
		 s.str("");
         s << URL_HISTORICAL(gameid,playerid,turn);
		 url = s.str();
	} else {
        s << FILENAME_COMPLETED(gameid,playerid);
        savefile = s.str();
		s.str("");
        s << URL_COMPLETED(gameid,playerid);
		url = s.str();
	}
	if(!fexist(savefile)) 
	{
		EasyBinaryGet ebg;
        ebg.HttpGet(url);
		if(ebg.isJavaScript()) 
		{
			lua_pushvalue(L,lua_upvalueindex(1));
			ebg.PushBody(L);
			lua_pushcclosure(L,lua_gzip,0);
			lua_pushvalue(L,-2);
			lua_call(L,1,1);
			lua_savefile(L,savefile.c_str());
			lua_pop(L,1);
			lua_call(L,1,1);
		}
		else 
		{ 
			cout << "-------------HTTP GET ERROR---------------" << endl;
			ebg.PrintDebug(L);
			luaL_error(L,"Expecting GZIP Data");
		 }
     } else {
		lua_pushvalue(L,lua_upvalueindex(1));
		lua_pushcclosure(L,lua_inf,0);
		//lua_pushvalue(L,lua_upvalueindex(1));
		lua_loadfile(L,savefile.c_str());
		lua_call(L,1,1);
		lua_call(L,1,1);

	}
    return 1;
}
  