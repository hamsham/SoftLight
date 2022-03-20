
# -------------------------------------
# Project Setup
# -------------------------------------
project(softlight VERSION 0.1.0 LANGUAGES CXX)

if (APPLE AND ${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.16.0")
    enable_language(OBJC)
    enable_language(OBJCXX)
endif()



# -------------------------------------
# Compiler Setup
# -------------------------------------
include(buildUtil.cmake)



# -------------------------------------
# OS Setup
# -------------------------------------
if (APPLE)
    set(PREFER_COCOA TRUE CACHE BOOL "Set preference to use the Cocoa window backend.")
endif()

if (PREFER_COCOA)
    sl_find_library(APPKIT_LIBRARY          AppKit         TRUE)
    sl_find_library(COCOA_LIBRARY           Cocoa          TRUE)
    sl_find_library(CORE_FOUNDATION_LIBRARY CoreFoundation TRUE)
    sl_find_library(CORE_GRAPHICS_LIBRARY   CoreGraphics   TRUE)
endif()

if (UNIX AND NOT MINGW AND NOT PREFER_COCOA)
    find_package(X11)

    find_file(XCB_XCB_HEADER   xcb/xcb.h       PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_PROTO_HEADER xcb/xproto.h    PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_IMAGE_HEADER xcb/xcb_image.h PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_SHM_HEADER   xcb/shm.h       PATHS ${CMAKE_INCLUDE_PATH})

    sl_find_library(XCB_SHM_LIBRARY xcb-shm FALSE)
    sl_find_library(XCB_LIBRARY     xcb     FALSE)
    sl_find_library(X11_XCB_LIBRARY X11-xcb FALSE)

    if (NOT X11_Xkb_FOUND)
        message(FATAL_ERROR "Unable to locate the X11 Xkb extension.")
    else()
        message("-- Found X11 Xkb: ${X11_Xkbfile_LIB}")
    endif()

    if (XCB_XCB_HEADER AND XCB_PROTO_HEADER AND XCB_LIBRARY)
        message("-- Found XCB: ${XCB_LIBRARY}")
        message("-- Found X11-XCB: ${X11_XCB_LIBRARY}")
        set(HAVE_XCB_BACKEND TRUE)

        if (NOT APPLE)
            if (XCB_IMAGE_HEADER AND XCB_SHM_HEADER AND XCB_SHM_LIBRARY)
                message("-- Looking for XCB XShm in ${XCB_SHM_LIBRARY} - found")
                option(ENABLE_XSHM "XCB Shared memory extension enabled." ON)
            else()
                message("-- Looking for X11 XShm - not found")
            endif()
        endif()
    endif()

    if (X11_FOUND)
        message("-- Found X11: ${X11_X11_LIB}")
        set(HAVE_X11_BACKEND TRUE)

        include_directories(BEFORE SYSTEM ${X11_INCLUDE_DIR})
        include_directories(BEFORE SYSTEM ${X11_X11_INCLUDE_PATH})

        if (NOT APPLE)
            if (X11_XShm_FOUND)
                message("-- Looking for X11 XShm in ${X11_Xext_LIB} - found")
                include_directories(BEFORE SYSTEM ${X11_XShm_INCLUDE_PATH})
                option(ENABLE_XSHM "Enable the use of shared memory for X11-based windows." ON)
            else()
                message("-- Looking for X11 XShm - not found")
            endif()
        endif()
    endif()

    if (NOT HAVE_XCB_BACKEND)
        message("Unable to locate XCB development libraries. Falling back to X11.")
        if (NOT HAVE_X11_BACKEND)
            message(FATAL_ERROR "Unable to locate X11 development libraries.")
        endif()
    endif()

    if (HAVE_XCB_BACKEND AND HAVE_X11_BACKEND)
        if (APPLE)
            set(PREFER_XCB TRUE CACHE BOOL "Set preference to use the X11-XCB window backend.")
        else()
            set(PREFER_XCB FALSE CACHE BOOL "Set preference to use the X11-XCB window backend.")
        endif()
    endif()

endif()



# -------------------------------------
# 3rd-party libraries
# -------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
include(assimpConfig.cmake)
include(FreeImageConfig.cmake)
include(freetypeConfig.cmake)



# -------------------------------------
# Source Paths
# -------------------------------------
set(SL_LIB_HEADERS
    include/softlight/SL_Animation.hpp
    include/softlight/SL_AnimationChannel.hpp
    include/softlight/SL_AnimationKeyList.hpp
    include/softlight/SL_AnimationPlayer.hpp
    include/softlight/SL_AnimationProperty.hpp
    include/softlight/SL_Atlas.hpp
    include/softlight/SL_BlitProcesor.hpp
    include/softlight/SL_BoundingBox.hpp
    include/softlight/SL_Camera.hpp
    include/softlight/SL_ClearProcesor.hpp
    include/softlight/SL_Color.hpp
    include/softlight/SL_ColorCompressed.hpp
    include/softlight/SL_ColorHSX.hpp
    include/softlight/SL_ColorYCoCg.hpp
    include/softlight/SL_Config.hpp
    include/softlight/SL_Context.hpp
    include/softlight/SL_Dither.hpp
    include/softlight/SL_FontLoader.hpp
    include/softlight/SL_FragmentProcessor.hpp
    include/softlight/SL_Framebuffer.hpp
    include/softlight/SL_Geometry.hpp
    include/softlight/SL_ImgFile.hpp
    include/softlight/SL_ImgFilePPM.hpp
    include/softlight/SL_IndexBuffer.hpp
    include/softlight/SL_KeySym.hpp
    include/softlight/SL_LineProcessor.hpp
    include/softlight/SL_LineRasterizer.hpp
    include/softlight/SL_Material.hpp
    include/softlight/SL_Mesh.hpp
    include/softlight/SL_Octree.hpp
    include/softlight/SL_PackedVertex.hpp
    include/softlight/SL_PipelineState.hpp
    include/softlight/SL_Plane.hpp
    include/softlight/SL_PointProcessor.hpp
    include/softlight/SL_PointRasterizer.hpp
    include/softlight/SL_ProcessorPool.hpp
    include/softlight/SL_Quadtree.hpp
    include/softlight/SL_RenderWindow.hpp
    include/softlight/SL_Sampler.hpp
    include/softlight/SL_ScanlineBounds.hpp
    include/softlight/SL_SceneFileLoader.hpp
    include/softlight/SL_SceneFileUtility.hpp
    include/softlight/SL_SceneGraph.hpp
    include/softlight/SL_SceneNode.hpp
    include/softlight/SL_Setup.hpp
    include/softlight/SL_Shader.hpp
    include/softlight/SL_ShaderUtil.hpp
    include/softlight/SL_ShaderProcessor.hpp
    include/softlight/SL_SpatialHierarchy.hpp
    include/softlight/SL_Swizzle.hpp
    include/softlight/SL_TextMeshLoader.hpp
    include/softlight/SL_Texture.hpp
    include/softlight/SL_Transform.hpp
    include/softlight/SL_TriProcessor.hpp
    include/softlight/SL_TriRasterizer.hpp
    include/softlight/SL_UniformBuffer.hpp
    include/softlight/SL_VertexArray.hpp
    include/softlight/SL_VertexBuffer.hpp
    include/softlight/SL_VertexCache.hpp
    include/softlight/SL_VertexProcessor.hpp
    include/softlight/SL_ViewportState.hpp
    include/softlight/SL_WindowBuffer.hpp
    include/softlight/SL_WindowEvent.hpp

    include/softlight/script/SL_SceneGraphScript.hpp
)

if (WIN32)
    set(SL_PLATFORM_HEADERS
        include/softlight/SL_KeySymWin32.hpp
        include/softlight/SL_RenderWindowWin32.hpp
        include/softlight/SL_WindowBufferWin32.hpp)

elseif (PREFER_COCOA)
    set(SL_PLATFORM_HEADERS
        include/softlight/SL_KeySymCocoa.hpp
        include/softlight/SL_RenderWindowCocoa.hpp
        include/softlight/SL_WindowBufferCocoa.hpp)

else()
    set(SL_PLATFORM_HEADERS
        include/softlight/SL_KeySymXlib.hpp
        include/softlight/SL_RenderWindowXCB.hpp
        include/softlight/SL_WindowBufferXCB.hpp
        include/softlight/SL_RenderWindowXlib.hpp
        include/softlight/SL_WindowBufferXlib.hpp)
endif()



set(SL_LIB_SOURCES
    src/SL_Animation.cpp
    src/SL_AnimationChannel.cpp
    src/SL_AnimationKeyList.cpp
    src/SL_AnimationPlayer.cpp
    src/SL_Atlas.cpp
    src/SL_BlitProcessor.cpp
    src/SL_BoundingBox.cpp
    src/SL_Camera.cpp
    src/SL_ClearProcessor.cpp
    src/SL_Color.cpp
    src/SL_Context.cpp
    src/SL_FontLoader.cpp
    src/SL_FragmentProcessor.cpp
    src/SL_Framebuffer.cpp
    src/SL_Geometry.cpp
    src/SL_ImgFile.cpp
    src/SL_ImgFilePPM.cpp
    src/SL_IndexBuffer.cpp
    src/SL_LineProcessor.cpp
    src/SL_LineRasterizer.cpp
    src/SL_Material.cpp
    src/SL_Mesh.cpp
    src/SL_PipelineState.cpp
    src/SL_PointProcessor.cpp
    src/SL_PointRasterizer.cpp
    src/SL_ProcessorPool.cpp
    src/SL_RenderWindow.cpp
    src/SL_SceneFileLoader.cpp
    src/SL_SceneFileUtility.cpp
    src/SL_SceneGraph.cpp
    src/SL_SceneNode.cpp
    src/SL_ShaderProcessor.cpp
    src/SL_SpatialHierarchy.cpp
    src/SL_TextMeshLoader.cpp
    src/SL_Texture.cpp
    src/SL_Transform.cpp
    src/SL_TriProcessor.cpp
    src/SL_TriRasterizer.cpp
    src/SL_UniformBuffer.cpp
    src/SL_VertexArray.cpp
    src/SL_VertexBuffer.cpp
    src/SL_VertexCache.cpp
    src/SL_VertexProcessor.cpp
    src/SL_ViewportState.cpp
    src/SL_WindowBuffer.cpp

    src/script/SL_SceneGraphScript.cpp
)

if(WIN32)
    message("-- Win32 window backend enabled.")
    set(SL_PLATFORM_SOURCES
        src/SL_KeySymWin32.cpp
        src/SL_RenderWindowWin32.cpp
        src/SL_WindowBufferWin32.cpp)

elseif (PREFER_COCOA)
    message("-- Cocoa window backend enabled.")
    add_definitions(-DSL_PREFER_COCOA)

    set(SL_PLATFORM_SOURCES
        src/SL_KeySymCocoa.mm
        src/SL_RenderWindowCocoa.mm
        src/SL_WindowBufferCocoa.mm)

elseif(UNIX)
    if (PREFER_XCB)
        add_definitions(-DSL_PREFER_XCB)
        message("-- X11-XCB window backend enabled.")
    else()
        add_definitions(-USR_PREFER_XCB)
        message("-- X11 window backend enabled.")
    endif()

    # X11-shm extensions can not be used on OSX due to shared memory
    # allocation limits
    if(NOT APPLE AND ENABLE_XSHM)
        add_definitions(-DSL_ENABLE_XSHM=1)
        message("-- X11 Shared memory extensions enabled for X11 windows.")
    else()
        add_definitions(-DSL_ENABLE_XSHM=0)
        message("-- X11 Shared memory extensions disabled.")
    endif()

    set(SL_PLATFORM_SOURCES
        src/SL_KeySymXlib.cpp
        src/SL_RenderWindowXCB.cpp
        src/SL_RenderWindowXlib.cpp
        src/SL_WindowBufferXCB.cpp
        src/SL_WindowBufferXlib.cpp)

else()
    message(FATAL_ERROR "Unknown/unsupported platform")
endif()



# -------------------------------------
# Library Setup
# -------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

if (BUILD_SHARED_LIBS)
    add_definitions(-DSL_BUILD_SHARED -DSL_SHARED)
    set(SL_LIB_TYPE SHARED)
else()
    set(SL_LIB_TYPE STATIC)
endif()

add_library(${PROJECT_NAME} ${SL_LIB_TYPE} ${SL_LIB_SOURCES} ${SL_PLATFORM_SOURCES} ${SL_LIB_HEADERS} ${SL_PLATFORM_HEADERS})

ls_configure_target(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        LightSky::Game
        LightSky::Script
        LightSky::Math
        LightSky::Utils
        LightSky::Setup
        ${ASSIMP_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
        ${FREETYPE_LIBRARIES}
)

if (NOT MSVC AND NOT PREFER_COCOA)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            ${XCB_LIBRARY}
            ${XCB_SHM_LIBRARY}
            ${X11_XCB_LIBRARY}
            ${X11_LIBRARIES}
            pthread
    )

elseif (PREFER_COCOA)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            "${APPKIT_LIBRARY}"
            "${COCOA_LIBRARY}"
            "${CORE_FOUNDATION_LIBRARY}"
            "${CORE_GRAPHICS_LIBRARY}"
    )

endif()

target_include_directories(${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_include_directories(${PROJECT_NAME} SYSTEM BEFORE
    PRIVATE ${FREEIMAGE_INCLUDE_PATH}
    PRIVATE ${FREETYPE_INCLUDE_DIRS}
)



# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS ${PROJECT_NAME}
    EXPORT SoftLight
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

install(DIRECTORY include/softlight DESTINATION include)

install(EXPORT SoftLight
    FILE SoftLight.cmake
    DESTINATION lib/cmake
)

export(EXPORT SoftLight)
add_library(SoftLight ALIAS ${PROJECT_NAME})



# -------------------------------------
# Precompiled Headers
# -------------------------------------
if (LIGHTSKY_ENABLE_PCH)
    # Can't have precompiled headers in a project if we're reusing them from
    # another project :(
    #target_precompile_headers(${PROJECT_NAME} PRIVATE ${SL_LIB_HEADERS})

    if (LS_UTILS_USE_PCH)
        target_precompile_headers(${PROJECT_NAME} REUSE_FROM lsutils)
    endif()

    if (LS_MATH_USE_PCH)
        target_precompile_headers(${PROJECT_NAME} REUSE_FROM lsmath)
    endif()
endif()



# -------------------------------------
# Library Tests
# -------------------------------------
option(SL_BUILD_TESTS "Build tests for the Soft Render library." ON)

if(SL_BUILD_TESTS)
    add_subdirectory(tests)
endif()
