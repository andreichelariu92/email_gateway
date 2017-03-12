#include "lua.h"
#include "lauxlib.h"

#include <stdio.h>

typedef struct {
    int x;
    int y;
} lua_point;

static int createPoint(lua_State* L)
{
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    
    if (x == 0 || y == 0) {    
        return luaL_error(L, "Error creating point, invalid parameters");
    }
    
    lua_point* p = lua_newuserdata(L, sizeof(lua_point));
    if (p  != NULL) {
        p->x = x;
        p->y = y;
        luaL_getmetatable(L, "test_luaPoint");
        lua_setmetatable(L, -2);
        printf("Success x=%d y=%d\n", p->x, p->y);
        return 1;
    }
    else {
        return luaL_error(L, "Failure allocating memory");
    }
}

static int getPointX(lua_State* L)
{
    lua_point* p = luaL_checkudata(L, 1, "test_luaPoint");
    
    if (p == NULL) {
        return luaL_error(L, "parameter is not a point");
    }
    
    lua_pushinteger(L, p->x);
    return 1;
}

static int getPointY(lua_State* L)
{
    lua_point* p = luaL_checkudata(L, 1, "test_luaPoint");
    if (p == NULL) {
        return luaL_error(L, "parameter is not a point");
    }
    
    lua_pushinteger(L, p->y);
    return 1;
}

static const struct luaL_Reg pointLib[] = {
    {"create", createPoint},
    {"getX", getPointX},
    {"getY", getPointY},
    {NULL, NULL}
};

int luaopen_lib_lua_email(lua_State* L)
{
    luaL_newmetatable(L, "test_luaPoint");
    luaL_newlib(L, pointLib);
    return 1;
}