#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <linux/input.h>

#include "vedve.h"

void send_key(int dev, int k, int state);

int lua_sendevent(lua_State *L) {
  int dev = lua_tointeger(L, lua_upvalueindex(1));
  struct input_event ev;
  ev.type = lua_tointeger(L, 1);
  ev.code = lua_tointeger(L, 2);
  ev.value = lua_tointeger(L, 3);
  send_key(dev, ev.code, ev.value);
  return 0;
}

void setup_environment(lua_State *L, int dev) {
  lua_pushinteger(L, dev);
  lua_pushcclosure(L, lua_sendevent, 1);
  lua_setglobal(L, "send_event");
}

lua_State *load_config(char *filename, struct config *config, int dev) {
  lua_State *L = luaL_newstate();
  luaopen_base(L);
  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  setup_environment(L, dev);

  if(luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)) {
    char buffer[1000];
    snprintf(buffer, 999, "can't load configuration file:\n%s", lua_tostring(L, -1));
    lua_pushstring(L, buffer);
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

int get_key_config(struct config *config, struct input_event *ev) {
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
    return -1;
  }
  /*if(lua_isstring(L, -1)) {
    char *str = (char *)lua_tostring(L, -1);
    lua_pop(L, 2);
    return str;
  }*/
  if(lua_isfunction(L, -1)) {
    lua_newtable(L);
    lua_pushinteger(L, ev->code);
    lua_setfield(L, -2, "code");
    lua_pushinteger(L, ev->type);
    lua_setfield(L, -2, "type");
    lua_pushinteger(L, ev->value);
    lua_setfield(L, -2, "value");
    lua_call(L, 1, 1);
    int k = lua_tointeger(L, -1);
    lua_pop(L, 2);
    return k;
  }
  printf("Error: Key config must be a function\n");
  return -1;
}
