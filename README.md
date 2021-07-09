# SoftLight
A Software Renderer Using The LightSky Framework.

[![Build](https://github.com/hamsham/SoftLight/actions/workflows/cmake.yml/badge.svg)](https://github.com/hamsham/SoftLight/actions/workflows/cmake.yml)



## About SoftLight
SoftLight started as a research project into software rendering and shader
technology which mimicked modern hardware rendering APIs. By looking at other
software rendering projects like
[tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki) and
[Mesa3D](https://www.mesa3d.org), SoftLight began to take shape.



## Tech
SoftLight has been built from the ground-up to run on Windows (Win32 & GDI),
Linux (X11 or XCB, optionally with XSHM-MIT), and OS X (Cocoa & Quartz) using
C++. For window creation, only the Win32 API and Xlib development libraries
are used. Direct framebuffer access is available which can allow for other
implementations to be created as well, or you can simply use the window
handles to embed an internal, native context into other applications which
support it.

SoftLight uses CPU-based SIMD acceleration (AVX2 on x86 and NEON/VFPv4 on ARM)
to increase performance. There are also multithreaded work queues which can
take advantage of as many cores as a system has available. The number of cores
can be configured at runtime to help increase flexibility.



## Getting Started
#### Compiler & Archtecture Support
Technically, any C++11-compliant compiler should be able to build the project.
To build the project, you'll need GCC, Clang, Microsoft Visual Studio 2017, or
MSVC 2019. Softlight should work on any modern architecture (tested on i686,
x64, ARM, AARCH64, and PowerPC-EL64), the project has been tested on the
following compilers:

| GCC   | Clang* | MSVC** | MinGW-W64** |
| ----- | ------ | ------ | ----------- |
| 4.8.2 | 3.5    | 2017   | 5.1.0       |
| 5.0.0 | 3.6    | 2019   | 7.4.0       |
| 7.3.0 | 6.0    |        | 8.4.0       |
| 9.3.0 | 10.0   |        |             |

\* Benchmarking has consistently shown Clang generates and builds the most
optimized code.

\**Windows builds typically run the slowest. MSVC is the worst performer and
using MinGW-w64 can lead to better performance. 


#### Dependencies
3rd-party development dependencies listed here will either be detected from
the local system or downloaded and built from source:

###### System/OS Dependencies
| Dependency        | Platform        | Required? | Use/Application           |
| ----------------- | --------------- | --------- | ------------------------- |
| cmake             | All             | Yes       | Generate build files      |
| x11-utils         | Linux           | Yes       | Required for X11 & XCB    |
| libx11-dev        | Linux           | Yes       | Required for X11 & XCB    |
| libx11-xcb-dev    | Linux           | Yes       | Enable X11 Shared Memory  |
| libxext-dev       | Linux           | No        | Required for X11 & XCB    |
| libxcb1-dev       | Linux           | Yes       | Required for X11 & XCB    |
| libxcb-image0-dev | Linux           | Yes       | Required for X11 & XCB    |
| libxcb-shm0-dev   | Linux           | No        | Enable XCB Shared Memory  |
| *libxkbcommon     | OS X + Homebrew | No        | Enable X11/XQuartz on OSX |
| TortoiseSVN       | Windows         | Yes       | Checkout 3rd-party libs   |

\* The X11 backend can be used on OS X with XQuartz but XSHM-MIT optimizations
will be unavailable due to limitations of shared memory.

Linux targets currently require both X11 and XCB development libraries to
compile. However the backend can be chosen at compile time by passing the
`-DPREFER_XCB=TRUE` build flag to CMake. Similarly on OSX, the X11 backend can
also be chosen by setting the `-DPREFER_COCOA=FALSE` flag. The X-Shared Memory
(XSHM-MIT) will be checked and enabled at compile-time for better performance.
All other dependencies will be downloaded from their source repositories and
compiled.

###### 3rd-Party Dependencies
| Dependency | Use/Application                                                |
| ---------- | -------------------------------------------------------------- |
| ASSIMP     | For loading various 3D asset file formats                      |
| FreeImage  | To load textures and image files.                              |
| ENet       | Dependency of LightUtils. Used for UDP Networking.             |
| GLM        | Dependency of LightMath. For unit testing and validation only. |
| FreeType   | Used for loading TTF fonts.                                    |

It is recommended to install the 3rd-party development packages onto the
system to speed up build times.



## Checking out the Code
SoftLight uses the following submodules:
- [LightSetup](https://github.com/hamsham/LightSetup)
- [LightUtils](https://github.com/hamsham/LightUtils)
- [LightMath](https://github.com/hamsham/LightMat)

Cloning the repository should be done recursively, using the following steps:
1. `git clone --recursive https://github.com/hamsham/SoftLight`
2. `git submodule foreach git checkout master`
3. `git submodule foreach git pull origin master`



## Examples
SoftLight is a very flexible software rasterizer. Compiling the code will
generate a set of tests you can use to play around with. Some interesting
examples include:
 * Parallel Mesh Instancing (`instancing_test.cpp`): Toggle threaded
 instancing by pressing the `F2` key.
 * Skinning and animation (`animation_test.cpp`).
 * Large indoor environment rendering (`live_scene_test.cpp`): Press the `F1`
 Key to capture the mouse and explore the environment using the WASD keys. You
 can also toggle PBR-based rendering with the `F2` key as well as change the
 number of threads used for rendering by using the up/down arrow keys.
 * Full-screen quad rendering (`fs_quad.cpp`). Render to an offscreen [Compact
 YCoCg Framebuffer](http://jcgt.org/published/0001/01/02/) then reconstruct
 the image in a second render pass.
 * Volumetric voxel rendering (`volume_test.cpp`).

Check out the below screenshots to see what else it can do!

![Diffuse Lighting, 122 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_diffuse.png)

![Physically Based Rendering, 114 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_pbr.png)

![Skeletal Animations](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_anims.png)

![Mesh Instancing, ~280 FPS, 14 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_instancing.png)

![Volume Rendering, ~9 FPS, 16 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_volumes.png)

![True-Type Font Rendering, ~562 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_text.png)



## Samples
To see how to build a sample application using either SoftLight or the
LightSky frameworks, explore the "tests" subdirectories within each project.
You will find plenty of examples on how to load a 3D model, create a shader,
render, and manipulate a 3D scene in real-time.




## TODO
- [ ] Implement stencil buffers.
- [ ] Provide support for mip-mapping.
- [ ] Anti-aliasing using FXAA.
- [ ] Doxygen-based documentation for the software rendering module.
- [ ] Additional Unit Tests.
- [ ] Create additional examples.
- [ ] Create Geometry Shader API



## Q&A
For any inquiries or pull requests, please email mileslacey@gmail.com. Bugs
and other issues can be added
[here](https://github.com/hamsham/SoftLight/issues) on GitHub. Finally, all
source code is available under the MIT License.

