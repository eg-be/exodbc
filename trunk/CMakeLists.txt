cmake_minimum_required (VERSION 3.0.2)
project (exodbc
  VERSION 0.9.3.0
  LANGUAGES CXX
)

SET(CPACK_GENERATOR "TGZ")
 
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PROJECT_VERSION_PATCH})

set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "exODBC - open source C++ ODBC library")
set(CPACK_PACKAGE_VENDOR "Elias Gerber")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/INSTALL.txt)
set(CPACK_GENERATOR TGZ)

# try to add revision to filename
SET(CPACK_PACKAGE_VERSION_TWEAK 0)
find_package(Subversion)
if(SUBVERSION_FOUND)
  Subversion_WC_INFO(${PROJECT_SOURCE_DIR} Project)
  set (CPACK_PACKAGE_VERSION_TWEAK ${Project_WC_LAST_CHANGED_REV})
  message(STATUS "Updating CPACK_PACKAGE_VERSION_TWEAK to ${CPACK_PACKAGE_VERSION_TWEAK}, read from ${PROJECT_SOURCE_DIR}")
endif()

if(DEFINED CPACK_EXPLICIT_VERSIONNAME)
  set(
    CPACK_SOURCE_PACKAGE_FILE_NAME
    "libexodbc-${CPACK_EXPLICIT_VERSIONNAME}-r${CPACK_PACKAGE_VERSION_TWEAK}"
    CACHE INTERNAL "tarball basename" FORCE
  )
else()
  set(
    CPACK_SOURCE_PACKAGE_FILE_NAME
    "libexodbc-${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}-r${CPACK_PACKAGE_VERSION_TWEAK}"
    CACHE INTERNAL "tarball basename" FORCE
  )
endif()
message(STATUS "Cpack output file is set to ${CPACK_SOURCE_PACKAGE_FILE_NAME}")

set(CPACK_SOURCE_GENERATOR TGZ)
# The following components are regex's to match anywhere (unless anchored)
# in absolute path + filename to find files or directories to be excluded
# from source tarball.
set(CPACK_SOURCE_IGNORE_FILES
"${CMAKE_BINARY_DIR}"
"\\\\.svn"
)
include(CPack)

# Options:
option(BUILD_TESTS "Build exodbctest program" ON)
option(BUILD_SAMPLES "Build exodbcsamples programs" ON)
# default to building static-libraries, so we dont need any 
# dlls at the right place:
option(BUILD_SHARED_LIBS "Build as shared library" OFF)

# default to use boost as a static lib on windows, to avoid
# having to copy around dlls:
if(WIN32)
  option(Boost_USE_STATIC_LIBS "Link against static boost libraries" ON)
endif()

# Only add Debug and Release Configuration  
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

# Make the subprojects use the same-version-information
# by providing them here
set(EXODBC_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(EXODBC_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(EXODBC_VERSION_PATCH "${PROJECT_VERSION_PATCH}")
set(EXODBC_VERSION_REV "${PROJECT_VERSION_TWEAK}")

# Output what we think we are building for:
message(STATUS "BUILD_SHARED_LIBS is set to: ${BUILD_SHARED_LIBS}")
if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
 message(STATUS "CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P}, indicating 64bit.")
else()
 message(STATUS "CMAKE_SIZEOF_VOID_P is ${CMAKE_SIZEOF_VOID_P}, indicating 32bit.")
endif()

# put output into a folder 'bin'
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)

if(${BUILD_SAMPLES})
  message(STATUS "Option BUILD_SAMPLES is set to ${BUILD_SAMPLES}, building exodbcsamples")
  add_subdirectory(samples/DatabaseInfo)
  add_subdirectory(samples/ShortIntro)
  add_subdirectory(samples/SqlCPointerBuffer)  
  add_subdirectory(samples/ListOdbcSources)  
  add_subdirectory(samples/OdbcConnect)
  add_subdirectory(samples/ExodbcExec)  
endif()

if(${BUILD_TESTS})
  message(STATUS "Option BUILD_TESTS is set to ${BUILD_TESTS}, building exodbctest")
  enable_testing()
  
  # and on GTest. We only need GTest  
  SET(BUILD_GMOCK OFF CACHE BOOL "Builds the googlemock subproject" FORCE)  
  SET(BUILD_GTEST ON CACHE BOOL "Builds the googletest subproject" FORCE)  
  
  # Download and unpack googletest at configure time
  configure_file(Googletest-CMakeLists.txt.in googletest-download/CMakeLists.txt)
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
  # settings on Windows: If not set, gtest will probably default to
  # use /MT (static runtime lib), but we prefer /MD (shared/dll 
  # runtime lib):
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines
  # the gtest and gtest_main targets.
  # hide some warnings from within googletest 1.8.0 with cmake 3.x
  # by setting policy CMP0048 to OLD:
  file(READ ${CMAKE_BINARY_DIR}/googletest-src/CMakeLists.txt GOOGELTEST_CMAKELISTS)
  if(NOT GOOGELTEST_CMAKELISTS MATCHES "SET CMP0048 OLD")
    string(TIMESTAMP NOW)
    STRING(REGEX REPLACE 
      "(.*cmake_minimum_required.*)project\\(" 
      "\\1# ${NOW} edited by exodbc/CMakeLists.txt. Avoid CMP0048 warnings:\ncmake_policy(SET CMP0048 OLD)\n\nproject(" 
      GOOGELTEST_CMAKELISTS ${GOOGELTEST_CMAKELISTS})
    file(WRITE ${CMAKE_BINARY_DIR}/googletest-src/CMakeLists.txt ${GOOGELTEST_CMAKELISTS})
  endif()
  
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build)
  
  # and include the test directory:
  add_subdirectory(test)
endif()
