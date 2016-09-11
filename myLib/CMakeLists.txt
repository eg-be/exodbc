cmake_minimum_required (VERSION 3.0)
project (myLib)
include_directories ("${PROJECT_SOURCE_DIR}/include/mylib")
set ( SRC_MYLIB 
src/myLib.cpp 
)
set ( HEADERS_MYLIB
include/mylib/myLib.h
)
add_library(myLib SHARED ${SRC_MYLIB} ${HEADERS_MYLIB} )
