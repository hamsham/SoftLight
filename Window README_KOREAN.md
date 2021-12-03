# Window 전용 가이드
Window 환경의 사용자들을 위한 가이드



## 실행 단계
1) PC 세팅을 영어로 하기
2) 리포지토리 복제
3) `visual studio 2022` 설치
4) `TortoiseSVN` 설치
5) `softlight` 폴더를 만들고 하위에 `build` 폴더를 만들기
6) `cmake` 구성
7) `"build/softlight/tests/Release"` 경로에 dll 파일을 붙혀넣기
8) `SoftLight.sln` 실행




## 1) PC 세팅을 영어로 하기
먼저 경로 설정을 영어로 해주어야 합니다.



## 2) 리포지토리 복제
리포지토리 복제는 다음 단계를 사용하여 재귀적으로 수행해야 합니다.
1. `git clone --recursive https://github.com/hamsham/SoftLight`
2. `cd SoftLight`
3. `git submodule foreach git checkout master`
4. `git submodule foreach git pull origin master`


## 3) `Visual Studio 2022`를 설치
다음 그림과 같이 체크하고 설치해주세요.

![image](https://user-images.githubusercontent.com/91865644/144486940-f0da49bd-08bd-479c-8295-b17a0d5bc41c.png)
![image](https://user-images.githubusercontent.com/91865644/144487118-747c143a-5f22-49d5-b8e6-ca8a2f4b7bd1.png)
![image](https://user-images.githubusercontent.com/91865644/144487136-898fcac4-7c5d-429d-a829-ef9c9ccb5edd.png)
![image](https://user-images.githubusercontent.com/91865644/144487140-2833750c-f4d0-4fc5-a7c9-a555f6f1cfa9.png)



## 4) `TortoiseSVN` 설치
다음 사이트에서 https://tortoisesvn.net/downloads.html 자신의 환경에 맞게 `TortoiseSVN` 다운로드 버튼을 눌러주세요. 
다음 그림과 같이 반드시 `command line client tools`에서 `Entire feature will be installed on local hard drive`를 선택해주세요.
![image](https://user-images.githubusercontent.com/91865644/144487317-b4661661-f50b-4c00-8426-579526968af5.png)

![image](https://user-images.githubusercontent.com/91865644/144487328-a7907c1c-07c4-47e0-b7fb-13a48c16211c.png)

`TortoiseSVN` 설치가 완료됐으면 다음과 같은 실행 파일이 생긴 것을 확인할 수 있습니다.

![image](https://user-images.githubusercontent.com/91865644/144487391-24086dc2-3a09-4bd3-b085-0da912ea5349.png)

그리고 다음 그림과 같이 환경변수를 추가해주세요.

![image](https://user-images.githubusercontent.com/91865644/144487427-2587453e-75ab-41fc-acc0-23819531829b.png)



## 5) `softlight` 폴더를 만들고 하위에 `build` 폴더를 만들기
다음 그림과 같이 바탕화면에 `softlight`라는 폴더를 만들고, 해당 폴더 안에 `build` 폴더를 만들어주세요.

![image](https://user-images.githubusercontent.com/91865644/144487663-874f608b-4c3f-486d-a840-63ec85684a7b.png)




## 6) `cmake` 구성
다음 사이트에서 https://cmake.org/ `cmake`를 다운해주세요.
설치를 완료하면 아래 그림과 같은 폴더가 생깁니다. 

![image](https://user-images.githubusercontent.com/91865644/144488392-7e77c9b1-ffbf-4aed-b45a-87c9c162cee6.png)

해당 폴더에서 `bin`을 들어가고, `cmake-gui.exe`를 실행시켜줍니다.

![image](https://user-images.githubusercontent.com/91865644/144488464-ca72e6c4-0fe4-4ed6-b37a-4a25391f4a0d.png)

위에는 `git`에서 `clone`한 `SoftLight` 경로를 적고, 아래는 5)단계에서 생성한 `build` 경로를 적어주세요.

![image](https://user-images.githubusercontent.com/91865644/144488621-8c6b31df-c62b-4070-a58f-913a55604377.png)

`generate` 버튼을 누르고 아래 그림과 같이 체크해주세요.

![image](https://user-images.githubusercontent.com/91865644/144488767-88a74775-364b-4c34-b5b1-2e238d3e9630.png)
![image](https://user-images.githubusercontent.com/91865644/144488863-0e6b0953-6f85-4d1a-a991-3ccf3165336f.png)

`Advanced` 버튼을 누르고 `svn.exe`가 있는 경로를 지정해줍니다.

![image](https://user-images.githubusercontent.com/91865644/144488974-7ed1b3bb-751e-4a15-895c-6661ba403c34.png)

`Configure` 버튼을 눌러줍니다.

![image](https://user-images.githubusercontent.com/91865644/144489060-6f241123-f41e-4ff8-9f82-575885389503.png)

`build`파일 안에 아래 그림과 같은 파일들이 생성되었음을 확인할 수 있습니다.

![image](https://user-images.githubusercontent.com/91865644/144489220-33be66f5-588f-4679-acbe-ebd2d0231a9d.png)





## 7) `"build/softlight/tests/Release"` 경로에 dll 파일을 붙혀넣기
해당 경로의 파일을 복사해서 `"build/softlight/tests/Release"` 경로에 넣어줍니다.
![image](https://user-images.githubusercontent.com/91865644/144489374-01d739eb-f0cd-4d0e-89b8-c74c177f97bd.png)




## 8) `SoftLight.sln` 실행
`build`파일 안에 있는 `SoftLight.sln` 파일이 있는 것을 확인할 수 있습니다.

![image](https://user-images.githubusercontent.com/91865644/144489570-eab54934-ad38-43d9-bb29-be1398603a25.png)

`visual studio` 2022로 실행시켜줍니다.

![image](https://user-images.githubusercontent.com/91865644/144489581-dd75e6d5-c047-48e3-9a08-1d943483eda3.png)

`Release`로 바꿔줍니다.

![image](https://user-images.githubusercontent.com/91865644/144489645-70a53cd8-ec06-4291-bb19-8a5986a8b34c.png)

실행시키고 싶은 `test` 파일을 아래 그림처럼 `Set as Startup Project`를 눌러줍니다.


![image](https://user-images.githubusercontent.com/91865644/144489720-7ed14f49-b240-4bb9-9ed9-1218d67ac918.png)

### 실행 가능한 파일

- `sl_animation_test`
- `sl_fullscreen_quad`
- `sl_instancing_test`
- `sl_large_scene_test`
- `sl_mesh_test`
- `sl_mrt_test`
- `sl_octree_rendering_test`
- `sl_quadtree_rendering_test`
- `sl_sdf_image_test`
- `sl_shading_test`
- `sl_skybox_test`
- `sl_text_test`
- `sl_volume_rendering_test`

아래 그림처럼 `Local Windows Debugger` 버튼을 눌러 실행해줍니다.

![image](https://user-images.githubusercontent.com/91865644/144490008-8a523c32-dbe5-48ee-8909-7f6a027fe642.png)


`F1`키를 누르고 `W, A, S, D` 키를 이용해 상하좌우로 움직일 수 있습니다.
`F2`키를 누르면 더 높은 수준의 `rendering`을 확인할 수 있습니다.
### `rendering` 전(F2키 누르기 전)

![image](https://user-images.githubusercontent.com/91865644/144490176-25bd2915-87af-44e1-b24f-fd2bc05fde1c.png)

### `rendering` 후(F2키 누른 후)

![image](https://user-images.githubusercontent.com/91865644/144490192-5f40a545-dc7c-4f76-ae5d-b2f94345dbcb.png)



