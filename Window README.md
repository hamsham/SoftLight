# Guide for window users
A guide for users in a window environment



## stage
1) Setting up the PC in English
2) Cloning the Repository
3) `visual studio 2022` installation
4) `TortoiseSVN` installation
5) Create a `softlight` folder and make a sub-folder named `build`.
6) `cmake` configuration
7) Paste dll file in path `"build/softlight/tests/release"`
8) Run `SoftLight.sln`



## 1) Setting up the PC in English
First, you have to set the route in English.



## 2) Cloning the Repository
Cloning the repository should be done recursively, using the following steps:
1. `git clone --recursive https://github.com/hamsham/SoftLight`
2. `cd SoftLight`
3. `git submodule foreach git checkout master`
4. `git submodule foreach git pull origin master`


## 3) `Visual Studio 2022` installation
check and install it as shown in the following picture.

![image](https://user-images.githubusercontent.com/91865644/144486940-f0da49bd-08bd-479c-8295-b17a0d5bc41c.png)
![image](https://user-images.githubusercontent.com/91865644/144487118-747c143a-5f22-49d5-b8e6-ca8a2f4b7bd1.png)
![image](https://user-images.githubusercontent.com/91865644/144487136-898fcac4-7c5d-429d-a829-ef9c9ccb5edd.png)
![image](https://user-images.githubusercontent.com/91865644/144487140-2833750c-f4d0-4fc5-a7c9-a555f6f1cfa9.png)



## 4) `TortoiseSVN` installation
At the following site https://tortoisesvn.net/downloads.html, please press the Download TortoiseSVN button to suit your environment.
Make sure to select `Entire feature will be installed on local hard drive` from `command line client tools` as shown in the following image.
![image](https://user-images.githubusercontent.com/91865644/144487317-b4661661-f50b-4c00-8426-579526968af5.png)

![image](https://user-images.githubusercontent.com/91865644/144487328-a7907c1c-07c4-47e0-b7fb-13a48c16211c.png)

Once the `TortoiseSVN` installation is completed, you can check that the following executable file has been created.

![image](https://user-images.githubusercontent.com/91865644/144487391-24086dc2-3a09-4bd3-b085-0da912ea5349.png)

And add environmental variable as shown in the following picture.

![image](https://user-images.githubusercontent.com/91865644/144487427-2587453e-75ab-41fc-acc0-23819531829b.png)



## 5) Create a `softlight` folder and build a folder below it
Create a folder called `softlight` on the desktop as shown in the following picture, and create a `build` folder in that folder.

![image](https://user-images.githubusercontent.com/91865644/144487663-874f608b-4c3f-486d-a840-63ec85684a7b.png)




## 6) `cmake` configuration
Download `cmake` from the following site https://cmake.org/.
When you complete the installation, you will find a folder as shown in the picture below.

![image](https://user-images.githubusercontent.com/91865644/144488392-7e77c9b1-ffbf-4aed-b45a-87c9c162cee6.png)

Enter `bin` from that folder and run `cmake-gui.exe`.

![image](https://user-images.githubusercontent.com/91865644/144488464-ca72e6c4-0fe4-4ed6-b37a-4a25391f4a0d.png)

Write down the clean `SoftLight` path in git above, and the `build` path created in step 5) below.

![image](https://user-images.githubusercontent.com/91865644/144488621-8c6b31df-c62b-4070-a58f-913a55604377.png)

Press the `generate` button and check it as shown in the picture below.

![image](https://user-images.githubusercontent.com/91865644/144488767-88a74775-364b-4c34-b5b1-2e238d3e9630.png)
![image](https://user-images.githubusercontent.com/91865644/144488863-0e6b0953-6f85-4d1a-a991-3ccf3165336f.png)

Press the `Advanced` button and specify the path with `svn.exe`.

![image](https://user-images.githubusercontent.com/91865644/144488974-7ed1b3bb-751e-4a15-895c-6661ba403c34.png)

Press the `Configure` button.

![image](https://user-images.githubusercontent.com/91865644/144489060-6f241123-f41e-4ff8-9f82-575885389503.png)

You can see that the files shown in the picture below have been created in the `build` file.

![image](https://user-images.githubusercontent.com/91865644/144489220-33be66f5-588f-4679-acbe-ebd2d0231a9d.png)





## 7) Paste dll file in path `"build/softlight/tests/release"`
Copy the file of that path and put it in the `"build/softlight/tests/release"` path.
![image](https://user-images.githubusercontent.com/91865644/144489374-01d739eb-f0cd-4d0e-89b8-c74c177f97bd.png)




## 8) Run `SoftLight.sln`
You can see that there is a `SoftLight.sln` file in the build file.

![image](https://user-images.githubusercontent.com/91865644/144489570-eab54934-ad38-43d9-bb29-be1398603a25.png)

Run it with `visual studio 2022`.

![image](https://user-images.githubusercontent.com/91865644/144489581-dd75e6d5-c047-48e3-9a08-1d943483eda3.png)

Change it to `Release`.

![image](https://user-images.githubusercontent.com/91865644/144489645-70a53cd8-ec06-4291-bb19-8a5986a8b34c.png)

Press `Set as Startup Project` as shown in the picture below with the `test` file you want to run.


![image](https://user-images.githubusercontent.com/91865644/144489720-7ed14f49-b240-4bb9-9ed9-1218d67ac918.png)

### executable file

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

Press the `Local Windows Debugger` button to run it.

![image](https://user-images.githubusercontent.com/91865644/144490008-8a523c32-dbe5-48ee-8909-7f6a027fe642.png)


Press `F1` and use `W`, `A`, `S`, and `D` keys to move up, down, left, and right.
Press the `F2` key to check a higher level of `rendering`.
### Before `rendering`(Before pressing F2 key)

![image](https://user-images.githubusercontent.com/91865644/144490176-25bd2915-87af-44e1-b24f-fd2bc05fde1c.png)

### After `rendering`(After pressing F2 key)

![image](https://user-images.githubusercontent.com/91865644/144490192-5f40a545-dc7c-4f76-ae5d-b2f94345dbcb.png)



