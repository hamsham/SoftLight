
find_package(assimp MODULE)



# #####################################
# External build for ASSIMP
# #####################################
if (ASSIMP_INCLUDE_DIR STREQUAL ASSIMP_INCLUDE_DIR-NOTFOUND OR ASSIMP_LIBRARIES STREQUAL ASSIMP_LIBRARIES-NOTFOUND)

    message("-- Building ASSIMP from source")
  
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
    set(ASSIMP_LIBRARIES assimp)



endif ()
