cmake_minimum_required (VERSION 3.0.2)
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

# we depend on boost, but header-only:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# we build the SqlCPointerBuffer application
add_executable(odbcconnect ${SRC_ODBCCONNECTIONTEST} ${HEADERS_ODBCCONNECTIONTEST})

# the ListOdbcSources applications needs to link against libexodbc
target_link_libraries(odbcconnect libexodbc)
