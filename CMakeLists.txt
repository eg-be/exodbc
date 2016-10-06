cmake_minimum_required (VERSION 3.0.2)
project (exodbc)

# put output into a folder 'bin'
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Only add Debug and Release Configuration  
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

add_subdirectory(src)
# add_subdirectory(test)
