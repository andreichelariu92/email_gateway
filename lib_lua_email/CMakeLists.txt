cmake_minimum_required(VERSION 2.8.9)

include_directories("../")

file(GLOB LIB_LUA_EMAIL_SOURCES "./src/*.c")
add_library(lib_lua_email SHARED ${LIB_LUA_EMAIL_SOURCES})
set_target_properties(lib_lua_email PROPERTIES PREFIX "")

find_package(Lua REQUIRED)
include_directories(${LUA_INCLUDE_DIR})
target_link_libraries(lib_lua_email ${LUA_LIBRARIES})
link_directories(${LUA_LIBRARIES})
target_link_libraries(lib_lua_email lib_email)

add_custom_command(TARGET lib_lua_email
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy 
        $<TARGET_FILE:lib_lua_email> 
        "${CMAKE_CURRENT_SOURCE_DIR}/test/"
)

add_custom_command(TARGET lib_lua_email
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    $<TARGET_FILE:lib_lua_email>
    "${CMAKE_CURRENT_SOURCE_DIR}/../app/"
)
    
if(DEFINED RUN_LUA_UNIT_TEST AND NOT DEFINED RUN_UNIT_TEST)
    add_custom_command(TARGET lib_lua_email
        POST_BUILD
        COMMAND "lua5.3"
        ARGS "${CMAKE_CURRENT_SOURCE_DIR}/test/test.lua"
    )
endif(DEFINED RUN_LUA_UNIT_TEST AND NOT DEFINED RUN_UNIT_TEST)