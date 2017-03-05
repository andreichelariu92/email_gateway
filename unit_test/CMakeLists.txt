cmake_minimum_required(VERSION 2.8.9)
file(GLOB UNIT_TEST_SOURCES "./src/*.c")
include_directories("../")
find_package(CURL)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    add_executable(unit_test ${UNIT_TEST_SOURCES})
    target_link_libraries(unit_test lib_util)
    target_link_libraries(unit_test lib_email)
    target_link_libraries(unit_test ${CURL_LIBRARIES})
ELSE(CURL_FOUND)
    MESSAGE(FATAL_ERROR "Could not find CURL library")
ENDIF(CURL_FOUND)

if(DEFINED RUN_UNIT_TEST)
    add_custom_command(TARGET unit_test
        POST_BUILD
        COMMAND "./unit_test")
endif(DEFINED RUN_UNIT_TEST)