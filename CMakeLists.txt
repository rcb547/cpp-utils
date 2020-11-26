cmake_minimum_required(VERSION 3.12)

project(cpp-utils VERSION 1.0
DESCRIPTION "Ross C. Brodie's C++ utilities"
LANGUAGES CXX)

add_library(file_utils OBJECT src/file_utils.cpp src/file_utils.h)
target_include_directories(file_utils PUBLIC src)

add_library(general_utils OBJECT src/general_utils.cpp src/general_utils.h)
target_include_directories(general_utils PUBLIC src)

set_target_properties(
  file_utils general_utils
  PROPERTIES CXX_STANDARD 11
  CXX_STANDARD_REQUIRED ON
)
