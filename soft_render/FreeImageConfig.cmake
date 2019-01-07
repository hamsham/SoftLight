
find_package(FreeImage MODULE)



# #####################################
# External build for FreeImage
# #####################################
if (FREEIMAGE_INCLUDE_PATH STREQUAL FREEIMAGE_INCLUDE_PATH-NOTFOUND OR FREEIMAGE_LIBRARY STREQUAL FREEIMAGE_LIBRARY-NOTFOUND)

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
                ${CMAKE_MAKE_PROGRAM} /upgrade FreeImage.2013.sln &&
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
        set(FREEIMAGE_LIBRARIES freeimage ws2_32)
    else()
        set(FREEIMAGE_LIBRARIES freeimage)
    endif()



endif()
  
