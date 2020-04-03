
# -------------------------------------
# Project Setup
# -------------------------------------
project(soft_render VERSION 0.1.0 LANGUAGES CXX)



# -------------------------------------
# Compiler Setup
# -------------------------------------
# MinGW doesn't always play nice with the Windows SDK
if (MINGW)
	set(CMAKE_SYSTEM_NAME Windows)
	
	# These defines were made in Windows 2000
	add_definitions(-D WINVER=0x0602)
	add_definitions(-D_WIN32_WINNT=0x0602)
endif()

include(buildUtil.cmake)



# -------------------------------------
# OS Setup
# -------------------------------------
if (APPLE)
    set(PREFER_COCOA TRUE CACHE BOOL "Set preference to use the Cocoa window backend.")
endif()

if (PREFER_COCOA)
    sr_find_library(APPKIT_LIBRARY          AppKit         TRUE)
    sr_find_library(COCOA_LIBRARY           Cocoa          TRUE)
    sr_find_library(CORE_FOUNDATION_LIBRARY CoreFoundation TRUE)
    sr_find_library(CORE_GRAPHICS_LIBRARY   CoreGraphics   TRUE)
endif()

if (UNIX AND NOT MINGW AND NOT PREFER_COCOA)
    find_package(X11 REQUIRED)
    if (NOT X11_FOUND)
        message(FATAL_ERROR "Unable to locate the X11 library.")
    else()
        message("-- Found X11: ${X11_X11_LIB}")
        include_directories(BEFORE SYSTEM ${X11_INCLUDE_DIR})
        include_directories(BEFORE SYSTEM ${X11_X11_INCLUDE_PATH})
    endif()

    if (NOT X11_Xkb_FOUND)
        message(FATAL_ERROR "Unable to locate the X11 Xkb extension.")
    else()
        message("-- Found X11 Xkb: ${X11_Xkbfile_LIB}")
    endif()

    if (NOT X11_XShm_FOUND)
        message(FATAL_ERROR "Unable to locate the X11 Shared Memory extension.")
    else()
        message("-- Found X11 XShm: ${X11_Xext_LIB}")
        include_directories(BEFORE SYSTEM ${X11_XShm_INCLUDE_PATH})
    endif()

    find_file(XCB_XCB_HEADER   xcb/xcb.h       PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_PROTO_HEADER xcb/xproto.h    PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_IMAGE_HEADER xcb/xcb_image.h PATHS ${CMAKE_INCLUDE_PATH})
    find_file(XCB_SHM_HEADER   xcb/shm.h       PATHS ${CMAKE_INCLUDE_PATH})

    sr_find_library(XCB_LIBRARY     xcb     FALSE)
    sr_find_library(X11_XCB_LIBRARY X11-xcb FALSE)
    sr_find_library(XCB_SHM_LIBRARY xcb-shm FALSE)

    if (NOT XCB_XCB_HEADER OR NOT XCB_PROTO_HEADER OR NOT XCB_LIBRARY OR NOT X11_XCB_LIBRARY)
        message("-- XCB Not found. Falling back to X11")
        set(PREFER_XCB FALSE CACHE BOOL "Set preference to use the X11-XCB window backend.")
    else()
        message("-- Found XCB: ${XCB_LIBRARY}")
        message("-- Found X11-XCB: ${X11_XCB_LIBRARY}")

        if (APPLE)
            set(PREFER_XCB TRUE CACHE BOOL "Set preference to use the X11-XCB window backend.")
        else()
            set(PREFER_XCB FALSE CACHE BOOL "Set preference to use the X11-XCB window backend.")
        endif()
    endif()

    if (XCB_IMAGE_HEADER AND XCB_SHM_HEADER AND XCB_SHM_LIBRARY)
        message("-- Found XCB-SHM: ${XCB_SHM_LIBRARY}")
        set(ENABLE_XCB_SHM TRUE CACHE BOOL "XCB Shared memory extension enabled.")
    else()
        set(ENABLE_XCB_SHM TRUE CACHE BOOL "XCB Shared memory extension disabled.")
    endif()
else()
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()



# -------------------------------------
# 3rd-party libraries
# -------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
include(assimpConfig.cmake)
include(FreeImageConfig.cmake)



