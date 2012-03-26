
#include "nu.h"
#include "dkjson.h"

#include <signal.h>
#include <fstream>
#include <iostream>
using namespace std;
static lua_State * globalL;

static const char *progname = "PlanetNu";

static void l_message (const char *pname, const char *msg) {
  if (pname) luai_writestringerror("%s: ", pname);
  luai_writestringerror("%s\n", msg);
}

static int report (lua_State *L, int status) {
  if (status != LUA_OK && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
    /* force a complete garbage collection in case of errors */
    lua_gc(L, LUA_GCCOLLECT, 0);
  }
  return status;
}

/* the next function is called unprotected, so it must avoid errors */
static void finalreport (lua_State *L, int status) {
  if (status != LUA_OK) {
    const char *msg = (lua_type(L, -1) == LUA_TSTRING) ? lua_tostring(L, -1)
                                                       : NULL;
    if (msg == NULL) msg = "(error object is not a string)";
    l_message(progname, msg);
    lua_pop(L, 1);
  }
}
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);
  luaL_error(L, "interrupted!");
}
static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens before lstop,
                              terminate process (default action) */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}

static int traceback (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  if (msg)
    luaL_traceback(L, L, msg, 1);
  else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
    if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
      lua_pushliteral(L, "(no error message)");
  }
  return 1;
}

int docall (lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
  //globalL = L;  /* to be available to 'laction' */
  signal(SIGINT, laction);
  status = lua_pcall(L, narg, nres, base);
  signal(SIGINT, SIG_DFL);
  lua_remove(L, base);  /* remove traceback function */
  return status;
}

int dofile (lua_State *L, const char *name) {
  int status = luaL_loadfile(L, name);
  if (status == LUA_OK) status = docall(L, 0, 0);
  return report(L, status);
}


int dostring (lua_State *L, const char *s, const char *name) {
  int status = luaL_loadbuffer(L, s, strlen(s), name);
  if (status == LUA_OK) status = docall(L, 0, 0);
  return report(L, status);
}


int dolibrary (lua_State *L, const char *name) {
  int status;
  lua_pushglobaltable(L);
  lua_getfield(L, -1, "require");
  lua_pushstring(L, name);
  status = docall(L, 1, 1);
  if (status == LUA_OK) {
    lua_setfield(L, -2, name);  /* global[name] = require return */
    lua_pop(L, 1);  /* remove global table */
  }
  else
    lua_remove(L, -2);  /* remove global table (below error msg.) */
  return report(L, status);
}

void stackdump_g(lua_State* l)
{
    int i;
    int top = lua_gettop(l);
	
    printf("total in stack %d\n",top);
 
    for (i = 1; i <= top; i++)
    {  /* repeat for each level */
        int t = lua_type(l, i);
        switch (t) {
            case LUA_TSTRING:  /* strings */
                printf("string: '%s'\n", lua_tostring(l, i));
                break;
            case LUA_TBOOLEAN:  /* booleans */
                printf("boolean %s\n",lua_toboolean(l, i) ? "true" : "false");
                break;
            case LUA_TNUMBER:  /* numbers */
                printf("number: %g\n", lua_tonumber(l, i));
                break;
            default:  /* other values */
                printf("%s\n", lua_typename(l, t));
                break;
        }
        printf("  ");  /* put a separator */
    }
    printf("\n");  /* end the listing */
}

int extract_dkjson(lua_State* L) 
{
	if(!fexist("dkjson.lua")) {
		// dkjson dosn't exist, so pull it from memory and save it
		lua_pushcclosure(L,lua_infgz,0);
		lua_pushlstring(L,(const char*)dkjson_gz,sizeof(dkjson_gz));
		lua_call(L,1,1);
		ofstream f;
		f.open("dkjson.lua", ios::out);
		f << lua_tostring(L,-1);
		f.close();
		lua_pop(L,3);
	}

	return 1;
}

