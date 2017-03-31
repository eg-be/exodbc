cmake_minimum_required (VERSION 3.0.2)
project (ExodbcExec)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()

# files:
set ( SRC_EXODBCEXEC
  InputGenerator.cpp
  ExodbcExec.cpp
)

set ( HEADERS_EXODBCEXEC
  InputGenerator.h
  DebugNew.h
  ExodbcExec.h
)

# we depend on boost, but header-only:
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# we build the OdbcExec application
add_executable(exodbcexec ${SRC_EXODBCEXEC} ${HEADERS_EXODBCEXEC})

# the OdbcExec applications needs to link against libexodbc
target_link_libraries(exodbcexec libexodbc)
