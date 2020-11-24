cmake_minimum_required(VERSION 3.10)

project(cpp-utils VERSION 1.0
DESCRIPTION "Ross C. Brodie's C++ utilities"
LANGUAGES CXX)

add_library(file_utils STATIC src/file_utils.cpp)
target_include_directories(file_utils PUBLIC src)
set_property(TARGET file_utils PROPERTY CXX_STANDARD 11)
set_property(TARGET file_utils PROPERTY CXX_STANDARD_REQUIRED ON)

add_library(general_utils STATIC src/general_utils.cpp)
target_include_directories(general_utils PUBLIC src)
set_property(TARGET general_utils PROPERTY CXX_STANDARD 11)
set_property(TARGET general_utils PROPERTY CXX_STANDARD_REQUIRED ON)
