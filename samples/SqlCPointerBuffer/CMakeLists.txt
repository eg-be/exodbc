cmake_minimum_required (VERSION 3.0.2)
project (SqlCPointerBuffer)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()

# files:
set ( SRC_SQLCPOINTERBUFFER
  Time2SqlCPointerBuffer.cpp
)

set ( HEADERS_SQLCPOINTERBUFFER
  msodbcsql.h
)

# we depend on boost, but header-only:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# we build the SqlCPointerBuffer application
add_executable(SqlCPointerBuffer ${SRC_SQLCPOINTERBUFFER} ${HEADERS_SQLCPOINTERBUFFER})

# the SqlCPointerBuffer applications needs to link against libexodbc
target_link_libraries(SqlCPointerBuffer PRIVATE libexodbc)
