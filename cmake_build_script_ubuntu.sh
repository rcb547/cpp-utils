#!/bin/sh

# BUILD_DIR is a temporary directory for building (compiling and linking)
export BUILD_DIR=$PWD/build-ubuntu
# INSTALL_DIR is the directory for installing the build package
export INSTALL_DIR=$PWD/install-ubuntu

mkdir $BUILD_DIR
cd $BUILD_DIR

#Example for the GNU compilers
cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target all
cmake --install . --prefix $INSTALL_DIR


