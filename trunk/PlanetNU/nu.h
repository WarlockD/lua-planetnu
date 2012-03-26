#pragma once
#pragma comment(lib,"wsock32.lib")
/*
#pragma comment(lib,"libcurl.a")
#pragma comment(lib, "libz.a")
#pragma comment(lib, "librtmp.a")
#pragma comment(lib, "libcurl.a")
#pragma comment(lib, "libidn.a")
#pragma comment(lib, "libtmp.a")

//#pragma comment(lib,"libssl32.a")
//#pragma comment(lib,"libssh2dll.a")
*/

#define CURL_STATICLIB
#include <curl\curl.h>

extern "C" {


#include <lua52\lua.h>
#include <lua52\lualib.h>
#include <lua52\lauxlib.h>
}


#include <vector>
#include <string>


#define PLANETNU_HTTP_API "http://api.planets.nu"
#define URL_COMPLETED(g,p) PLANETNU_HTTP_API << "/game/loadturn?gameid=" << (unsigned)g << "&playerid=" << (unsigned)p
#define URL_HISTORICAL(g,p,t) URL_COMPLETED(g,p) << "&turn=" << (unsigned)t;
#define FILENAME_COMPLETED(g,p) "PlanetNu_Game(" << (unsigned)g << ")_Player(" << (unsigned)p << ").z"
#define FILENAME_HISTORICAL(g,p,t) "PlanetNu_Game(" << (unsigned)g << ")_Player(" << (unsigned)p << ")_Turn(" << (unsigned)t << ").z"

#define URL_LOADGAMEINFO(g) PLANETNU_HTTP_API << "/game/loadinfo?gameid=" << (unsigned)g
#define FILENAME_LOADGMAEINFO(g) "PlanetNu_GameInfo(" << (unsigned)g << ").z"

#define URL_LISTGAMES PLANETNU_HTTP_API << "/games/list" 
#define LISTGAME_FIELD_STATUS(g) "status=" << (unsigned)g
#define LISTGAME_FIELD_TYPE(g) "type=" << (unsigned)g
#define LISTGAME_FIELD_SCOPE(g) "scope=" << (unsigned)g
#define LISTGAME_FIELD_USERNAME(g) "username=" << (unsigned)g

int get_turn_data(lua_State* L);
int def(const std::vector<char>& source,std::vector<char>& dest,int level);

int inf(const std::string& filename,std::vector<char>& dest);

int infgz(const std::string& filename,std::vector<char>& dest);
int lua_infgz(lua_State*L);
int extract_dkjson(lua_State* L);

int dolibrary (lua_State *L, const char *name); 
int dostring (lua_State *L, const char *s, const char *name); 
int dofile (lua_State *L, const char *name);
int docall (lua_State *L, int narg, int nres); 

int fexist(const std::string& filename);
void stackdump_g(lua_State* l);
extern "C" { int luaopen_lpeg (lua_State *L); }
