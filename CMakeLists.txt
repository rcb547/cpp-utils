cmake_minimum_required(VERSION 3.16)
cmake_policy(SET CMP0076 NEW)
SET(CMAKE_COLOR_MAKEFILE ON)

function(reportvar var)
  message(STATUS "${ARGVN}${var}=${${var}}")
endfunction()

function(reportallvars)
	get_cmake_property(_variableNames VARIABLES)
	list (SORT _variableNames)
	foreach (_variableName ${_variableNames})
		message(STATUS "${_variableName}=${${_variableName}}")
	endforeach()
endfunction()

# Set project name
project(cpp-utils VERSION 1.0 DESCRIPTION "Ross C. Brodie's C++ utilities" LANGUAGES CXX)
message(STATUS "Configuring cpp-utils")

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

option(WITH_MPI "WITH_MPI" ON)
option(WITH_GDAL "WITH_GDAL" ON)
option(WITH_CGAL "WITH_CGAL" ON)

reportvar(WITH_MPI)
reportvar(WITH_GDAL)
reportvar(WITH_CGAL)

# Set the C++ standard to C++17
set(CMAKE_CXX_STANDARD 17)

# Set the compile flags
if("${CMAKE_C_COMPILER_ID}" MATCHES "MSVC" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2")
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   /O2")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
else()
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wno-unused-but-set-variable -Wno-sign-compare -Wno-format-security -Wno-tautological-constant-compare ")
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -O3 -Wno-unused-but-set-variable -Wno-sign-compare -Wno-format-security -Wno-tautological-constant-compare ")
	# Adding filesystem library
	link_libraries(-lstdc++fs)
endif()

if(CMAKE_COMPILER_IS_GNUCC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-result -Wno-date-time -Wno-error=date-time ")
	SET(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS}   -Wno-unused-result -Wno-date-time -Wno-error=date-time ")
	#On GCC, even with -Wno-date-time, still get warings of the form: warning: macro "__DATE__" might prevent reproducible builds [-Wdate-time]	
endif()

# Find PkgConfig
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
	message(STATUS "PkgConfig package was found")
else()
	message(STATUS "PkgConfig package was NOT found")
endif()

# Configure MPI if opted for
if(${WITH_MPI})
	message(STATUS "\n\nChecking for MPI")
	find_package(MPI)
	if(MPI_FOUND)
		message(STATUS "MPI was found")
	endif()
endif()

#If BOOST_DIR is defined and not BOOST_ROOT, set it to BOOST_ROOT to BOOST_DIR
if(NOT DEFINED ENV{BOOST_ROOT})
	if(DEFINED ENV{BOOST_DIR})
		set(BOOST_ROOT $ENV{BOOST_DIR})
		reportvar(BOOST_ROOT)
	endif()
endif()

# Configure CGAL if opted for
if(${WITH_CGAL})
	message(STATUS "\n\nChecking for CGAL")
	#GMP is not necessary
	set(CGAL_DISABLE_GMP ON)
	find_package(CGAL REQUIRED COMPONENTS Core)
	if(CGAL_FOUND)
		message(STATUS "CGAL ${CGAL_VERSION} was found")
		reportvar(CGAL_INCLUDE_DIRS "	")
		reportvar(CGAL_LIBRARIES "	")
	else()
		message(WARNING "CGAL was NOT found -- cgal_utils.cpp will not be used")
	endif()
endif()

# Configure GDAL if opted for
if(${WITH_GDAL})
	message(STATUS "\n\nChecking for GDAL")
	find_package(GDAL REQUIRED)
	if(GDAL_FOUND)
		message(STATUS "GDAL ${GDAL_VERSION} was found")
		reportvar(GDAL_INCLUDE_DIRS "	")
		reportvar(GDAL_LIBRARIES "	")
	else()
		message(WARNING "GDAL was NOT found -- gdal_utils.cpp will not be used")
	endif()
endif()

set(sources ${sources} 
	src/file_utils.cpp
	src/general_utils.cpp
	src/RamerDouglasPeucker.cpp
)

if(${WITH_GDAL} AND GDAL_FOUND)
	set(sources ${sources} src/gdal_utils.cpp)
	add_definitions(-DHAVE_GDAL)
endif()

if(${WITH_CGAL} AND CGAL_FOUND)
	set(sources ${sources} src/cgal_utils.cpp)
	add_definitions(-DHAVE_CGAL)
endif()

# Add the cpp-utils static library
set(target cpp-utils-static)
add_library(${target} STATIC ${sources} )
#set_target_properties(${target} PROPERTIES OUTPUT_NAME cpp-utils)
set_target_properties(${target} PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(${target} PRIVATE src/)
target_include_directories(${target} INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>")
if(GDAL_FOUND)
	target_include_directories(${target} PRIVATE GDAL::GDAL)
	target_link_libraries(${target} PRIVATE GDAL::GDAL)
endif()
if(CGAL_FOUND)
	target_include_directories(${target} PRIVATE CGAL::CGAL)
	target_link_libraries(${target} PRIVATE CGAL::CGAL)
endif()
if(${WITH_MPI})
	target_compile_definitions(${target} PRIVATE -D_MPI_ENABLED -DOMPI_SKIP_MPICXX)
	target_include_directories(${target} PRIVATE SYSTEM ${MPI_INCLUDE_PATH})
	target_link_libraries(${target} PRIVATE ${MPI_C_LIBRARIES})
endif()
#install(TARGETS ${target} ARCHIVE EXCLUDE_FROM_ALL)
add_library(cpp-utils::static ALIAS cpp-utils-static)

### # Add the cpp-utils shared library
### set(target cpp-utils-shared)
### add_library(${target} SHARED ${sources} )
### #set_target_properties(${target} PROPERTIES OUTPUT_NAME cpp-utils)
### target_include_directories(${target} PRIVATE src/)
### target_include_directories(${target} INTERFACE "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/>")
### if(GDAL_FOUND)
### 	target_include_directories(${target} PRIVATE GDAL::GDAL)
### 	target_link_libraries(${target} PRIVATE GDAL::GDAL)
### endif()
### if(CGAL_FOUND)
### 	target_include_directories(${target} PRIVATE CGAL::CGAL)
### 	target_link_libraries(${target} PRIVATE CGAL::CGAL)
### endif()
### if(${WITH_MPI})
### 	target_compile_definitions(${target} PRIVATE -D_MPI_ENABLED -DOMPI_SKIP_MPICXX)
### 	target_include_directories(${target} PRIVATE SYSTEM ${MPI_INCLUDE_PATH})
### 	target_link_libraries(${target} PRIVATE ${MPI_C_LIBRARIES})
### endif()
### #install(TARGETS ${target} LIBRARY EXCLUDE_FROM_ALL)
### add_library(cpp-utils::shared ALIAS cpp-utils-shared)