# -------------------------------------
# Source Paths
# -------------------------------------
include_directories(include)



set(SR_LIB_HEADERS
    include/soft_render/SR_Animation.hpp
    include/soft_render/SR_AnimationChannel.hpp
    include/soft_render/SR_AnimationKeyList.hpp
    include/soft_render/SR_AnimationPlayer.hpp
    include/soft_render/SR_AnimationProperty.hpp
    include/soft_render/SR_BlitProcesor.hpp
    include/soft_render/SR_BoundingBox.hpp
    include/soft_render/SR_Camera.hpp
    include/soft_render/SR_Color.hpp
    include/soft_render/SR_Config.hpp
    include/soft_render/SR_Context.hpp
    include/soft_render/SR_FragmentProcessor.hpp
    include/soft_render/SR_Framebuffer.hpp
    include/soft_render/SR_Geometry.hpp
    include/soft_render/SR_ImgFile.hpp
    include/soft_render/SR_ImgFilePPM.hpp
    include/soft_render/SR_IndexBuffer.hpp
    include/soft_render/SR_KeySym.hpp
    include/soft_render/SR_Material.hpp
    include/soft_render/SR_Mesh.hpp
    include/soft_render/SR_PackedVertex.hpp
    include/soft_render/SR_Plane.hpp
    include/soft_render/SR_ProcessorPool.hpp
    include/soft_render/SR_RenderWindow.hpp
    include/soft_render/SR_Sampler.hpp
    include/soft_render/SR_ScanlineBounds.hpp
    include/soft_render/SR_SceneFileLoader.hpp
    include/soft_render/SR_SceneFileUtility.hpp
    include/soft_render/SR_SceneGraph.hpp
    include/soft_render/SR_SceneNode.hpp
    include/soft_render/SR_Setup.hpp
    include/soft_render/SR_Shader.hpp
    include/soft_render/SR_ShaderUtil.hpp
    include/soft_render/SR_ShaderProcessor.hpp
    include/soft_render/SR_Swizzle.hpp
    include/soft_render/SR_Texture.hpp
    include/soft_render/SR_TexSampler.hpp
    include/soft_render/SR_Transform.hpp
    include/soft_render/SR_UniformBuffer.hpp
    include/soft_render/SR_VertexArray.hpp
    include/soft_render/SR_VertexBuffer.hpp
    include/soft_render/SR_VertexProcessor.hpp
    include/soft_render/SR_WindowBuffer.hpp
    include/soft_render/SR_WindowEvent.hpp)

if (WIN32)
    set(SR_LIB_HEADERS
        ${SR_LIB_HEADERS}
        include/soft_render/SR_KeySymWin32.hpp
        include/soft_render/SR_RenderWindowWin32.hpp
        include/soft_render/SR_WindowBufferWin32.hpp)

elseif (PREFER_COCOA)
    set(SR_LIB_HEADERS
        ${SR_LIB_HEADERS}
        include/soft_render/SR_KeySymCocoa.hpp
        include/soft_render/SR_RenderWindowCocoa.hpp
        include/soft_render/SR_WindowBufferCocoa.hpp)

else()
    set(SR_LIB_HEADERS
        ${SR_LIB_HEADERS}
        include/soft_render/SR_KeySymXlib.hpp
        include/soft_render/SR_RenderWindowXCB.hpp
        include/soft_render/SR_WindowBufferXCB.hpp
        include/soft_render/SR_RenderWindowXlib.hpp
        include/soft_render/SR_WindowBufferXlib.hpp)
endif()



set(SR_LIB_SOURCES
    src/SR_Animation.cpp
    src/SR_AnimationChannel.cpp
    src/SR_AnimationKeyList.cpp
    src/SR_AnimationPlayer.cpp
    src/SR_BlitProcessor.cpp
    src/SR_BoundingBox.cpp
    src/SR_Camera.cpp
    src/SR_Color.cpp
    src/SR_Context.cpp
    src/SR_FragmentProcessor.cpp
    src/SR_Framebuffer.cpp
    src/SR_Geometry.cpp
    src/SR_ImgFile.cpp
    src/SR_ImgFilePPM.cpp
    src/SR_IndexBuffer.cpp
    src/SR_Material.cpp
    src/SR_Mesh.cpp
    src/SR_ProcessorPool.cpp
    src/SR_RenderWindow.cpp
    src/SR_SceneFileLoader.cpp
    src/SR_SceneFileUtility.cpp
    src/SR_SceneGraph.cpp
    src/SR_SceneNode.cpp
    src/SR_Shader.cpp
    src/SR_ShaderProcessor.cpp
    src/SR_Texture.cpp
    src/SR_TexSampler.cpp
    src/SR_Transform.cpp
    src/SR_UniformBuffer.cpp
    src/SR_VertexArray.cpp
    src/SR_VertexBuffer.cpp
    src/SR_VertexProcessor.cpp
    src/SR_WindowBuffer.cpp)

