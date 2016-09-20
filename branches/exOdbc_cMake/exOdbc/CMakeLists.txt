cmake_minimum_required (VERSION 3.0.2)
project (exodbc 
  VERSION 0.9.0.0
  LANGUAGES CXX
)

# Only add Debug and Release Configuration  
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# put output into a folder 'bin'
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# versioning
#set (myLib_VERSION_MAJOR 1)
#set (myLib_VERSION_MINOR 0)
#set (myLib_VERSION_BUILD 9)
#set (myLib_VERSION_REV 0)

# update tweak version with info from subversion, if available:
find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  message("Current revision is ${Project_WC_REVISION}")
  Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
  message("Last changed log is ${Project_LAST_CHANGED_LOG}")
  set (PROJECT_VERSION_TWEAK ${Project_WC_REVISION})
endif()

# create some files from the version information:
configure_file (
  "${PROJECT_SOURCE_DIR}/exodbcConfig.h.in"
  "${PROJECT_BINARY_DIR}/exodbcConfig.h"
)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()


# includes: Add the build dir where the updated files will be written too:
include_directories ("${PROJECT_BINARY_DIR}")
# and add the library headers:
include_directories ("${PROJECT_SOURCE_DIR}/include/exodbc")

# we explicitely list all files, as recommended by cmake:
set ( SRC_EXODBC 
  src/AssertionException.cpp 
  src/ColumnBuffer.cpp 
  src/Database.cpp 
  src/Environment.cpp 
  src/Exception.cpp 
  src/ExecutableStatement.cpp
  src/exOdbc.cpp 
  src/Helpers.cpp 
  src/InfoObject.cpp 
  src/LogHandler.cpp 
  src/LogManager.cpp 
  src/SpecializedExceptions.cpp 
  src/Sql2BufferTypeMap.cpp 
  src/SqlStatementCloser.cpp 
  src/Table.cpp 
  src/TablePrivileges.cpp 
  src/targetver.h
)

set ( HEADERS_EXODBC
  include/exodbc/AssertionException.h
  include/exodbc/bitmask_operators.hpp
  include/exodbc/ColumnBuffer.h
  include/exodbc/ColumnBufferVisitors.h
  include/exodbc/Database.h
  include/exodbc/DebugNew.h
  include/exodbc/DoxygenToc.h
  include/exodbc/EnumFlags.h
  include/exodbc/Environment.h
  include/exodbc/Exception.h
  include/exodbc/ExecutableStatement.h
  include/exodbc/exOdbc.h
  include/exodbc/Helpers.h
  include/exodbc/InfoObject.h
  include/exodbc/LogHandler.h
  include/exodbc/LogManager.h
  include/exodbc/SpecializedExceptions.h
  include/exodbc/Sql2BufferTypeMap.h
  include/exodbc/SqlHandle.h
  include/exodbc/SqlStatementCloser.h
  include/exodbc/Table.h
  include/exodbc/TablePrivileges.h
)

set (RES_EXODBC
  res/exOdbc.rc
)

# we depend on boost:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
else()
  message(WARNING "Boost libraries were not found, try setting CMAKE_PREFIX_PATH or BOOST_INCLUDE_DIR")
endif()

# and we build the shared and static exodbc library
add_library(exodbc SHARED ${SRC_EXODBC} ${HEADERS_EXODBC} ${RES_EXODBC} )
add_library(libexodbc STATIC ${SRC_EXODBC} ${HEADERS_EXODBC} ${RES_EXODBC} )

# provide a group for the rc-file
source_group( "Resources"
  FILES
  res/exOdbc.rc
)

# and for the linker, add odbc32.lib to the standard libraries:
if(MSVC)
  target_link_libraries(exodbc PRIVATE odbc32)
  target_link_libraries(libexodbc PRIVATE odbc32)
endif()
