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

# update tweak version with info from subversion, if available:
find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
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

set ( CONFIG_EXODBC 
  ${PROJECT_BINARY_DIR}/exodbcConfig.h
)

set (RES_EXODBC
  res/exOdbc.rc
)

# we depend on boost:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# and we build the shared and static exodbc library
add_library(exodbc SHARED ${SRC_EXODBC} ${HEADERS_EXODBC} ${RES_EXODBC} ${CONFIG_EXODBC} )
add_library(libexodbc STATIC ${SRC_EXODBC} ${HEADERS_EXODBC} ${RES_EXODBC} ${CONFIG_EXODBC} )
target_include_directories(exodbc PUBLIC include)
target_include_directories(libexodbc PUBLIC include)

# provide a group for the rc-file
source_group( "Resources"
  FILES
  res/exOdbc.rc
)

# and one for the generated config-file
source_group( "Config"
  FILES
  ${PROJECT_BINARY_DIR}/exodbcConfig.h
)

# and for the linker, add odbc32.lib to the standard libraries:
if(MSVC)
  target_link_libraries(exodbc PRIVATE odbc32)
  target_link_libraries(libexodbc PRIVATE odbc32)
endif()
