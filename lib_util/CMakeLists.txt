cmake_minimum_required(VERSION 2.8.9)

file(GLOB LIB_UTIL_SOURCES "./src/*.c")

add_library(lib_util SHARED ${LIB_UTIL_SOURCES})
set_target_properties(lib_util PROPERTIES PREFIX "")
