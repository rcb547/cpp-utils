cmake_minimum_required(VERSION 3.12)

# Set name
project(cpp-utils VERSION 1.0 DESCRIPTION "Ross C. Brodie's C++ utilities" LANGUAGES CXX)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Configure find package
find_package(PkgConfig REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unused-result -Wno-format-security -Wno-date-time")
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3 -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-unused-result -Wno-format-security -Wno-date-time")

pkg_search_module(GDAL gdal IMPORTED_TARGET) 
if(GDAL_FOUND)
	message("GDAL was found")
	include_directories(PkgConfig::GDAL)
	link_libraries     (PkgConfig::GDAL)
	add_definitions(-DHAVE_GDAL)
endif()

find_package(CGAL)
if(CGAL_FOUND)
	message("CGAL was found")	
else()
	message(NOTICE "CGAL was NOT found")
endif()

set(sources ${sources} 
	src/file_utils.cpp
	src/general_utils.cpp
)

if(GDAL_FOUND)
	set(sources ${sources} src/gdal_utils.cpp)
endif()

if(CGAL_FOUND)
	set(sources ${sources} src/cgal_utils.cpp)
endif()

# Add the cpp-utils static and shared libraries
add_library(cpp-utils-shared SHARED ${sources} )
set_target_properties(cpp-utils-shared PROPERTIES OUTPUT_NAME cpp-utils)
target_include_directories(cpp-utils-shared PUBLIC src)

add_library(cpp-utils-static STATIC ${sources} )
set_target_properties(cpp-utils-static PROPERTIES OUTPUT_NAME cpp-utils)
target_include_directories(cpp-utils-static PUBLIC src)

set_target_properties(
  cpp-utils-shared cpp-utils-static
  PROPERTIES CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
)

if(CGAL_FOUND)
	target_include_directories(cpp-utils-shared PRIVATE CGAL::CGAL)
	target_link_libraries(cpp-utils-shared CGAL::CGAL)

	target_include_directories(cpp-utils-static PRIVATE CGAL::CGAL)
	target_link_libraries(cpp-utils-static CGAL::CGAL)
	add_definitions(-DHAVE_CGAL)
endif()

