cmake_minimum_required(VERSION 3.0)

set(project spawn_test)

if (CMAKE_VERSION VERSION_LESS 3.0)
    PROJECT(${project} CXX)
else ()
    cmake_policy(SET CMP0048 NEW)
    PROJECT(${project} VERSION "${CMAKE_PROJECT_VERSION}" LANGUAGES CXX)
endif ()

set(SOURCE_FILES main.cpp)

message(STATUS "PROJECT_NAME = ${PROJECT_NAME}")

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} actor-zeta CONAN_PKG::catch2)

include(CTest)
include(Catch)
catch_discover_tests(${PROJECT_NAME})
