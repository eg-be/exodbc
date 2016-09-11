cmake_minimum_required (VERSION 3.0)
project (myLib)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
# versioning
set (myLib_VERSION_MAJOR 1)
set (myLib_VERSION_MINOR 0)
set (myLib_VERSION_BUILD 9)
set (myLib_VERSION_REV 0)

find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  message("Current revision is ${Project_WC_REVISION}")
  Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
  message("Last changed log is ${Project_LAST_CHANGED_LOG}")
  set (myLib_VERSION_REV ${Project_WC_REVISION})
endif()

configure_file (
  "${PROJECT_SOURCE_DIR}/myLibConfig.h.in"
  "${PROJECT_BINARY_DIR}/myLibConfig.h"
)

add_definitions(-D_UNICODE)

include_directories ("${PROJECT_BINARY_DIR}")

include_directories ("${PROJECT_SOURCE_DIR}/include/mylib")

set ( SRC_MYLIB 
src/myLib.cpp 
)
set ( HEADERS_MYLIB
include/mylib/myLib.h
)

find_package(Boost 1.36.0)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

add_library(myLib SHARED ${SRC_MYLIB} ${HEADERS_MYLIB} )
