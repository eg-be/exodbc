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
# http://stackoverflow.com/questions/28887680/linking-boost-library-with-boost-use-static-lib-off-on-windows
set(Boost_NO_SYSTEM_PATHS true)
set(Boost_USE_STATIC_LIBS OFF CACHE BOOL "use static libraries from Boost")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.55.0 REQUIRED COMPONENTS
                system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES})
if (WIN32)
  # force all boost libraries to dynamic link. If not set, boost (?) somehow
  # wants to link to dlls names 'lib-whatever.lib', but on windows we do not have
  # a 'lib' prefix.
  add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

# we build the exodbcTest application
add_executable(exodbctest ${SRC_EXODBCTEST} ${HEADERS_EXODBCTEST} ${RES_EXODBCTEST} ${CONFIG_EXODBCTEST} )

# the exodbctest applications needs to link against libexodbc, odbc32, gtest, boost: 
target_link_libraries(exodbctest libexodbc gtest ${Boost_LIBRARIES} odbc32)

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

# on successfull compilation, copy the TestSettings.xml file
add_custom_command(TARGET exodbctest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${PROJECT_SOURCE_DIR}/TestSettings.xml
                       $<TARGET_FILE_DIR:exodbctest>)
