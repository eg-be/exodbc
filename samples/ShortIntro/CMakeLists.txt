cmake_minimum_required (VERSION 3.5)
project (ShortIntro)

# some help for the compiler:
if(MSVC)
add_definitions(
  -D_UNICODE
  -D_SCL_SECURE_NO_WARNINGS
  -D_CRT_SECURE_NO_WARNINGS
)
endif()

# files:
set ( SRC_SHORTINTRO
  ShortIntro.cpp
)

# we depend on boost, but header-only:
cmake_policy(SET CMP0074 NEW)
find_package(Boost 1.55.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

# we build the ShortIntro application
add_executable(shortintro ${SRC_SHORTINTRO} )

# the ShortIntro applications needs to link against libexodbc
target_link_libraries(shortintro libexodbc)
