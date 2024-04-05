@ECHO off

REM Run this script from the "X64 Native Tools Command Prompt for VS 2019 (or VS 2022)" so that all the compiler stuff is set up

REM BUILD_DIR is a temporary directory for building (compiling and linking)
set BUILD_DIR=%cd%\build-windows-vs2022
set INSTALL_DIR=%cd%\install-windows-vs2022

REM Optionally delete the BUILD_DIR to ensure a clean cache/start
RMDIR /S /Q %BUILD_DIR%

REM Set the GDAL path if not set in user environment externally
SET GDAL_DIR=%LocalAppData%\gdal-3.0.4

REM Set the CGAL & BOOST paths if necesary
set CGAL_DIR=%LocalAppData%\CGAL-5.4.1
set BOOST_ROOT=%LocalAppData%\boost_1_84_0

REM Create and cd to the BUILD_DIR
mkdir %BUILD_DIR%
cd    %BUILD_DIR%

REM First generate the build cache first
REM cmake -Wno-dev -G "Visual Studio 16 2019" -A x64 -DCMAKE_CXX_COMPILER=msvc ..
cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_COMPILER=msvc ..

REM Switches for turning off certain dependencies if they are not wanted or available
REM 	-DWITH_MPI=OFF
REM 	-DWITH_GDAL=OFF
REM 	-DWITH_CGAL=OFF
REM cmake -Wno-dev -G "Visual Studio 17 2022" -A x64 -DCMAKE_CXX_COMPILER=msvc -DWITH_MPI=OFF -DWITH_GDAL=OFF -DWITH_CGAL=OFF ..

REM Build and install everything
cmake --build . --config=Release
cmake --install . --prefix %INSTALL_DIR%

cd ..
PAUSE

