cmake_minimum_required(VERSION 2.8.9)
include_directories("../")

file(GLOB LIB_EMAIL_SOURCES "./src/*.c")
add_library(lib_email SHARED ${LIB_EMAIL_SOURCES})
set_target_properties(lib_email PROPERTIES PREFIX "")
find_package(CURL)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(lib_email ${CURL_LIBRARIES})
    target_link_libraries(lib_email lib_util)
ELSE(CURL_FOUND)
    MESSAGE(FATAL_ERROR "Could not find CURL library")
ENDIF(CURL_FOUND)
