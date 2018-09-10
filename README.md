# SoftLight
A Software Renderer Using The LightSky Framework.



## About SoftLight
SoftLight started as a research project into software rendering and shader technology which mimicked modern hardware rendering APIs. By looking at other software rendering projects like [tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki) and [Mesa3D](https://www.mesa3d.org), SoftLight began to take shape.



## Tech
SoftLight has been built from the ground-up to run on Windows, Linux (X11-based), and OS X using C++. For window creation, only the Win32 API and Xlib development libraries are used. Direct framebuffer access is available which can allow for other implementations to be created as well, or you can simply use the window handles to embed an Xlib/Win32 context into other applications which support it.

SoftLight uses CPU-based SIMD acceleration (SSE3 on x86 and NEON/VFPv4 on ARM) to increase performance. There are also multithreaded work queues which can take advantage of as many cores as a system has available. The number of cores can be configured at runtime to help increase flexibility.



## Getting Started
To build the project, you'll need GCC, Clang, or Microsoft Visual Studio
2017. The project has been tested on the following compilers:

| GCC | Clang | MSVC | MinGW |
| --- | ----- | ---- | ----- |
| 4.8.2 | 3.5 | 2017 | 5.1.0 |
| 5.0.0 | 3.6 |      |       |
| 7.3.0 | 6.0 |      |       |

Technically, any C++11-compliant compiler should be able to build the project.

Additionally, Linux and OS X targets need the Xlib development libraries with the X Shared Memory (XShm) extension. All other dependencies will be downloaded from their source repositories and compiled. Currently, the project will download Assimp (to load 3D mesh files), GLM (for testing the math library), and FreeImage (to load various image file formats).



## Samples
To see how to build a sample application using either SoftLight or the LightSky frameworks, explore the "tests" subdirectories within each project. You will find plenty of examples on how to load a 3D model, create a shader, render, and manipulate a 3D scene in real-time.



## TODO
- [ ] Doxygen-based documentation for the software rendering module.
- [X] Doxygen-based documentation for the LightSky modules.
- [X] Add Unit Tests.
- [ ] Finish Examples.



## Q&A
For any inquiries or pull requests, please email mileslacey@gmail.com. Bugs and other issues can be added [here](https://github.com/hamsham/SoftLight/issues) on GitHub. Finally, all source code is available under the MIT License.

