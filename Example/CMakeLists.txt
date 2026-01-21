cmake_minimum_required(VERSION 3.10)

project(Example)

include_directories(..)

add_executable(Example WIN32 main.cpp)
target_link_options(Example PRIVATE "/MANIFESTUAC:level='requireAdministrator' uiAccess='false'")
target_link_libraries(Example MemX)
