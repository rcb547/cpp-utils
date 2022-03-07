cmake_minimum_required(VERSION 3.12)

# Set name
project(cpp-utils VERSION 1.0 DESCRIPTION "Ross C. Brodie's C++ utilities" LANGUAGES CXX)
message(STATUS "Configuring cpp-utils")

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Configure find package
find_package(PkgConfig REQUIRED)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-format-security")
SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3 -Wno-unknown-pragmas -Wno-unused-variable -Wno-unused-but-set-variable -Wno-sign-compare -Wno-format-security")
if(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Wno-date-time")
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wno-unused-result -Wno-date-time")
	#On GCC, even with -Wno-date-time, still get warings of the form: warning: macro "__DATE__" might prevent reproducible builds [-Wdate-time]
endif()

pkg_search_module(GDAL gdal IMPORTED_TARGET) 
if(GDAL_FOUND)
	message(STATUS "GDAL ${GDAL_VERSION} was found")
	include_directories(PkgConfig::GDAL)
	link_libraries     (PkgConfig::GDAL)
	add_definitions(-DHAVE_GDAL)
endif()

find_package(CGAL QUIET)
if(CGAL_FOUND)
	message(STATUS "CGAL ${CGAL_VERSION} was found")
else()
	message(WARNING "CGAL was NOT found -- src/cgal_utils.cpp will not be used")
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

