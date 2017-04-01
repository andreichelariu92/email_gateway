#include "lua.h"
#include "lauxlib.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int x;
    int y;
} lua_point;

lua_point* lua_point_create(int x, int y)
{
    lua_point* output = NULL;
    
    output = malloc(sizeof(lua_point));
    if (output) {
        output->x = x;
        output->y = y;
    }
    
    return output;
}

void lua_point_delete(lua_point* p)
{
    free(p);
}

static int createPoint(lua_State* L)
{
    int x = lua_tointeger(L, 1);
    int y = lua_tointeger(L, 2);
    
    if (x == 0 || y == 0) {    
        return luaL_error(L, "Error creating point, invalid parameters");
    }
    
    lua_point* p = lua_point_create(x, y);
    lua_point** output = lua_newuserdata(L, sizeof(lua_point*));
    if (output != NULL && p != NULL) {
        *output = p;
        luaL_getmetatable(L, "test_luaPoint");
        lua_setmetatable(L, -2);
        printf("Success x=%d y=%d\n", p->x, p->y);
        return 1;
    }
    else {
        if (p) {
            lua_point_delete(p);
        }
        return luaL_error(L, "Failure allocating memory");
    }
}

static int getPointX(lua_State* L)
{
    lua_point** p = luaL_checkudata(L, 1, "test_luaPoint");
    
    if (p == NULL) {
        return luaL_error(L, "parameter is not a point");
    }
    
    lua_pushinteger(L, (*p)->x);
    return 1;
}

static int getPointY(lua_State* L)
{
    lua_point** p = luaL_checkudata(L, 1, "test_luaPoint");
    if (p == NULL) {
        return luaL_error(L, "parameter is not a point");
    }
    
    lua_pushinteger(L, (*p)->y);
    return 1;
}

static int pointGC(lua_State* L)
{
    lua_point** p = lua_touserdata(L, 1);
    lua_point_delete(*p);
    printf("Garbage Collector called\n");
    
    return 0;
}

static const struct luaL_Reg pointLib[] = {
    {"create", createPoint},
    {NULL, NULL}
};

static const struct luaL_Reg pointFunctions[] = {
    {"getX", getPointX},
    {"getY", getPointY},
    {NULL, NULL}
};

/*
int luaopen_lib_lua_email(lua_State* L)
{
    luaL_newmetatable(L, "test_luaPoint");
    
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    
    lua_pushstring(L, "__gc");
    lua_pushcfunction(L, pointGC);
    lua_settable(L, -3);
    
    luaL_setfuncs(L, pointFunctions, 0);
    luaL_newlib(L, pointLib);
    return 1;
}
*/