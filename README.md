# SoftLight
A Software Renderer Using The LightSky Framework.

[![Build Status](https://travis-ci.org/hamsham/SoftLight.svg?branch=master)](https://travis-ci.org/hamsham/SoftLight)



## About SoftLight
SoftLight started as a research project into software rendering and shader technology which mimicked modern hardware rendering APIs. By looking at other software rendering projects like [tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki) and [Mesa3D](https://www.mesa3d.org), SoftLight began to take shape.



## Tech
SoftLight has been built from the ground-up to run on Windows, Linux (X11-based), and OS X using C++. For window creation, only the Win32 API and Xlib development libraries are used. Direct framebuffer access is available which can allow for other implementations to be created as well, or you can simply use the window handles to embed an Xlib/Win32 context into other applications which support it.

SoftLight uses CPU-based SIMD acceleration (AVX2 on x86 and NEON/VFPv4 on ARM) to increase performance. There are also multithreaded work queues which can take advantage of as many cores as a system has available. The number of cores can be configured at runtime to help increase flexibility.



## Getting Started
#### Compiler Support
To build the project, you'll need GCC, Clang, or Microsoft Visual Studio 2017. The project has been tested on the following compilers:

| GCC   | Clang | MSVC | MinGW-W64 |
| ----- | ----- | ---- | --------- |
| 4.8.2 | 3.5   | 2017 | 5.1.0     |
| 5.0.0 | 3.6   |      | 7.4.0     |
| 7.3.0 | 6.0   |      |           |



#### Dependencies
Technically, any C++11-compliant compiler should be able to build the project.

3rd-party development dependencies will either be detected and used or downloaded and built from source.
Linux and OS X targets need the Xlib development libraries with the X Shared Memory (XShm) extension. All other dependencies will be downloaded from their source repositories and compiled. Currently, the project will download Assimp (to load 3D mesh files), GLM (for testing the math library), and FreeImage (to load various image file formats).

For MinGW, it is recommended to install the FreeImage development package to ensure a successful build.



#### Checking out the Code
SoftLight uses the following submodules:
- [LightSetup](https://github.com/hamsham/LightSetup)
- [LightUtils](https://github.com/hamsham/LightUtils)
- [LightMath](https://github.com/hamsham/LightMat)

Cloning the repository should be done recursively, using the following steps:
1. `git clone --recursive https://github.com/hamsham/SoftLight`
2. `git submodule foreach git checkout master`
3. `git submodule foreach git pull origin master`



## Examples
SoftLight is a very flexible software rasterizer. Check out the below screenshots to see what it can do!

![Diffuse Lighting, 52 FPS, 4 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_diffuse.png)

![Physically Based Rendering, 36 FPS, 4 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_pbr.png)

![Scene Graph Animations](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_anims.png)

![Volume Rendering, ~9 FPS, 16 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_volumes.png)



## Samples
To see how to build a sample application using either SoftLight or the LightSky frameworks, explore the "tests" subdirectories within each project. You will find plenty of examples on how to load a 3D model, create a shader, render, and manipulate a 3D scene in real-time.




## TODO
- [ ] Implement stencil buffers.
- [ ] Provide support for mip-mapping.
- [ ] Anti-aliased textures.
- [ ] Doxygen-based documentation for the software rendering module.
- [ ] Additional Unit Tests.
- [ ] Create additional examples.
- [ ] Complete feature parity of GLSL functions.
- [X] Implement vertex skinning on top of the current animation framework.
- [ ] Create Geometry Shader API



## Q&A
For any inquiries or pull requests, please email mileslacey@gmail.com. Bugs and other issues can be added [here](https://github.com/hamsham/SoftLight/issues) on GitHub. Finally, all source code is available under the MIT License.

