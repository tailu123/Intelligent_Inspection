cmake_minimum_required(VERSION 3.10)

# RapidXML是头文件库，不需要编译
add_library(rapidxml INTERFACE)
target_include_directories(rapidxml INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})