if(WIN32)
    message("-- Win32 window backend enabled.")
    set(SR_LIB_SOURCES
        ${SR_LIB_SOURCES}
        src/SR_KeySymWin32.cpp
        src/SR_RenderWindowWin32.cpp
        src/SR_WindowBufferWin32.cpp)

elseif (PREFER_COCOA)
    message("-- Cocoa window backend enabled.")
    add_definitions(-DSR_PREFER_COCOA)

    set(SR_LIB_SOURCES
        ${SR_LIB_SOURCES}
        src/SR_KeySymCocoa.mm
        src/SR_RenderWindowCocoa.mm
        src/SR_WindowBufferCocoa.mm)

elseif(UNIX)
    if (PREFER_XCB)
        add_definitions(-DSR_PREFER_XCB)
        message("-- X11-XCB window backend enabled.")
    else()
        add_definitions(-USR_PREFER_XCB)
        message("-- X11 window backend enabled.")
    endif()

    # X11-shm extensions can not be used on OSX due to shared memory
    # allocation limits
    if(NOT APPLE)
        add_definitions(-DSR_ENABLE_XSHM=1)
        message("-- X11 Shared memory extensions enabled for X11 windows.")
    else()
        message("-- X11 Shared memory extensions disabled.")
    endif()

    set(SR_LIB_SOURCES
        ${SR_LIB_SOURCES}
        src/SR_KeySymXlib.cpp
        src/SR_RenderWindowXCB.cpp
        src/SR_RenderWindowXlib.cpp
        src/SR_WindowBufferXCB.cpp
        src/SR_WindowBufferXlib.cpp)

else()
    message(FATAL_ERROR "Unknown/unsupported platform")
endif()



# -------------------------------------
# Library Setup
# -------------------------------------
option(BUILD_SHARED_LIBS "Build Shared Libraries" OFF)

if (BUILD_SHARED_LIBS)
    add_definitions(-DSR_BUILD_SHARED -DSR_SHARED)
    set(SR_LIB_TYPE SHARED)
else()
    set(SR_LIB_TYPE STATIC)
endif()

add_library(${PROJECT_NAME} ${SR_LIB_TYPE} ${SR_LIB_SOURCES} ${SR_LIB_HEADERS})

target_link_libraries(
    ${PROJECT_NAME}
    LightSky::Game
    LightSky::Math
    LightSky::Utils
    LightSky::Setup
    ${ASSIMP_LIBRARIES}
    ${FREEIMAGE_LIBRARIES})

if (NOT MSVC AND NOT PREFER_COCOA)
    target_link_libraries(
        ${PROJECT_NAME}
        ${XCB_LIBRARY}
        ${XCB_SHM_LIBRARY}
        ${X11_XCB_LIBRARY}
        ${X11_LIBRARIES}
        pthread)

elseif (PREFER_COCOA)
    target_link_libraries(
        ${PROJECT_NAME}
        "${APPKIT_LIBRARY}"
        "${COCOA_LIBRARY}"
        "${CORE_FOUNDATION_LIBRARY}"
        "${CORE_GRAPHICS_LIBRARY}")

endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${FREEIMAGE_INCLUDE_PATH})


# -------------------------------------
# Library Installation Setup
# -------------------------------------
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(DIRECTORY include/soft_render DESTINATION include)



# -------------------------------------
# Library Tests
# -------------------------------------
option(SR_BUILD_TESTS "Build tests for the Soft Render library." ON)

if(SR_BUILD_TESTS)
    add_subdirectory(tests)
endif()
