#project(lib_util)
#set(CMAKE_BUILD_TYPE Release)

#include_directories(./src)
#file(GLOB SOURCES "src/lib_util/*.c")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib_util")
#add_library(lib_util STATIC ${SOURCES})
#set_target_properties(lib_util PROPERTIES PREFIX "")

#########Test purposes

#general project settings
cmake_minimum_required(VERSION 2.8.9)
project(email_gateway)
set(CMAKE_BUILD_TYPE Debug)
include_directories("./src")

#lib_util settings
file(GLOB LIB_UTIL_SOURCES "./src/lib_util/*.c")

add_library(lib_util STATIC ${LIB_UTIL_SOURCES})
set_target_properties(lib_util PROPERTIES PREFIX "")

#test settings
file(GLOB TEST_SOURCES "./src/test/*.c")
add_executable(test ${TEST_SOURCES} "./src/app/Email.c" "./src/app/SmtpConnection.c" "./src/app/ImapConnection.c")
target_link_libraries(test lib_util)
find_package(CURL)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(test ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
    MESSAGE(FATAL_ERROR "Could not find CURL library")
ENDIF(CURL_FOUND)

#app settings
file(GLOB APP_SOURCES "./src/app/*.c")
find_package(CURL)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    add_executable(app ${APP_SOURCES})
    target_link_libraries(app lib_util)
    target_link_libraries(app ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
    MESSAGE(FATAL_ERROR "Could not find CURL library")
ENDIF(CURL_FOUND)
