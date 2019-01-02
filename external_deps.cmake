
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



# #####################################
# External build for ASSIMP
# #####################################
set(ASSIMP_BRANCH "master" CACHE STRING "Git branch or tag for checking out Assimp.")
mark_as_advanced(ASSIMP_BRANCH)

if (MINGW)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -Wa,-mbig-obj")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wa,-mbig-obj")
endif()

# Configure build options
set(ASSIMP_BUILD_FLAGS
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_RC_COMPILER:FILEPATH=${CMAKE_RC_COMPILER}
    -DCMAKE_INSTALL_PREFIX:FILEPATH=${EXTERNAL_PROJECT_PREFIX}
    -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
    -DBUILD_SHARED_LIBS:BOOL=TRUE
    -DASSIMP_BUILD_ZLIB:BOOL=ON
    -DASSIMP_BUILD_TESTS:BOOL=OFF
    -DASSIMP_BUILD_SAMPLES:BOOL=OFF
    -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF)
mark_as_advanced(ASSIMP_BUILD_FLAGS)

# Build Assimp
ExternalProject_Add(
    Assimp
    PREFIX
        ${EXTERNAL_PROJECT_PREFIX}
    GIT_REPOSITORY
        "https://github.com/assimp/assimp.git"
    GIT_TAG
        "${ASSIMP_BRANCH}"
    UPDATE_COMMAND
        ${GIT_EXECUTABLE} fetch
    CMAKE_COMMAND
        ${CMAKE_COMMAND}
    CMAKE_ARGS
        ${ASSIMP_BUILD_FLAGS}
    CMAKE_CACHE_ARGS
        ${ASSIMP_BUILD_FLAGS}
    BUILD_COMMAND
        ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_PREFIX}/src/Assimp-build ${CMAKE_COMMAND} --build . --config ${CMAKE_CFG_INTDIR}
    INSTALL_DIR
        ${EXTERNAL_PROJECT_PREFIX}
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_PREFIX}/src/Assimp-build ${CMAKE_COMMAND} --build . --config ${CMAKE_CFG_INTDIR} --target install
)

# Add the imported library target
add_library(zlibstatic STATIC IMPORTED)
set_target_properties(zlibstatic PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}zlibstatic${CMAKE_STATIC_LIBRARY_SUFFIX})
add_dependencies(zlibstatic Assimp)

add_library(IrrXML STATIC IMPORTED)
set_target_properties(IrrXML PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}IrrXML${CMAKE_STATIC_LIBRARY_SUFFIX})
add_dependencies(IrrXML zlibstatic)

if (WIN32)
    if (MSVC)
        string(REGEX REPLACE "^v([0-9]+)[0-9]$" "vc\\10" ASSIMP_TOOLSET_VERSION "${CMAKE_VS_PLATFORM_TOOLSET}")
        set(ASSIMP_LIB "assimp-${ASSIMP_TOOLSET_VERSION}-mt")
        add_library(assimp STATIC IMPORTED)
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
    else()
        set(ASSIMP_LIB assimp.dll)
        add_library(assimp STATIC IMPORTED)
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
    endif()
else()
    set(ASSIMP_LIB assimp)
    add_library(assimp SHARED IMPORTED)
    set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
endif()

add_dependencies(assimp IrrXML)
set(ASSIMP_LIBS assimp)



# #####################################
# External build for GLM (for testing only)
# #####################################
set(GLM_BRANCH "master" CACHE STRING "Git branch or tag for checking out GLM.")
#set(GLM_BRANCH "0.9.9.2" CACHE STRING "Git branch or tag for checking out GLM.")
mark_as_advanced(GLM_BRANCH)

# Configure build options
set(GLM_BUILD_FLAGS
    -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
    -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
    -DCMAKE_C_FLAGS:STRING=${CMAKE_C_FLAGS}
    -DCMAKE_INSTALL_PREFIX:FILEPATH=${EXTERNAL_PROJECT_PREFIX}
    -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
    -DGLM_TEST_ENABLE:BOOL=OFF
    -DGLM_TEST_ENABLE_CXX_11:BOOL=ON
    -DGLM_TEST_ENABLE_CXX_14:BOOL=OFF
    -DGLM_TEST_ENABLE_CXX_17:BOOL=OFF
    -DGLM_TEST_ENABLE_CXX_20:BOOL=OFF
    -DGLM_TEST_ENABLE_LANG_EXTENSIONS:BOOL=OFF
    -DGLM_TEST_ENABLE_FAST_MATH:BOOL=OFF)
mark_as_advanced(GLM_BUILD_FLAGS)

# Compiler settings for GLM
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Build GLM
ExternalProject_Add(
    Glm
    PREFIX
        ${EXTERNAL_PROJECT_PREFIX}
    GIT_REPOSITORY
        "https://github.com/g-truc/glm.git"
    GIT_TAG
        "${GLM_BRANCH}"
    UPDATE_COMMAND
        ${GIT_EXECUTABLE} fetch
    CMAKE_COMMAND
        ${CMAKE_COMMAND}
    CMAKE_CACHE_ARGS
        ${GLM_BUILD_FLAGS}
    CMAKE_ARGS
        ${GLM_BUILD_FLAGS}
    BUILD_COMMAND
        ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_PREFIX}/src/Glm-build ${CMAKE_COMMAND} --build . --config ${CMAKE_CFG_INTDIR}
    INSTALL_DIR
        ${EXTERNAL_PROJECT_PREFIX}
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E chdir ${EXTERNAL_PROJECT_PREFIX}/src/Glm-build ${CMAKE_COMMAND} --build . --config ${CMAKE_CFG_INTDIR} --target install
)



