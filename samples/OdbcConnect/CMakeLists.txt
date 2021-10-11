cmake_minimum_required (VERSION 3.5)
project (OdbcConnect)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()

# files:
set ( SRC_ODBCCONNECTIONTEST
  OdbcConnect.cpp
)

set ( HEADERS_ODBCCONNECTIONTEST

)

# we depend on boost, with some built libs:
# http://stackoverflow.com/questions/28887680/linking-boost-library-with-boost-use-static-lib-off-on-windows
cmake_policy(SET CMP0074 NEW)
find_package(Boost 1.55.0 REQUIRED COMPONENTS
                program_options)
include_directories(${Boost_INCLUDE_DIRS})
link_libraries(${Boost_LIBRARIES})
if (WIN32 AND NOT Boost_USE_STATIC_LIBS)
  # force all boost libraries to link dynamically. If not set, boost tries
  # to link againts dlls names 'lib-whatever.lib', but on windows we do not have
  # a 'lib' prefix.
  message(STATUS "Plattform is Win32 and Boost_USE_STATIC_LIBS is not set: Adding BOOST_ALL_DYN_LINK")
  add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

# we depend on boost, but header-only:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# we build the SqlCPointerBuffer application
add_executable(odbcconnect ${SRC_ODBCCONNECTIONTEST} ${HEADERS_ODBCCONNECTIONTEST})

# the ListOdbcSources applications needs to link against libexodbc
target_link_libraries(odbcconnect libexodbc)
