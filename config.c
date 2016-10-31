#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <linux/input.h>

#include "vedve.h"

lua_State *load_config(char *filename, struct config *config) {
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

  lua_getglobal(L, "f");
  if(!lua_isfunction(L, -1)) {
    printf("Lua: `f' should be a function");
  } else {
    if(lua_pcall(L, 0, 1, 0)) {
      const char *err = lua_tostring(L, -1);
      printf("Error calling f: %s\n", err);
    } else {
      const char *res = lua_tostring(L, -1);
      printf("Got result: %s\n", res);
    }
  }
  config->L = L;

  return L;
}

char *get_key_config(struct config *config, struct input_event *ev) {
  lua_State *L = config->L;
  lua_getglobal(L, "keys");
  if(!lua_istable(L, -1)) {
    lua_pushstring(L, "`keys' should be a table");
    lua_error(L);
  }
  lua_pushnumber(L, ev->code);
  lua_gettable(L, -2);
  if(lua_isnil(L, -1)) {
    lua_pop(L, 2);
    return NULL;
  }
  if(lua_isstring(L, -1)) {
    char *str = (char *)lua_tostring(L, -1);
    lua_pop(L, 2);
    return str;
  }
  printf("Error: Key config must be a string\n");
  return NULL;
}
