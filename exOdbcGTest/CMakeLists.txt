cmake_minimum_required (VERSION 3.0.2)
project (exodbctest 
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
  "${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h.in"
  "${PROJECT_BINARY_DIR}/exOdbcTestConfig.h"
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

# we explicitely list all files, as recommended by cmake:
set ( SRC_EXODBCTEST 
  src/ColumnBufferTest.cpp 
  src/DatabaseTest.cpp 
  src/EnumFlagsTest.cpp
  src/EnvironmentTest.cpp
  src/ExcelTest.cpp
  src/ExecutableStatementTest.cpp
  src/exOdbcGTest.cpp 
  src/exOdbcGTestHelpers.cpp 
  src/HelpersTest.cpp 
  src/LogManagerTest.cpp 
  src/ManualTestTables.cpp
  src/SqlHandleTest.cpp
  src/SqlStmtCloserTest.cpp
  src/TableTest.cpp 
  src/TestDbCreator.cpp
  src/TestParams.cpp
  src/wxCompatibilityTest.cpp
)

set ( HEADERS_EXODBCTEST
  src/ColumnBufferTest.h
  src/DatabaseTest.h
  src/DebugNew.h
  src/EnumFlagsTest.h
  src/EnvironmentTest.h
  src/ExcelTest.h
  src/ExecutableStatementTest.h
  src/exOdbcGTest.h
  src/exOdbcGTestHelpers.h
  src/HelpersTest.h
  src/LogManagerTest.h
  src/ManualTestTables.h
  src/SqlHandleTest.h
  src/SqlStmtCloserTest.h
  src/TableTest.h 
  src/TestDbCreator.h
  src/TestParams.h
  src/wxCompatibilityTest.h
)

set ( CONFIG_EXODBCTEST 
  ${PROJECT_BINARY_DIR}/exodbcTestConfig.h
)

set (RES_EXODBCTEST
  res/exOdbcTest.rc
)

# we depend on boost:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# and we build the exodbcTest application
add_executable(exodbcTest ${SRC_EXODBCTEST} ${HEADERS_EXODBCTEST} ${RES_EXODBCTEST} ${CONFIG_EXODBCTEST} )

# provide a group for the rc-file
source_group( "Resources"
  FILES
  res/exOdbcTest.rc
)

# and one for the generated config-file
source_group( "Config"
  FILES
  ${PROJECT_BINARY_DIR}/exodbcTestConfig.h
)

# and for the linker, add odbc32.lib to the standard libraries:
#if(MSVC)
#  target_link_libraries(exodbc PRIVATE odbc32)
#  target_link_libraries(libexodbc PRIVATE odbc32)
#endif()
