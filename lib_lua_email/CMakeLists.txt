cmake_minimum_required(VERSION 2.8.9)

include_directories("../")

file(GLOB LIB_LUA_EMAIL_SOURCES "./src/*.c")
add_library(lib_lua_email SHARED ${LIB_LUA_EMAIL_SOURCES})
set_target_properties(lib_lua_email PROPERTIES PREFIX "")

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(lib_lua_email ${LUA_LIBRARIES})
link_directories(${LUA_LIBRARIES})