cmake_minimum_required (VERSION 3.0.2)
project (exodbctest)

# update tweak version with info from subversion, if available:
find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  Subversion_WC_LOG(${PROJECT_SOURCE_DIR} Project)
  set (EXODBCTEST_VERSION_TWEAK ${Project_WC_REVISION})
  message(STATUS "Updating EXODBCTEST_VERSION_TWEAK to ${EXODBCTEST_VERSION_TWEAK}, read from ${PROJECT_SOURCE_DIR}")  
endif()

# create some files from the version information:
configure_file (
  "${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h.in"
  "${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h"
)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()
add_definitions(
  -DEXODBC_TEST
)

# we explicitely list all files, as recommended by cmake:
set ( SRC_EXODBCTEST 
  ColumnBufferTest.cpp 
  DatabaseTest.cpp 
  EnumFlagsTest.cpp
  EnvironmentTest.cpp
  ExcelTest.cpp
  ExecutableStatementTest.cpp
  exOdbcGTest.cpp 
  exOdbcGTestHelpers.cpp 
  HelpersTest.cpp 
  LogManagerTest.cpp 
  ManualTestTables.cpp
  SqlHandleTest.cpp
  SqlStmtCloserTest.cpp
  TableTest.cpp 
  TestDbCreator.cpp
  TestParams.cpp
  wxCompatibilityTest.cpp
)

set ( HEADERS_EXODBCTEST
  ColumnBufferTest.h
  DatabaseTest.h
  DebugNew.h
  EnumFlagsTest.h
  EnvironmentTest.h
  ExcelTest.h
  ExecutableStatementTest.h
  exOdbcGTest.h
  exOdbcGTestHelpers.h
  HelpersTest.h
  LogManagerTest.h
  ManualTestTables.h
  SqlHandleTest.h
  SqlStmtCloserTest.h
  TableTest.h 
  TestDbCreator.h
  TestParams.h
  wxCompatibilityTest.h
)

set ( CONFIG_EXODBCTEST 
  exOdbcTestConfig.h
)

set (RES_EXODBCTEST
  exOdbcTest.rc
)

# we depend on boost:
find_package(Boost 1.55.0 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

# and on GTest
#find_package(GTest 1.7.0 REQUIRED)
#include_directories(${GTEST_INCLUDE_DIRS})
# Enable ExternalProject CMake module
include(ExternalProject)

# Download and install GoogleTest
ExternalProject_Add(
    gtest
    URL https://github.com/google/googletest/archive/master.zip
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/gtest
    # Disable install step
    INSTALL_COMMAND ""
)

# Get GTest source and binary directories from CMake project
ExternalProject_Get_Property(gtest source_dir binary_dir)

# Create a libgtest target to be used as a dependency by test programs
add_library(libgtest IMPORTED STATIC GLOBAL)
add_dependencies(libgtest gtest)

# Set libgtest properties
#set_target_properties(libgtest PROPERTIES
#    "IMPORTED_LOCATION" "${binary_dir}/googlemock/gtest/libgtest.a"
#    "IMPORTED_LINK_INTERFACE_LIBRARIES" "${CMAKE_THREAD_LIBS_INIT}"
#)

# and we build the exodbcTest application
add_executable(exodbctest ${SRC_EXODBCTEST} ${HEADERS_EXODBCTEST} ${RES_EXODBCTEST} ${CONFIG_EXODBCTEST} )

# and on exodbc
add_dependencies(exodbctest libexodbc libgtest)

# provide a group for the rc-file
source_group( "Resources"
  FILES
  exOdbcTest.rc
)

# and one for the generated config-file
source_group( "Config"
  FILES
  exOdbcTestConfig.h
)

target_link_libraries(exodbctest exodbc)

# and for the linker, add odbc32.lib to the standard libraries:
#if(MSVC)
#  target_link_libraries(exodbc PRIVATE odbc32)
#  target_link_libraries(libexodbc PRIVATE odbc32)
#endif()
