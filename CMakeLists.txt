cmake_minimum_required (VERSION 3.0.2)
project (exodbc
  VERSION 0.9.0.0
  LANGUAGES CXX
)

# Make the subprojects use the same-version-information
# by providing them here
set(EXODBC_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(EXODBC_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(EXODBC_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(EXODBC_VERSION_REV "${PROJECT_VERSION_TWEAK}")

# default to building shared-libraries
SET(BUILD_SHARED_LIBS ON)

# Set the build type if it isn't already
if(NOT CMAKE_BUILD_TYPE)
  message(STATUS "CMAKE_BUILD_TYPE not set, defaulting to 'Release'")
  set(CMAKE_BUILD_TYPE Release)
endif()

# put output into a folder 'bin'
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Only add Debug and Release Configuration  
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

add_subdirectory(src)
message(STATUS "foddo")
# a note how we could set the env variable from within cmake:
#SET(ENV{LIBEXODBC_ONLY} "1")
if(NOT $ENV{LIBEXODBC_ONLY})
  message(STATUS "Environment variable LIBEXODBC_ONLY is not set.")
  message(STATUS "Including 'test' in build.")
  add_subdirectory(test)
endif()
