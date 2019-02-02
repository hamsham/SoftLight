
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 11)


# #####################################
# External Project Setup
# #####################################
# Include the CMake module to support the builds of 3rd-party dependencies
include(ExternalProject)

# Sub-directories within the build folder (not installation folder).
set(EXTERNAL_PROJECT_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/external" CACHE STRING "Build prefix for 3rd-party libraries.")
mark_as_advanced(EXTERNAL_PROJECT_PREFIX)

# Include directory for 3rd-party libraries
include_directories(BEFORE SYSTEM ${EXTERNAL_PROJECT_PREFIX}/include)



# #####################################
# Folders for the external data
# #####################################
file(MAKE_DIRECTORY ${EXTERNAL_PROJECT_PREFIX})
file(MAKE_DIRECTORY ${EXTERNAL_PROJECT_PREFIX}/include)
file(MAKE_DIRECTORY ${EXTERNAL_PROJECT_PREFIX}/lib)
file(MAKE_DIRECTORY ${EXTERNAL_PROJECT_PREFIX}/bin)



# #####################################
# Version Control Tools
# #####################################
find_package(Git REQUIRED)
find_package(Subversion REQUIRED)
