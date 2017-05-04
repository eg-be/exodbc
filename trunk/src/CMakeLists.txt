cmake_minimum_required (VERSION 3.0.2)
project (libexodbc)

# update tweak version with info from subversion, if available:
# note: WC_LOG needs access to the repo, do not use
find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  set (LIBEXODBC_VERSION_TWEAK ${Project_WC_LAST_CHANGED_REV})
  message(STATUS "Updating LIBEXODBC_VERSION_TWEAK to ${LIBEXODBC_VERSION_TWEAK}, read from ${PROJECT_SOURCE_DIR}")
endif()

# create some files from the version information 
# and place it in the include-folder (so it could be used)
configure_file (
  "${PROJECT_SOURCE_DIR}/exOdbcConfig.h.in"
  "${PROJECT_SOURCE_DIR}/../include/exodbc/exOdbcConfig.h"
)
message(STATUS "Generated ${PROJECT_SOURCE_DIR}/../include/exodbc/exOdbcConfig.h")

# compiler flags
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()

# define import/export
if(BUILD_SHARED_LIBS)
add_definitions(
  -DEXODBC_LIB
)
endif()

# add the library headers:
include_directories ("${PROJECT_SOURCE_DIR}/../include/exodbc")

# we explicitely list all files:
set ( SRC_EXODBC 
  AssertionException.cpp 
  ColumnBuffer.cpp 
  ColumnBufferWrapper.cpp
  ColumnDescription.cpp
  ColumnInfo.cpp
  Database.cpp 
  DatabaseCatalog.cpp
  Environment.cpp 
  Exception.cpp 
  ExecutableStatement.cpp
  exOdbc.cpp 
  GetDataWrapper.cpp
  Helpers.cpp 
  LogHandler.cpp 
  LogManager.cpp 
  ParameterDescription.cpp
  PrimaryKeyInfo.cpp
  SpecialColumnInfo.cpp
  SpecializedExceptions.cpp 
  SqlInfoProperty.cpp
  Sql2BufferTypeMap.cpp 
  SqlStatementCloser.cpp 
  SqlTypeInfo.cpp
  Table.cpp 
  TableInfo.cpp
)

set ( HEADERS_EXODBC
  ../include/exodbc/AssertionException.h
  ../include/exodbc/bitmask_operators.hpp
  ../include/exodbc/ColumnBuffer.h
  ../include/exodbc/ColumnBufferVisitors.h
  ../include/exodbc/ColumnBufferWrapper.h
  ../include/exodbc/ColumnDescription.h
  ../include/exodbc/ColumnInfo.h
  ../include/exodbc/Database.h
  ../include/exodbc/DatabaseCatalog.h
  ../include/exodbc/DebugNew.h
  ../include/exodbc/DoxygenToc.h
  ../include/exodbc/EnumFlags.h
  ../include/exodbc/Environment.h
  ../include/exodbc/Exception.h
  ../include/exodbc/ExecutableStatement.h
  ../include/exodbc/exOdbc.h
  ../include/exodbc/GetDataWrapper.h
  ../include/exodbc/Helpers.h
  ../include/exodbc/LogHandler.h
  ../include/exodbc/LogManager.h
  ../include/exodbc/LogManagerOdbcMacros.h
  ../include/exodbc/ParameterDescription.h
  ../include/exodbc/PrimaryKeyInfo.h
  ../include/exodbc/SpecialColumnInfo.h
  ../include/exodbc/SpecializedExceptions.h
  ../include/exodbc/Sql2BufferTypeMap.h
  ../include/exodbc/SqlHandle.h
  ../include/exodbc/SqlInfoProperty.h  
  ../include/exodbc/SqlStatementCloser.h
  ../include/exodbc/SqlTypeInfo.h
  ../include/exodbc/Table.h
  ../include/exodbc/TableInfo.h
)

set ( CONFIG_EXODBC 
  ../include/exodbc/exOdbcConfig.h
)

set (RES_EXODBC
  exOdbc.rc
)

# we depend on boost:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# and we build the libexodbc library:
add_library(libexodbc ${SRC_EXODBC} ${HEADERS_EXODBC} ${RES_EXODBC} ${CONFIG_EXODBC} )
target_include_directories(libexodbc PUBLIC ../include)

# provide a group for the rc-file
source_group( "Resources"
  FILES
  exOdbc.rc
)

# and one for the generated config-file
source_group( "Config"
  FILES
  ../include/exodbc/exOdbcConfig.h
)

# and for the linker, add odbc32.lib to the standard libraries:
if(MSVC)
  target_link_libraries(libexodbc PUBLIC odbc32)
else()
  target_link_libraries(libexodbc PUBLIC odbc)
endif()

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
  message(STATUS "Doxygen has been found, adding target 'doc'")
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doygen-configuration.in ${CMAKE_CURRENT_BINARY_DIR}/Doxgen.configuration @ONLY)
  add_custom_target(doc
  ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxgen.configuration 
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
endif()