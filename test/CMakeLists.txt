﻿cmake_minimum_required (VERSION 3.5)
project (exodbctest)

# add some options to enable / disable tests and add database specific values

# SqlServer
option(ENABLE_SQLSERVER_TEST "Enable the test for Sql Server" ON)
set(SQLSERVER_TEST_CS "Driver={SQL Server Native Client 11.0};Server=192.168.56.20\\EXODBC;Database=exodbc;Uid=ex;Pwd=extest;" CACHE STRING "Connection-String used to run tests against Sql Server")
set(SQLSERVER_TEST_GTEST_FILTER "*" CACHE STRING "value for gtest argument '--gtest_filter': Run all tests")
set(SQLSERVER_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against Sql Server")

# Postgresql: Driver named differently depending on bitness
# And depending on Plattform, we have different issues
option(ENABLE_POSTGRESQL_TEST "Enable the test for PostgreSQL" ON)
set(POSTGRESQL_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against PostgreSQL")
string(CONCAT postgreSqlFilters	"-ExecutableStatementTest.SelectAbsolute"
                ":ExecutableStatementTest.SelectFirst"
                ":ExecutableStatementTest.SelectLast"
                ":ExecutableStatementTest.SelectPrev"
                ":ExecutableStatementTest.SelectRelative"
                ":TableTest.SelectAbsolute"
                ":TableTest.SelectFirst"
                ":TableTest.SelectLast"
                ":TableTest.SelectPrev"
                ":TableTest.SelectRelative"
                ":DatabaseCatalogTest.ReadSpecialColumnInfo")
# The default install for PostgreSQL odbc driver on linux has no (x64) added (debian, jessie)
if (CMAKE_SIZEOF_VOID_P LESS 8 OR NOT WIN32)
  set(POSTGRESQL_TEST_CS "Driver={PostgreSQL Unicode};Server=192.168.56.20;Port=5432;Database=exodbc;Uid=ex;Pwd=extest" CACHE STRING "Connection-String used to run tests against PostgreSQL")
else()
  set(POSTGRESQL_TEST_CS "Driver={PostgreSQL Unicode(x64)};Server=192.168.56.20;Port=5432;Database=exodbc;Uid=ex;Pwd=extest" CACHE STRING "Connection-String used to run tests against PostgreSQL")
endif()
# There are some different known failures on the linux / windows odbc driver:
if (WIN32)
  set(postgreSqlFilters "${postgreSqlFilters}:StatementCloserTest.CloseStmtHandle")
else()
  set(postgreSqlFilters "${postgreSqlFilters}:DatabaseTest.CanSetTransactionIsolationMode")
endif()
set(POSTGRESQL_TEST_GTEST_FILTER "${postgreSqlFilters}" CACHE STRING "value for gtest argument '--gtest_filter': Skip tests known to fail")


# MySql (failed to setup odbc on linux)
if(WIN32)
  option(ENABLE_MYSQL_TEST "Enable the test for MySql" ON)
else()
  option(ENABLE_MYSQL_TEST "Enable the test for MySql" OFF)
endif()
if(ENABLE_MYSQL_TEST)
  set(MYSQL_TEST_CS "Provider=MSDASQL;Driver={MySQL ODBC 8.0 UNICODE Driver};Server=192.168.56.20;Database=exodbc;User=ex;Password=extest;Option=3;" CACHE STRING "Connection-String used to run tests against MySql")
	string(CONCAT mySqlFilters	"-StatementCloserTest.CloseStmtHandle"
								":TableTest.DeleteFailOnNoData")  
  set(MYSQL_TEST_GTEST_FILTER "${mySqlFilters}" CACHE STRING "value for gtest argument '--gtest_filter': Skip tests known to fail")
  set(MYSQL_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against MySql")  
endif()

# DB2 (did not find odbc driver for linux)
if(WIN32)
  option(ENABLE_DB2_TEST "Enable the test for DB2" ON)
else()
  option(ENABLE_DB2_TEST "Enable the test for DB2" OFF)
endif()
if(ENABLE_DB2_TEST)
  set(DB2_TEST_CS "Driver={IBM DB2 ODBC DRIVER};Database=EXODBC;Hostname=192.168.56.20;Port=50000;Protocol=TCPIP;Uid=db2ex;Pwd=extest;" CACHE STRING "Connection-String used to run tests against DB2")
  set(DB2_TEST_GTEST_FILTER "*" CACHE STRING "value for gtest argument '--gtest_filter': Run all tests")
  set(DB2_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against DB2")    
endif()

# Access and Excel: We have only 32bit drivers and only on Windows
if(WIN32 AND CMAKE_SIZEOF_VOID_P LESS 8)
	message(STATUS "Plattform is Win32 and CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P}, assuming x86 and adding options to test Access and Excel")

	option(ENABLE_ACCESS_TEST "Enable the test for Access" ON)
  set(ACCESS_TEST_CS "Driver={Microsoft Access Driver (*.mdb)};Dbq=$<TARGET_FILE_DIR:exodbctest>\\exodbc.mdb;Uid=Admin;Pwd=;ExtendedAnsiSQL=1;" CACHE STRING "Connection-String used to run tests against Access.")  
  string(CONCAT accessFilters "-BigIntColumnTest.*"
                ":wxCompatibilityTest.*"
                ":DatabaseCatalogTest.ListSchemas"
                ":DatabaseCatalogTest.ReadSpecialColumnInfo"
				":DatabaseCatalogTest.ReadPrimaryKeyInfo"
                ":DatabaseTest.ReadSchemas"
                ":DatabaseTest.ReadSpecialColumns"
                ":ExecutableStatementTest.SelectAbsolute"
                ":ExecutableStatementTest.SelectFirst"
                ":ExecutableStatementTest.SelectLast"
                ":ExecutableStatementTest.SelectPrev"
                ":ExecutableStatementTest.SelectRelative"
                ":SqlCPointerTest.ReadBigIntValue"
                ":SqlCPointerTest.WriteBigIntValue"
                ":SqlCPointerTest.ReadNumeric_*"
                ":SqlCPointerTest.WriteNumeric_*"
                ":TableTest.CreateAutoNumericTypeBuffers"
                ":TableTest.DeleteFailOnNoData"
                ":TableTest.DeletePk:TableTest.OpenAutoSkipUnsupportedColumn"
                ":TableTest.QueryPrimaryKeysAndUpdateColumns"
                ":TableTest.SelectAbsolute"
                ":TableTest.SelectFirst"
                ":TableTest.SelectFromAutoWithSkippedUnsupportedColumn"
                ":TableTest.SelectLast:TableTest.SelectPrev"
                ":TableTest.SelectRelative"
                ":TableTest.UpdateFlag"
                ":TableTest.UpdatePk")
  set(ACCESS_TEST_GTEST_FILTER "${accessFilters}" CACHE STRING "value for gtest argument '--gtest_filter': Skip tests known to fail")
  set(ACCESS_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against Access")  

	option(ENABLE_EXCEL_TEST "Enable the test for Excel" ON)
  set(EXCEL_TEST_CS "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};DBQ=$<TARGET_FILE_DIR:exodbctest>\\excelTest.xls;" CACHE STRING "Connection-String used to run tests against Excel.")    
  set(EXCEL_TEST_GTEST_FILTER "ExcelTest.*" CACHE STRING "value for gtest argument '--gtest_filter': Only test the few very basic Excel-things")
  set(EXCEL_TEST_LOG_LEVEL "Warning" CACHE STRING "LogLevel argument passed to exodbctest to run tests against Excel")    
endif()


# update tweak version with info from subversion, if available:
SET(EXODBCTEST_VERSION_TWEAK 0)
#find_package(Subversion)
#if(SUBVERSION_FOUND)
#  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
#  set (EXODBCTEST_VERSION_TWEAK ${Project_WC_LAST_CHANGED_REV})
#  message(STATUS "Updating EXODBCTEST_VERSION_TWEAK to ${EXODBCTEST_VERSION_TWEAK}, read from ${PROJECT_SOURCE_DIR}")  
#endif()

# create some files from the version information:
configure_file (
  "${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h.in"
  "${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h"
)
message(STATUS "Generated ${PROJECT_SOURCE_DIR}/exOdbcTestConfig.h")

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

# we explicitely list all files:
set ( SRC_EXODBCTEST 
  ColumnBufferTest.cpp 
  DatabaseCatalogTest.cpp
  DatabaseTest.cpp 
  EnumFlagsTest.cpp
  EnvironmentTest.cpp
  ExcelTest.cpp
  ExecutableStatementTest.cpp
  exOdbcTest.cpp 
  exOdbcTestHelpers.cpp 
  GetDataWrapperTest.cpp
  LogManagerTest.cpp 
  ManualTestTables.cpp
  SetDescriptionFieldWrapperTest.cpp
  SqlHandleTest.cpp
  SqlInfoPropertyTest.cpp
  SqlStmtCloserTest.cpp
  SqlStructHelperTest.cpp
  TableTest.cpp 
  TestDbCreator.cpp
  TestParams.cpp
  UnicodeTest.cpp
  wxCompatibilityTest.cpp
)

set ( HEADERS_EXODBCTEST
  ColumnBufferTest.h
  DatabaseCatalogTest.h
  DatabaseTest.h
  DebugNew.h
  EnumFlagsTest.h
  EnvironmentTest.h
  ExcelTest.h
  ExecutableStatementTest.h
  exOdbcTest.h
  exOdbcTestHelpers.h
  GetDataWrapperTest.h
  LogManagerTest.h
  ManualTestTables.h
  SetDescriptionFieldWrapperTest.h
  SqlHandleTest.h
  SqlInfoPropertyTest.h
  SqlStmtCloserTest.h
  SqlStructHelperTest.h
  TableTest.h 
  TestDbCreator.h
  TestParams.h
  UnicodeTest.h
  wxCompatibilityTest.h
)

set ( CONFIG_EXODBCTEST 
  exOdbcTestConfig.h
  TestSettings.xml
)

set (RES_EXODBCTEST
  exOdbcTest.rc
)

# we depend on boost, with some built libs:
# http://stackoverflow.com/questions/28887680/linking-boost-library-with-boost-use-static-lib-off-on-windows
cmake_policy(SET CMP0074 NEW)
find_package(Boost 1.55.0 REQUIRED COMPONENTS
                system filesystem)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES})
if (WIN32 AND NOT Boost_USE_STATIC_LIBS)
  # force all boost libraries to link dynamically. If not set, boost tries
  # to link againts dlls names 'lib-whatever.lib', but on windows we do not have
  # a 'lib' prefix.
  message(STATUS "Plattform is Win32 and Boost_USE_STATIC_LIBS is not set: Adding BOOST_ALL_DYN_LINK")
  add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

# we build the exodbcTest application
add_executable(exodbctest ${SRC_EXODBCTEST} ${HEADERS_EXODBCTEST} ${RES_EXODBCTEST} ${CONFIG_EXODBCTEST} )

# the exodbctest applications needs to link against libexodbc, odbc32, gtest, boost: 
if(MSVC)
  target_link_libraries(exodbctest libexodbc gtest ${Boost_LIBRARIES} odbc32)
else()
  target_link_libraries(exodbctest libexodbc gtest ${Boost_LIBRARIES} odbc)
endif()

# provide a group for the rc-file
source_group( "Resources"
  FILES
  exOdbcTest.rc
)

# and one for the generated config-file
source_group( "Config"
  FILES
  exOdbcTestConfig.h
  TestSettings.xml
)

# on successfull compilation, copy the TestSettings.xml file
add_custom_command(TARGET exodbctest POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy
                       ${PROJECT_SOURCE_DIR}/TestSettings.xml
                       $<TARGET_FILE_DIR:exodbctest>)

# and copy the create test db scripts:
add_custom_command(TARGET exodbctest POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory
					${PROJECT_SOURCE_DIR}/sql
					$<TARGET_FILE_DIR:exodbctest>/CreateScripts)

# copy the excel and access files only for windows
if(WIN32 AND CMAKE_SIZEOF_VOID_P LESS 8)
	if(ENABLE_ACCESS_TEST)
		add_custom_command(TARGET exodbctest POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy
					${PROJECT_SOURCE_DIR}/db/access/exodbc.mdb
					$<TARGET_FILE_DIR:exodbctest>)

		add_test( NAME AccessTest
              COMMAND exodbctest
              "-CS" "${ACCESS_TEST_CS}"
              "--gtest_filter=${ACCESS_TEST_GTEST_FILTER}"
              "--logLevel" "${ACCESS_TEST_LOG_LEVEL}"
              "--case" "l"
            )
	endif()

	if(ENABLE_EXCEL_TEST)
		add_custom_command(TARGET exodbctest POST_BUILD	
					COMMAND ${CMAKE_COMMAND} -E copy
					${PROJECT_SOURCE_DIR}/db/excel/excelTest.xls
					$<TARGET_FILE_DIR:exodbctest>)

		add_test( NAME ExcelTest
              COMMAND exodbctest
              "-CS" "${EXCEL_TEST_CS}"
              "--gtest_filter=${EXCEL_TEST_GTEST_FILTER}"
              "--logLevel" "${EXCEL_TEST_LOG_LEVEL}"
              "--case" "l"
            )
	endif()
endif()

if(ENABLE_MYSQL_TEST)
	add_test( NAME MySqlTest 
            COMMAND exodbctest
            "-CS" "${MYSQL_TEST_CS}"
            "--gtest_filter=${MYSQL_TEST_GTEST_FILTER}"
            "--logLevel" "${MYSQL_TEST_LOG_LEVEL}"
            "--case" "l"
          )
endif()

if(ENABLE_SQLSERVER_TEST)
	add_test( NAME SqlServerTest 
            COMMAND exodbctest
            "-CS" "${SQLSERVER_TEST_CS}"
            "--gtest_filter=${SQLSERVER_TEST_GTEST_FILTER}"
            "--logLevel" "${SQLSERVER_TEST_LOG_LEVEL}"
            "--case" "l"
          )
endif()

if(ENABLE_POSTGRESQL_TEST)
	add_test( NAME PostgreSQLTest 
            COMMAND exodbctest
            "-CS" "${POSTGRESQL_TEST_CS}"
            "--gtest_filter=${POSTGRESQL_TEST_GTEST_FILTER}"
            "--logLevel" "${POSTGRESQL_TEST_LOG_LEVEL}"
            "--case" "l"
          )
endif()

if(ENABLE_DB2_TEST)
	add_test( NAME Db2Test
            COMMAND exodbctest
            "-CS" "${DB2_TEST_CS}"
            "--gtest_filter=${DB2_TEST_GTEST_FILTER}"
            "--logLevel" "${DB2_TEST_LOG_LEVEL}"
            "--case" "u"
          )
endif()
