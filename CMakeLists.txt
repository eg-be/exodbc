cmake_minimum_required (VERSION 3.0.2)
project (exodbc
  VERSION 0.9.0.0
  LANGUAGES CXX
)

option(BUILD_TESTS "Build exodbctest program" OFF)
option(BUILD_SAMPLES "Build exodbcsamples programs" OFF)

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
# a note how we could set the env variable from within cmake:
#SET(ENV{LIBEXODBC_ONLY} "1")
if(NOT $ENV{LIBEXODBC_ONLY})
  message(STATUS "Environment variable LIBEXODBC_ONLY is not set.")
  message(STATUS "Including 'test' in build.")
  
  # and on GTest
  SET(BUILD_GMOCK OFF)
  SET(BUILD_GTEST ON)
  
  # Download and unpack googletest at configure time
  configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
  execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()
  execute_process(COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/googletest-download )
  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker
  # settings on Windows
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build)

  # The gtest/gtest_main targets carry header search path
  # dependencies automatically when using CMake 2.8.11 or
  # later. Otherwise we have to add them here ourselves.
  if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include")
  endif()  
  
  
  add_subdirectory(test)
endif()
