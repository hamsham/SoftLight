
find_package(assimp MODULE)



# #####################################
# External build for ASSIMP
# #####################################
option(BUILD_ASSIMP "Force ASSIMP to build from source." OFF)

if (BUILD_ASSIMP OR NOT ASSIMP_INCLUDE_DIR OR NOT ASSIMP_LIBRARIES)

    message("-- Building ASSIMP from source")
  
    set(ASSIMP_BRANCH "master" CACHE STRING "Git branch or tag for checking out Assimp.")
    mark_as_advanced(ASSIMP_BRANCH)

    if (MINGW)
        set(ASSIMP_C_FLAGS "${CMAKE_C_FLAGS} -Os -Wa,-mbig-obj")
        set(ASSIMP_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Os -Wa,-mbig-obj")
    elseif (MSVC)
        set(ASSIMP_C_FLAGS "${CMAKE_C_FLAGS} /p:CharacterSet=Unicode")
        set(ASSIMP_CXX_FLAGS "${CMAKE_CXX_FLAGS} /p:CharacterSet=Unicode")
    else()
        set(ASSIMP_C_FLAGS "${CMAKE_C_FLAGS}")
        set(ASSIMP_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()

    mark_as_advanced(ASSIMP_CXX_FLAGS)
    mark_as_advanced(ASSIMP_C_FLAGS)

    # Configure build options
    set(ASSIMP_BUILD_FLAGS
        -DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}
        -DCMAKE_CXX_FLAGS:STRING=${ASSIMP_CXX_FLAGS}
        -DCMAKE_C_COMPILER:STRING=${CMAKE_C_COMPILER}
        -DCMAKE_C_FLAGS:STRING=${ASSIMP_C_FLAGS}
        -DCMAKE_RC_COMPILER:FILEPATH=${CMAKE_RC_COMPILER}
        -DCMAKE_INSTALL_PREFIX:FILEPATH=${EXTERNAL_PROJECT_PREFIX}
        -DCMAKE_SYSTEM_NAME:STRING=${CMAKE_SYSTEM_NAME}
        -DCMAKE_MAKE_PROGRAM:PATH=${CMAKE_MAKE_PROGRAM}
        -DBUILD_SHARED_LIBS:BOOL=TRUE
        -DASSIMP_BUILD_ZLIB:BOOL=ON
        -DASSIMP_BUILD_TESTS:BOOL=OFF
        -DASSIMP_BUILD_SAMPLES:BOOL=OFF
        -DASSIMP_BUILD_ASSIMP_TOOLS:BOOL=OFF
        -DASSIMP_BUILD_NO_EXPORT:BOOL=ON
        -DASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT:BOOL=OFF
        -DASSIMP_INSTALL_PDB:BOOL=FALSE
        -DASSIMP_WARNINGS_AS_ERRORS:BOOL=OFF
        -DASSIMP_BUILD_ASSIMP_VIEW:BOOL=OFF
    )
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
        GIT_SHALLOW
            TRUE
        GIT_PROGRESS
            TRUE
        GIT_REMOTE_UPDATE_STRATEGY
            CHECKOUT
        UPDATE_COMMAND
            ${GIT_EXECUTABLE} pull origin ${ASSIMP_BRANCH}
        CMAKE_GENERATOR
            "${CMAKE_GENERATOR}"
        CMAKE_COMMAND
            ${CMAKE_COMMAND}
        CMAKE_CACHE_ARGS
            ${ASSIMP_BUILD_FLAGS}
        INSTALL_DIR
            ${EXTERNAL_PROJECT_PREFIX}
        LOG_BUILD
            TRUE
        STEP_TARGETS
            lib
            utf8cpp
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
            # These were extracted from the ASSIMP repo and should remain updated.
            if(MSVC70 OR MSVC71)
                set(MSVC_PREFIX "vc70")
            elseif(MSVC80)
                set(MSVC_PREFIX "vc80")
            elseif(MSVC90)
                set(MSVC_PREFIX "vc90")
            elseif(MSVC10)
                set(MSVC_PREFIX "vc100")
            elseif(MSVC11)
                set(MSVC_PREFIX "vc110")
            elseif(MSVC12)
                set(MSVC_PREFIX "vc120")
            elseif(MSVC_VERSION LESS 1910)
                set(MSVC_PREFIX "vc140")
            elseif(MSVC_VERSION LESS 1920)
                set(MSVC_PREFIX "vc141")
            elseif(MSVC_VERSION LESS 1930)
                set(MSVC_PREFIX "vc142")
            elseif(MSVC_VERSION LESS 1940)
                set(MSVC_PREFIX "vc143")
            else()
                MESSAGE(WARNING "unknown msvc version ${MSVC_VERSION}")
                set(MSVC_PREFIX "vc150")
            endif()

            # Not a command-line build
            if (NOT CMAKE_VS_MSBUILD_COMMAND)
                set(ASSIMP_LIB_SUFFIX "d")
            endif()

            set(ASSIMP_LIB "assimp-${MSVC_PREFIX}-mt${ASSIMP_LIB_SUFFIX}")
            add_library(assimp STATIC IMPORTED)
            set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
        else()
            set(ASSIMP_LIB assimp.dll)
            add_library(assimp STATIC IMPORTED)
            set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})
        endif()

        ExternalProject_Add_Step(Assimp lib BYPRODUCTS ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_STATIC_LIBRARY_SUFFIX})

    else()
        set(ASSIMP_LIB assimp)
        add_library(assimp SHARED IMPORTED)
        set_target_properties(assimp PROPERTIES IMPORTED_LOCATION ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
        ExternalProject_Add_Step(Assimp lib BYPRODUCTS ${EXTERNAL_PROJECT_PREFIX}/lib/${CMAKE_SHARED_LIBRARY_PREFIX}${ASSIMP_LIB}${CMAKE_SHARED_LIBRARY_SUFFIX})
    endif()

    # Assimp needs its own 3rd-party dependency installed
    ExternalProject_Add_Step(
        Assimp
        utf8cpp
        COMMAND
            ${CMAKE_COMMAND} -E make_directory "${EXTERNAL_PROJECT_PREFIX}/include/contrib/utf8cpp/"
        COMMAND
            ${CMAKE_COMMAND} -E create_symlink "${EXTERNAL_PROJECT_PREFIX}/src/Assimp/contrib/utf8cpp/source" "${EXTERNAL_PROJECT_PREFIX}/include/contrib/utf8cpp/source"
        BYPRODUCTS
            ${EXTERNAL_PROJECT_PREFIX}/include/contrib/utf8cpp/source
            ${EXTERNAL_PROJECT_PREFIX}/include/contrib/utf8cpp/
            ${EXTERNAL_PROJECT_PREFIX}/include/contrib/
        LOG
            TRUE
    )

    add_dependencies(assimp IrrXML utf8cpp)
    set(ASSIMP_LIBRARIES assimp)
endif ()
