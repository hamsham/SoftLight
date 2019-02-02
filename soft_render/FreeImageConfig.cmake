
find_package(FreeImage MODULE)



# #####################################
# External build for FreeImage
# #####################################
if (FREEIMAGE_INCLUDE_PATH STREQUAL FREEIMAGE_INCLUDE_PATH-NOTFOUND OR FREEIMAGE_LIBRARY STREQUAL FREEIMAGE_LIBRARY-NOTFOUND)

    set(FREEIMAGE_VERSION "3.18.0" CACHE STRING "Version of the FreeImage static library to be built.")
    mark_as_advanced(FREEIMAGE_VERSION)

    if (MSVC)
        set(FREEIMAGE_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}FreeImaged.lib)
        set(FREEIMAGE_DLL ${CMAKE_SHARED_LIBRARY_PREFIX}FreeImaged.dll)
        set(FREEIMAGE_LIB_TYPE SHARED)

        string(REPLACE "/" "\\" FREEIMAGE_TIF_CONFIG_PATH0 "${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Source/LibTIFF4/tif_config.vc.h")
        set(FREEIMAGE_PATCH_CMD0 findstr \/v \/C:snprintf "${FREEIMAGE_TIF_CONFIG_PATH0}" > ${FREEIMAGE_TIF_CONFIG_PATH0}.tmp && type ${FREEIMAGE_TIF_CONFIG_PATH0}.tmp > ${FREEIMAGE_TIF_CONFIG_PATH0})

        string(REPLACE "/" "\\" FREEIMAGE_TIF_CONFIG_PATH1 "${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Source/LibTIFF4/tif_config.h")
        set(FREEIMAGE_PATCH_CMD1 findstr \/v \/C:snprintf "${FREEIMAGE_TIF_CONFIG_PATH1}" > ${FREEIMAGE_TIF_CONFIG_PATH1}.tmp && type ${FREEIMAGE_TIF_CONFIG_PATH1}.tmp > ${FREEIMAGE_TIF_CONFIG_PATH1})
    else()
        set(FREEIMAGE_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}freeimage${CMAKE_STATIC_LIBRARY_SUFFIX})
        set(FREEIMAGE_LIB_TYPE STATIC)
    endif()

    # Build FreeImage
    if (MSVC)
        if (CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(FREEIMAGE_PLATFORM "x64")
            set(FREEIMAGE_DIST_DIR "x64")
        else()
            set(FREEIMAGE_PLATFORM "x86")
            set(FREEIMAGE_DIST_DIR "x32")
        endif()
      
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
                ${CMAKE_MAKE_PROGRAM} FreeImage.2013.sln /p:Platform=${FREEIMAGE_PLATFORM}
            BUILD_IN_SOURCE
                1
            INSTALL_COMMAND
                ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/${FREEIMAGE_LIB} ${EXTERNAL_PROJECT_PREFIX}/lib &&
                ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/${FREEIMAGE_DLL} ${EXTERNAL_PROJECT_PREFIX}/bin &&
                ${CMAKE_COMMAND} -E copy ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/FreeImage.h      ${EXTERNAL_PROJECT_PREFIX}/include
        )
    else()
        ExternalProject_Add(
            FreeImage
            PREFIX
                ${EXTERNAL_PROJECT_PREFIX}
            SVN_REPOSITORY
                "https://svn.code.sf.net/p/freeimage/svn/FreeImage/trunk"
            UPDATE_COMMAND
                ${Subversion_SVN_EXECUTABLE} update
            CONFIGURE_COMMAND
                ""
            BUILD_COMMAND
                make
            BUILD_IN_SOURCE
                1
            INSTALL_COMMAND
                ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_LIB} ${EXTERNAL_PROJECT_PREFIX}/lib &&
                ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/FreeImage.h      ${EXTERNAL_PROJECT_PREFIX}/include
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
  
