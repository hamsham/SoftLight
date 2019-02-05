
find_package(FreeImage MODULE)



# #####################################
# External build for FreeImage
# #####################################
if (FREEIMAGE_INCLUDE_PATH STREQUAL FREEIMAGE_INCLUDE_PATH-NOTFOUND OR FREEIMAGE_LIBRARY STREQUAL FREEIMAGE_LIBRARY-NOTFOUND)

    message("-- Building FreeImage from source")
  
    set(FREEIMAGE_VERSION "3.18.0" CACHE STRING "Version of the FreeImage static library to be built.")
    mark_as_advanced(FREEIMAGE_VERSION)

    if (MSVC)
        set(FREEIMAGE_LIB ${CMAKE_STATIC_LIBRARY_PREFIX}FreeImage${CMAKE_STATIC_LIBRARY_SUFFIX})
        set(FREEIMAGE_DLL ${CMAKE_SHARED_LIBRARY_PREFIX}FreeImage${CMAKE_SHARED_LIBRARY_SUFFIX})
        set(FREEIMAGE_LIB_TYPE SHARED)
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
            set(FREEIMAGE_PLATFORM "Win32")
            set(FREEIMAGE_DIST_DIR "x32")
        endif()
        
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
                ${CMAKE_MAKE_PROGRAM} FreeImage.2017.sln /p:Configuration=Release /p:Platform=${FREEIMAGE_PLATFORM}
            BUILD_IN_SOURCE
                1
            INSTALL_COMMAND
                ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/${FREEIMAGE_LIB} ${EXTERNAL_PROJECT_PREFIX}/lib &&
                ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/${FREEIMAGE_DLL} ${EXTERNAL_PROJECT_PREFIX}/bin &&
                ${CMAKE_COMMAND} -E copy_if_different ${EXTERNAL_PROJECT_PREFIX}/src/FreeImage/Dist/${FREEIMAGE_DIST_DIR}/FreeImage.h      ${EXTERNAL_PROJECT_PREFIX}/include
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
        set(FREEIMAGE_LIBRARIES ${EXTERNAL_PROJECT_PREFIX}/lib/${FREEIMAGE_LIB} ws2_32)
    else()
        set(FREEIMAGE_LIBRARIES freeimage)
    endif()



endif()
  
