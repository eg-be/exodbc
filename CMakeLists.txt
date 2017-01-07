﻿cmake_minimum_required (VERSION 3.0.2)
project (exodbc
  VERSION 0.9.0.0
  LANGUAGES CXX
)

option(BUILD_TESTS "Build exodbctest program" ON)
option(BUILD_SAMPLES "Build exodbcsamples programs" ON)

# Make the subprojects use the same-version-information
# by providing them here
set(EXODBC_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(EXODBC_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(EXODBC_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(EXODBC_VERSION_REV "${PROJECT_VERSION_TWEAK}")

# default to building shared-libraries
message(STATUS "BUILD_SHARED_LIBS: ${BUILD_SHARED_LIBS}")
if(NOT DEFINED BUILD_SHARED_LIBS)
 message(STATUS "BUILD_SHARED_LIBS not set, defaulting to 'ON'")
 SET(BUILD_SHARED_LIBS ON)
endif()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
 message(STATUS "CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P}, indicating 64bit.")
else()
 message(STATUS "CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P}, indicating 32bit.")
endif()

# put output into a folder 'bin'
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin/Debug)
#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin/Release)

# Only add Debug and Release Configuration  
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

add_subdirectory(src)

if(${BUILD_SAMPLES})
  message(STATUS "Option BUILD_SAMPLES is set to ${BUILD_SAMPLES}, building exodbcsamples")
  add_subdirectory(samples/DatabaseInfo)
  add_subdirectory(samples/ShortIntro)
  add_subdirectory(samples/SqlCPointerBuffer)  
endif()

if(${BUILD_TESTS})
  message(STATUS "Option BUILD_TESTS is set to ${BUILD_TESTS}, building exodbctest")
  enable_testing()
  
  # and on GTest. We only need GTest  
  SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)  
  SET(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject" FORCE)  
  
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
  
  # and include the test directory:
  add_subdirectory(test)
endif()