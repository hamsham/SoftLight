
# #####################################
# CMake Setup
# #####################################
cmake_minimum_required(VERSION 3.2)
project(SoftLight)

set(CMAKE_CXX_STANDARD 11)



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

add_subdirectory(soft_render)
include_directories(soft_render/include)
