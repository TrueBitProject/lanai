cmake_minimum_required(VERSION 3.0.0)

project(lanai-int VERSION "0.0.1")

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall")

aux_source_directory(. SRC_LIST)
add_executable(lanai-int ${SRC_LIST})
