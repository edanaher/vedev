#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <sys/time.h>
#include <linux/input.h>

#include "vedve.h"

void send_key(int dev, int k, int state);

struct callback {
  long long time;
  int function_ref;
};

struct callback callbacks[1000];

long long now() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)tv.tv_sec * 1000000 + tv.tv_usec;
}

int lua_schedule(lua_State *L) {
  double offset = lua_tonumber(L, 1);
  int function_ref = luaL_ref(L, LUA_REGISTRYINDEX);
  int i;
  long long time = now() + 1000 * offset;
  for(i = 0; callbacks[i].time; i++);
  for(; i >= 0 && (!callbacks[i].time || callbacks[i].time > time); i--) {
    callbacks[i + 1].time = callbacks[i].time;
    callbacks[i + 1].function_ref = callbacks[i].function_ref;
  }
  callbacks[i + 1].time = time;
  callbacks[i + 1].function_ref = function_ref;
  return 0;
}

long long time_to_next_callback() {
  return callbacks[0].time ? callbacks[0].time - now() : 0;
}

void run_callbacks(lua_State *L) {
  int c;
  long long time = now();
  for(c = 0; callbacks[c].time && callbacks[c].time < time; c++) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, callbacks[c].function_ref);
    if(lua_pcall(L, 0, 0, 0)) {
      luaL_error(L, "Error in callback: %s", lua_tostring(L, -1));
      // longjmp away!
    }
    luaL_unref(L, LUA_REGISTRYINDEX, callbacks[c].function_ref);
  }
  if(c)
    for(int i = c; callbacks[i - 1].time; i++) {
      callbacks[i - c].time = callbacks[i].time;
      callbacks[i - c].function_ref = callbacks[i].function_ref;
    }
}

int lua_sendevent(lua_State *L) {
  int dev = lua_tointeger(L, lua_upvalueindex(1));
  struct input_event ev;
  ev.type = lua_tointeger(L, 1);
  ev.code = lua_tointeger(L, 2);
  ev.value = lua_tointeger(L, 3);
  send_key(dev, ev.code, ev.value);
  return 0;
}

struct { int k; char *sym; } keymapping[] =  {
  { KEY_W, "W"},
  { KEY_E, "E"},
  { KEY_J, "J"},
  { KEY_UP, "UP"},
  { KEY_DOWN, "DOWN"},
  { KEY_F6, "F6"},
  { KEY_F7, "F7"}
};

void setup_environment(lua_State *L, int dev) {
  callbacks[0].time = 0;
  lua_pushinteger(L, dev);
  lua_pushcclosure(L, lua_sendevent, 1);
  lua_setglobal(L, "send_event");
  lua_pushcfunction(L, lua_schedule);
  lua_setglobal(L, "schedule");
  lua_newtable(L);
  for(int i = 0; i < sizeof(keymapping) / sizeof(keymapping[0]); i++) {
    lua_pushinteger(L, keymapping[i].k);
    lua_setfield(L, -2, keymapping[i].sym);
  }
  lua_setglobal(L, "keys");
}

lua_State *load_config(char *filename, struct config *config, int dev) {
  lua_State *L = luaL_newstate();
  luaopen_base(L);
  luaopen_io(L);
  luaopen_string(L);
  luaopen_math(L);

  setup_environment(L, dev);

  if(luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, 0)) {
    luaL_error(L, "can't load configuration file:\n%s", lua_tostring(L, -1));
    // longjmp away
  }

  lua_getglobal(L, "name");
  if(!lua_isstring(L, -1)) {
    lua_pushstring(L, "`name' should be a string");
    lua_error(L);
  }
  config->name = (char *)lua_tostring(L, -1);

  config->L = L;

  return L;
}

double time_to_float(struct timeval *tv) {
  return tv->tv_sec + (double)tv->tv_usec / 1000000;
}

int get_key_config(struct config *config, struct input_event *ev) {
  lua_State *L = config->L;
  lua_getglobal(L, "keymap");
  if(!lua_istable(L, -1)) {
    lua_pushstring(L, "`keymap' should be a table");
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
    lua_pushnumber(L, time_to_float(&ev->time));
    lua_setfield(L, -2, "time");
    lua_call(L, 1, 1);
    int k = lua_tointeger(L, -1);
    lua_pop(L, 2);
    return k;
  }
  printf("Error: Key config must be a function\n");
  return -1;
}
