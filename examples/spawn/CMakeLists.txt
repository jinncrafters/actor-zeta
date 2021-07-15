cmake_minimum_required(VERSION 3.0)

project(spawn)

#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS_} -O0 -g -fno-omit-frame-pointer -fsanitize=thread") #address")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} actor-zeta testsuite)
