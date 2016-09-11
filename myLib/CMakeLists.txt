cmake_minimum_required (VERSION 3.0)
project (myLib)
# versioning
set (myLib_VERSION_MAJOR 1)
set (myLib_VERSION_MINOR 0)
set (myLib_VERSION_BUILD 9)

configure_file (
  "${PROJECT_SOURCE_DIR}/myLibConfig.h.in"
  "${PROJECT_BINARY_DIR}/myLibConfig.h"
)

include_directories ("${PROJECT_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/include/mylib")

set ( SRC_MYLIB 
src/myLib.cpp 
)
set ( HEADERS_MYLIB
include/mylib/myLib.h
)
add_library(myLib SHARED ${SRC_MYLIB} ${HEADERS_MYLIB} )
