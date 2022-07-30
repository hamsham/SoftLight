# 소프트라이트
LightSky 프레임워크를 사용하는 소프트웨어 렌더.

[![Build](https://github.com/hamsham/SoftLight/actions/workflows/cmake.yml/badge.svg)](https://github.com/hamsham/SoftLight/actions/workflows/cmake.yml)



## 소프트라이트 소개
SoftLight는 최신 하드웨어 렌더링 API를 모방한 소프트웨어 렌더링 및 셰이더 기술에 대한 연구 프로젝트로 시작되었습니다.
[tinyrenderer](https://github.com/ssloy/tinyrenderer/wiki) 및
[Mesa3D](https://www.mesa3d.org) 와 같은 다른 소프트웨어 렌더링 프로젝트를 살펴봄 으로써 SoftLight 가 구체화되기 시작했습니다.



## 기술
SoftLight는 처음부터 C++를 사용하여 Windows(Win32 및 GDI), Linux(X11 또는 XCB, 선택적으로 XSHM-MIT 포함) 및 OS X(Cocoa 및 Quartz)에서 실행되도록 구축되었습니다. 창 생성에는 Win32 API 및 Xlib 개발 라이브러리만 사용됩니다. 직접 프레임 버퍼 접근이 가능하므로 다른 구현도 생성할 수 있습니다. 또는 내부, 기본 컨텍스트를 지원하는 다른 응용 프로그램에 포함시키기 위해서 창 핸들을 사용할 수 있습니다.

SoftLight는 CPU 기반 SIMD 가속(x86의 AVX2 및 ARM의 NEON/VFPv4)을 사용하여 성능을 향상시킵니다. 또한 시스템이 사용할 수 있는 만큼의 코어를 이용할 수 있는 다중 스레드 작업 대기열도 있습니다. 런타임에 코어 수를 구성하여 유연성을 높일 수 있습니다.



## 시작하기
#### 컴파일러 및 아키텍처 지원
기술적으로 모든 C++11 호환 컴파일러는 프로젝트를 빌드할 수 있어야 합니다. 프로젝트를 빌드하려면 GCC, Clang, Microsoft Visual Studio 2017 또는 MSVC 2019가 필요합니다. Softlight는 모든 최신 아키텍처에서 작동해야 합니다(i686, x64, ARM, AARCH64 및 PowerPC-EL64에서 테스트). 다음 컴파일러에서 테스트:

| GCC    | Clang* | MSVC** | MinGW-W64** |
| ------ | ------ | ------ | ----------- |
| 4.8.2  | 3.5    | 2017   | 5.1.0       |
| 5.0.0  | 3.6    | 2019   | 7.4.0       |
| 7.3.0  | 6.0    | 2022   | 8.4.0       |
| 9.3.0  | 10.0   |        | 11.0.0      |
| 11.1.0 | 12.0   |        |             |

\* 벤치마킹은 Clang이 가장 최적화된 코드를 생성하고 빌드한다는 것을 일관되게 보여주었습니다.

\** Windows 빌드는 일반적으로 가장 느리게 실행됩니다. MSVC는 최악의 성능을 보이며 MinGW-w64를 사용하면 성능이 향상될 수 있습니다. 


#### 종속성
여기에 나열된 타사 개발 종속성은 로컬 시스템에서 감지되거나 소스에서 다운로드 및 빌드됩니다.

###### 시스템/OS 종속성
| 의존성       | 플랫폼       | 필수? | 사용/적용           |
| ----------------- | --------------- | --------- | ------------------------- |
| cmake             | 모두             | 예       | 빌드 파일 생성      |
| x11-utils         | Linux           | 예       | X11 및 XCB에 필요    |
| libx11-dev        | Linux           | 예       | X11 및 XCB에 필요    |
| libx11-xcb-dev    | Linux           | 예       | X11 공유 메모리 활성화  |
| libxext-dev       | Linux           | 아니요        | X11 및 XCB에 필요    |
| libxcb1-dev       | Linux           | 예       | X11 및 XCB에 필요    |
| libxcb-image0-dev | Linux           | 예       | X11 및 XCB에 필요    |
| libxcb-shm0-dev   | Linux           | 아니요        | XCB 공유 메모리 활성화  |
| *libxkbcommon     | OS X + Homebrew | 아니요        | 	OSX에서 X11/XQuartz 활성화 |
| TortoiseSVN       | Windows         | 예       | 체크아웃 타사 라이브러리   |

\* X11 백엔드는 XQuartz가 있는 OS X에서 사용할 수 있지만 공유 메모리의 제한으로 인해 XSHM-MIT 최적화를 사용할 수 없습니다.

Linux 대상은 현재 컴파일을 위해 X11 및 XCB 개발 라이브러리가 모두 필요합니다. 그러나
`-DPREFER_XCB=TRUE` 빌드 플래그를 CMake 에 전달하여 컴파일 타임에 백엔드를 선택할 수 있습니다 . 마찬가지로 OSX에서  `-DPREFER_COCOA=FALSE` 플래그 를 설정하여 X11 백엔드를 선택할 수도 있습니다 . X-공유 메모리(XSHM-MIT)는 더 나은 성능을 위해 컴파일 타임에 확인되고 활성화됩니다. 다른 모든 종속성은 소스 리포지토리에서 다운로드되어 컴파일됩니다.

###### 타사 종속성
| 의존성 | 사용/적용                                              |
| ---------- | -------------------------------------------------------------- |
| ASSIMP     | 다양한 3D 자산 파일 형식 로드용                      |
| FreeImage  | 텍스처 및 이미지 파일을 로드합니다.                              |
| ENet       | 텍스처 및 이미지 파일을 로드합니다.            |
| GLM        | LightMath의 종속성. 단위 테스트 및 유효성 검사 전용입니다. |
| FreeType   | TTF 글꼴을 로드하는 데 사용됩니다.                                    |

빌드 시간을 단축하려면 시스템에 타사 개발 패키지를 설치하는 것이 좋습니다.



## 코드 확인하기
SoftLight는 다음 하위 모듈을 사용합니다.
- [LightSetup](https://github.com/hamsham/LightSetup)
- [LightUtils](https://github.com/hamsham/LightUtils)
- [LightMath](https://github.com/hamsham/LightMath)
- [LightGame](https://github.com/hamsham/LightGame)
- [LightScript](https://github.com/hamsham/LightScript)

리포지토리 복제는 다음 단계를 사용하여 재귀적으로 수행해야 합니다.
1. `git clone --recursive https://github.com/hamsham/SoftLight`
2. `cd SoftLight`
3. `git submodule foreach git checkout master`
4. `git submodule foreach git pull origin master`



## 예시
SoftLight는 매우 유연한 소프트웨어 래스터라이저입니다. 코드를 컴파일하면 가지고 놀 수 있는 테스트 세트가 생성됩니다. 몇 가지 흥미로운 예는 다음과 같습니다.
 * 병렬 메시 인스턴싱 (`sl_instancing_test.cpp`): `F2` 키 를 눌러 스레드 인스턴싱을 토글 합니다.
 * 스키닝 및 애니메이션 (`sl_animation_test.cpp`).
 * 대형 실내 환경 렌더링 (`sl_large_scene_test.cpp`): `F1`키를 눌러 마우스를 캡처하고 WASD 키를 사용하여 환경을 탐색합니다. 또한
  `F2` 키로 PBR 기반 렌더링을 토글할 수 있을 뿐만 아니라 위쪽/아래쪽 화살표 키를 사용하여 렌더링에 사용되는 스레드 수를 변경할 수 있습니다.
 * 전체 화면 쿼드 렌더링 (`sl_fullscreen_quad.cpp`). 오프스크린 [Compact
 YCoCg Framebuffer](http://jcgt.org/published/0001/01/02/) 로 렌더링한 다음 두 번째 렌더링 패스에서 이미지를 재구성합니다.
 * 체적 복셀 렌더링 (`sl_volume_rendering_test.cpp`).

아래 스크린샷을 확인하여 다른 작업을 수행할 수 있습니다!

![Diffuse Lighting, 122 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_diffuse.png)

![Physically Based Rendering, 114 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_pbr.png)

![Skeletal Animations](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_anims.png)

![Mesh Instancing, ~280 FPS, 14 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_instancing.png)

![Volume Rendering, ~9 FPS, 16 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_volumes.png)

![True-Type Font Rendering, ~562 FPS, 15 Threads, Ryzen 1800X](https://github.com/hamsham/SoftLight/blob/master/examples/softlight_text.png)



## 샘플
SoftLight 또는 LightSky 프레임워크를 사용하여 샘플 애플리케이션을 빌드하는 방법을 보려면 각 프로젝트 내의 "tests" 하위 디렉토리를 탐색하십시오. 3D 모델을 로드하고, 셰이더를 생성하고, 3D 장면을 실시간으로 렌더링 및 조작하는 방법에 대한 많은 예를 찾을 수 있습니다.



## 할 것
- [ ] 밉 매핑 지원을 제공합니다.
- [ ] 최신 "파이프라인" 구문을 사용하도록 셰이더 시스템을 이송합니다(Metal & Vulkan과 유사).
- [ ] 프레임 버퍼 시스템을 이송하여 패스를 렌더링합니다(Metal & Vulkan과 유사).
- [ ] 렌더 상태 개체에 대한 지원을 완료합니다(깊이-스텐실 상태 포함).
- [ ] 소프트웨어 렌더링 모듈에 대한 완전한 Doxygen 기반 문서.
- [ ] 추가 단위 테스트.
- [ ] 추가 예제를 만듭니다.
- [ ] 프로젝트 이식성을 위해 렌더링 모듈을 C API로 래핑합니다.
- [ ] 높은 수준의 장면 그래프 구성을 LightGame으로 이송합니다.
- [ ] SDL2 백엔드 지원을 추가합니다.



## Q&A
문의 사항이나 풀 리퀘스트가 있는 경우, mileslacey@gmail.com으로 이메일을 보내 주십시오 .
[여기](https://github.com/hamsham/SoftLight/issues) GitHub 에서 버그 및 기타 문제를 추가할 수 있습니다 . 마지막으로 모든 소스 코드는 MIT 라이선스에 따라 사용할 수 있습니다.