# #####################################
# External build for FreeImage
# #####################################
set(FREEIMAGE_BRANCH "master" CACHE STRING "Git branch or tag for checking out FreeImage.")
mark_as_advanced(FREEIMAGE_BRANCH)

if (MSVC)
    set(FREEIMAGE_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}FreeImaged.lib)
    set(FREEIMAGE_DLL ${CMAKE_SHARED_LIBRARY_PREFIX}FreeImaged.dll)
    set(FREEIMAGE_LIB_TYPE STATIC)

    string(REPLACE "/" "\\" FREEIMAGE_TIF_CONFIG_PATH0 "${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Source/LibTIFF4/tif_config.vc.h")
    set(FREEIMAGE_PATCH_CMD0 findstr \/v \/C:snprintf "${FREEIMAGE_TIF_CONFIG_PATH0}" > ${FREEIMAGE_TIF_CONFIG_PATH0}.tmp && type ${FREEIMAGE_TIF_CONFIG_PATH0}.tmp > ${FREEIMAGE_TIF_CONFIG_PATH0})

    string(REPLACE "/" "\\" FREEIMAGE_TIF_CONFIG_PATH1 "${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Source/LibTIFF4/tif_config.h")
    set(FREEIMAGE_PATCH_CMD1 findstr \/v \/C:snprintf "${FREEIMAGE_TIF_CONFIG_PATH1}" > ${FREEIMAGE_TIF_CONFIG_PATH1}.tmp && type ${FREEIMAGE_TIF_CONFIG_PATH1}.tmp > ${FREEIMAGE_TIF_CONFIG_PATH1})
else()
    if (MINGW)
        set(FREEIMAGE_LIB_TYPE STATIC)
        set(FREEIMAGE_LIB ${CMAKE_SHARED_LIBRARY_PREFIX}FreeImage${CMAKE_STATIC_LIBRARY_SUFFIX})
        add_definitions(-D FREEIMAGE_LIB)
    else()
    set(FREEIMAGE_LIB_TYPE SHARED)
        set(FREEIMAGE_LIB ${CMAKE_SHARED_LIBRARY_PREFIX}freeimage${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()
endif()

# Build FreeImage
if (MSVC)
    ExternalProject_Add(
        FreeImage
        PREFIX
            ${EXTERNAL_PROJECT_PREFIX}
        GIT_REPOSITORY
            "https://github.com/MonoGame/FreeImage.git"
        GIT_TAG
            "${FREEIMAGE_BRANCH}"
        UPDATE_COMMAND
            ${GIT_EXECUTABLE} fetch
        CONFIGURE_COMMAND
            ""
        PATCH_COMMAND
            ${FREEIMAGE_PATCH_CMD0} && ${FREEIMAGE_PATCH_CMD1}
        BUILD_COMMAND
            ${CMAKE_VS_DEVENV_COMMAND} /upgrade FreeImage.2013.sln &&
            ${CMAKE_MAKE_PROGRAM} FreeImage.2013.sln
        BUILD_IN_SOURCE
            1
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/x32/${FREEIMAGE_LIB} ${EXTERNAL_PROJECT_PREFIX}/lib &&
            ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/x32/${FREEIMAGE_DLL} ${EXTERNAL_PROJECT_PREFIX}/bin &&
            ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/x32/FreeImage.h      ${EXTERNAL_PROJECT_PREFIX}/include
    )
else()
    configure_file(${PROJECT_SOURCE_DIR}/build_freeimage.sh ${EXTERNAL_PROJECT_PREFIX}/build_freeimage.sh @ONLY)

    ExternalProject_Add(
        FreeImage
        PREFIX
            ${EXTERNAL_PROJECT_PREFIX}
        GIT_REPOSITORY
            "https://github.com/MonoGame/FreeImage.git"
        GIT_TAG
            "${FIMG_BRANCH}"
        UPDATE_COMMAND
            ${GIT_EXECUTABLE} fetch
        CONFIGURE_COMMAND
            bash ${EXTERNAL_PROJECT_PREFIX}/build_freeimage.sh --configure
        BUILD_COMMAND
            bash ${EXTERNAL_PROJECT_PREFIX}/build_freeimage.sh --make
        BUILD_IN_SOURCE
            1
        INSTALL_COMMAND
            bash ${EXTERNAL_PROJECT_PREFIX}/build_freeimage.sh --install
    )
endif()

# Add the imported library target
add_library(freeimage ${FREEIMAGE_LIB_TYPE} IMPORTED)
set_target_properties(freeimage PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${FREEIMAGE_LIB})
add_dependencies(freeimage FreeImage)

if (WIN32)
    set(FREEIMAGE_LIBS freeimage ws2_32)
else()
    set(FREEIMAGE_LIBS freeimage)
endif()

