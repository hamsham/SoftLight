
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ASSIMP_ARCHITECTURE "64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ASSIMP_ARCHITECTURE "32")
endif()

if(MSVC)
    set(ASSIMP_ROOT_DIR CACHE PATH "ASSIMP root directory")

    # Find path of each library
    find_path(ASSIMP_INCLUDE_DIR
        NAMES
            assimp/anim.h
        HINTS
            ${ASSIMP_ROOT_DIR}/include
    )

    if(MSVC12)
        set(ASSIMP_MSVC_VERSION "vc120")
    elseif(MSVC14)
        set(ASSIMP_MSVC_VERSION "vc140")
    endif()

    if(MSVC12 OR MSVC14)

        find_path(ASSIMP_LIBRARY_DIR
            NAMES
                assimp-${ASSIMP_MSVC_VERSION}-mt.lib
            HINTS
                ${ASSIMP_ROOT_DIR}/lib${ASSIMP_ARCHITECTURE}
        )

        find_library(ASSIMP_LIBRARY_RELEASE
            assimp-${ASSIMP_MSVC_VERSION}-mt.lib
            PATHS
                ${ASSIMP_LIBRARY_DIR})

        find_library(ASSIMP_LIBRARY_DEBUG
            assimp-${ASSIMP_MSVC_VERSION}-mtd.lib
            PATHS
                ${ASSIMP_LIBRARY_DIR})

        set(ASSIMP_LIBRARY
            optimized 	${ASSIMP_LIBRARY_RELEASE}
            debug		${ASSIMP_LIBRARY_DEBUG}
            PARENT_SCOPE
        )

        set(ASSIMP_LIBRARIES "ASSIMP_LIBRARY_RELEASE" "ASSIMP_LIBRARY_DEBUG" PARENT_SCOPE)

        function(ASSIMP_COPY_BINARIES TargetDirectory)
            add_custom_target(AssimpCopyBinaries
                COMMAND
                    ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll ${TargetDirectory}/Debug/assimp-${ASSIMP_MSVC_VERSION}-mtd.dll
                COMMAND
                    ${CMAKE_COMMAND} -E copy ${ASSIMP_ROOT_DIR}/bin${ASSIMP_ARCHITECTURE}/assimp-${ASSIMP_MSVC_VERSION}-mt.dll ${TargetDirectory}/Release/assimp-${ASSIMP_MSVC_VERSION}-mt.dll
            COMMENT
                "Copying Assimp binaries to '${TargetDirectory}'"
            VERBATIM)
        endfunction()

    endif()

else()

    find_path(
      ASSIMP_INCLUDE_DIR
      NAMES
        assimp/postprocess.h
        assimp/scene.h
        assimp/version.h
        assimp/config.h
        assimp/cimport.h
      PATHS
        /usr/include
        /usr/local/include
        /sw/include
        /opt/local/include
    )

    find_library(
      ASSIMP_LIBRARIES
      NAMES
        assimp
        assimp.dll
      PATHS
        /usr/lib64
        /usr/lib
        /usr/local/lib64
        /usr/local/lib
        /sw/lib
        /opt/local/lib
    )
    if(ASSIMP_INCLUDE_DIR AND ASSIMP_LIBRARIES)
        message("-- Found ASSIMP: ${ASSIMP_LIBRARIES}")
        set(ASSIMP_FOUND TRUE PARENT_SCOPE)

    else()
      if(ASSIMP_FIND_REQUIRED)
        message(FATAL_ERROR "Could not find asset importer library")
      endif()
    endif()

endif()

