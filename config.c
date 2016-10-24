#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "vedve.h"

void load_config(char *filename, struct config *config) {
  lua_State *L = luaL_newstate();
  luaopen_base(L);
  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  if(luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)) {
    lua_pushstring(L, "can't load configuration file: %s");
    lua_error(L);
  }

  lua_getglobal(L, "name");
  if(!lua_isstring(L, -1)) {
    lua_pushstring(L, "`name' should be a string");
    lua_error(L);
  }
  config->name = (char *)lua_tostring(L, -1);
  lua_close(L);
}
