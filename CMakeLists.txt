
# #####################################
# CMake Setup
# #####################################
cmake_minimum_required(VERSION 3.2)
project(SoftLight)

set(CMAKE_CXX_STANDARD 11)



if (CMAKE_BUILD_TYPE)
    string(TOLOWER ${CMAKE_BUILD_TYPE} _CURRENT_BUILD_TYPE)
    if(${_CURRENT_BUILD_TYPE} STREQUAL debug OR ${_CURRENT_BUILD_TYPE} STREQUAL relwithdebinfo)
        message("-- Debug mode enabled.")
        add_definitions(-D LS_DEBUG)
    else()
        message("-- Release mode enabled.")
    endif()
endif()



# #####################################
# Build Options
# #####################################
option(ENABLE_X86_OPTIMIZATIONS "Enable x86-specific compiler optimization flags." ON)
option(ENABLE_ARM_OPTIMIZATIONS "Enable arm-specific compiler optimization flags." OFF)

option(ENABLE_PROFILING "Enable profiling (recommended for release builds with debug information)." OFF)



# #####################################
# Compiler Options
# #####################################
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions(-Wall)
    add_definitions(-Werror)
    add_definitions(-Wextra)
    add_definitions(-pedantic)
    add_definitions(-pedantic-errors)
    add_definitions(-Wno-implicit-fallthrough)
    add_definitions(-Wno-pessimizing-move)
    add_definitions(-ftree-vectorize)
    add_definitions(-ffast-math)
    add_definitions(-funsafe-math-optimizations)
    add_definitions(-fno-stack-protector)

    if (ENABLE_PROFILING)
        message("-- Profiling enabled")
        add_definitions(-Og -pg)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg -Og") # failsafe
    else()
        add_definitions(-fomit-frame-pointer)
    endif()

    if (ENABLE_ARM_OPTIMIZATIONS)
        message("-- ARM compiler optimizations enabled.")
        add_definitions(-mhard-float)
        add_definitions(-mfloat-abi=hard)
        add_definitions(-mfpu=neon-vfpv4)
        add_definitions(-march=armv7-a)
        add_definitions(-faligned-new)
        add_definitions(-mno-unaligned-access)
    endif()

    if (ENABLE_X86_OPTIMIZATIONS)
        message("-- x86 compiler optimizations enabled.")
        add_definitions(-mmmx)
        add_definitions(-msse)
        add_definitions(-msse2)
        add_definitions(-msse3)
        add_definitions(-mssse3)
        add_definitions(-msse4)
        add_definitions(-msse4.1)
        add_definitions(-msse4.2)
        add_definitions(-mfma)
        add_definitions(-mavx)
        add_definitions(-mavx2)

        if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            add_definitions(-mfpmath=both)
            #add_definitions(-march=core2)
        endif()
    endif()
    
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if (ENABLE_X86_OPTIMIZATIONS)
        add_definitions(/arch:AVX2) # enable AVX2
    endif()
    
    add_definitions(/GL) # global program optimization
    add_definitions(/Oi) # enable instrinsic functions
    add_definitions(/Ot) # favor fast code
    add_definitions(/Oy) # Omit frame pointers
    add_definitions(/permissive-) # enable conformance mode
    add_definitions(/Wall) # enable all warnings
    add_definitions(/sdl-) # Disable Security Development Lifecycle checks
    add_definitions(/GS-) # Disable global security cookies
    add_definitions(/W3) # production-quality warnings
    add_definitions(/WX) # warnings are errors
    add_definitions(/Zc:rvalueCast /Zc:strictStrings /Zc:inline)

    # Prevent libraries from being rebuilt
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    /LTCG:INCREMENTAL")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /LTCG:INCREMENTAL")
    set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /LTCG:INCREMENTAL")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /LTCG:INCREMENTAL")

    # Extra enabled warnings
    add_definitions(/we4700) # Enable check for use of uninitialized variables
    add_definitions(/we4533) # Code initializing a variable will not be executed

    # Disabled warnings
    add_definitions(/wd4146) # unsigned negation using the '-' operator
    add_definitions(/wd4996) # insecure use of wcstombs
    add_definitions(/wd4365) # Implicit conversion from signed to unsigned (built into the standard library :/ )
    add_definitions(/wd4820) # Padding being added to structures using "alignas"... also built into the standard libary
    add_definitions(/wd4571) # Catch semantics changed
    add_definitions(/wd4625) # Implicly deleted copy constructor
    add_definitions(/wd4626) # Implicly deleted copy operator
    add_definitions(/wd5026) # Implicly deleted move constructor
    add_definitions(/wd5027) # Implicly deleted move operator
    add_definitions(/wd4774) # Format string is not a string literal in sprintf_s
    add_definitions(/wd4868) # compiler may not enforce left-to-right evaluation order in braced initializer list
    add_definitions(/wd5039) # An 'extern "C"' function contains something which might throw an exception
    add_definitions(/wd4244) # possible loss of data when converting between time_t and unsigned int
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Intel")
    add_definitions(-xAVX2)
    add_definitions(-msse)
    add_definitions(-msse2)
    add_definitions(-msse3)
    add_definitions(-mavx2)
    add_definitions(-mtune=core2)
    add_definitions(-march=core2)
    add_definitions(-ip)
    add_definitions(-vecabi=gcc)
    add_definitions(-fp-model fast=2)
    add_definitions(-finline)
    add_definitions(-gcc)
else()
    message(FATAL_ERROR "Your compiler is currently unsupported: ${CMAKE_CXX_COMPILER_ID}")
endif()



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

add_subdirectory(light_math)
include_directories(light_math/include)

add_subdirectory(soft_render)
include_directories(soft_render/include)
