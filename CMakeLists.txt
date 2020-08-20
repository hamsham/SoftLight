
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
include_directories(light_setup/include)

add_subdirectory(light_utils)
include_directories(light_utils/include)

add_subdirectory(light_game)
include_directories(light_game/include)

add_subdirectory(light_math)
include_directories(light_math/include)

add_subdirectory(softlight)
include_directories(softlight/include)
