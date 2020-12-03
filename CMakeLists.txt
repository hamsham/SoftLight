
# #####################################
# CMake Setup
# #####################################
cmake_minimum_required(VERSION 3.9)
project(SoftLight)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)



# #####################################
# 3rd-party libraries
# #####################################
include(external_deps.cmake)



# #####################################
# Internal Libraries
# #####################################
add_subdirectory(light_setup)
add_subdirectory(light_utils)
add_subdirectory(light_game)
add_subdirectory(light_math)
add_subdirectory(softlight)
