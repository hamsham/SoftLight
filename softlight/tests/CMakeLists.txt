
# -------------------------------------
# Project Setup
# -------------------------------------
project(soft_render_tests CXX)



# -------------------------------------
# Include Paths
# -------------------------------------
include_directories(".")



# -------------------------------------
# Resource Files
# -------------------------------------
configure_file(testdata/heart/heart.obj           testdata/heart/heart.obj           COPYONLY)
configure_file(testdata/heart/heart.mtl           testdata/heart/heart.mtl           COPYONLY)
configure_file(testdata/heart/Heart_env.png       testdata/heart/Heart_env.png       COPYONLY)
configure_file(testdata/heart/HeartUtuwaFrame.png testdata/heart/HeartUtuwaFrame.png COPYONLY)
configure_file(testdata/towerG.obj                testdata/towerG.obj                COPYONLY)
configure_file(testdata/earth.png                 testdata/earth.png                 COPYONLY)

configure_file(testdata/bob/Bob.md5anim       testdata/bob/Bob.md5anim       COPYONLY)
configure_file(testdata/bob/Bob.md5mesh       testdata/bob/Bob.md5mesh       COPYONLY)
configure_file(testdata/bob/bob.source.txt    testdata/bob/bob.source.txt    COPYONLY)
configure_file(testdata/bob/guard1_body.png   testdata/bob/guard1_body.png   COPYONLY)
configure_file(testdata/bob/guard1_face.png   testdata/bob/guard1_face.png   COPYONLY)
configure_file(testdata/bob/guard1_helmet.png testdata/bob/guard1_helmet.png COPYONLY)
configure_file(testdata/bob/iron_grill.png    testdata/bob/iron_grill.png    COPYONLY)
configure_file(testdata/bob/round_grill.png   testdata/bob/round_grill.png   COPYONLY)

configure_file(testdata/african_head/african_head.obj testdata/african_head/african_head.obj COPYONLY)
configure_file(testdata/african_head/african_head.mtl testdata/african_head/african_head.mtl COPYONLY)
configure_file(testdata/african_head/african_head.ppm testdata/african_head/african_head.ppm COPYONLY)

#configure_file(testdata/base_model/base_model.dae                   testdata/base_model/base_model.dae                   COPYONLY)
#configure_file("testdata/base_model/Material.001 Diffuse Color.png" "testdata/base_model/Material.001 Diffuse Color.png" COPYONLY)
#configure_file("testdata/base_model/Material.002 Diffuse Color.png" "testdata/base_model/Material.002 Diffuse Color.png" COPYONLY)

configure_file(testdata/rover/MSL_dirty.blend      testdata/rover/MSL_dirty.blend      COPYONLY)
configure_file(testdata/rover/MSL_Shadow2.png.meta testdata/rover/MSL_Shadow2.png.meta COPYONLY)
configure_file(testdata/rover/MSL_Shadow.png.meta  testdata/rover/MSL_Shadow.png.meta  COPYONLY)
configure_file(testdata/rover/parts_AO.png         testdata/rover/parts_AO.png         COPYONLY)
configure_file(testdata/rover/parts_AO.png.meta    testdata/rover/parts_AO.png.meta    COPYONLY)
configure_file(testdata/rover/testmesh.dae         testdata/rover/testmesh.dae         COPYONLY)
configure_file(testdata/rover/tex_01.png           testdata/rover/tex_01.png           COPYONLY)
configure_file(testdata/rover/tex_01.png.meta      testdata/rover/tex_01.png.meta      COPYONLY)
configure_file(testdata/rover/tex_02.png           testdata/rover/tex_02.png           COPYONLY)
configure_file(testdata/rover/tex_02.png.meta      testdata/rover/tex_02.png.meta      COPYONLY)
configure_file(testdata/rover/tex_03_n.png         testdata/rover/tex_03_n.png         COPYONLY)
configure_file(testdata/rover/tex_03_n.png.meta    testdata/rover/tex_03_n.png.meta    COPYONLY)
configure_file(testdata/rover/tex_03.png           testdata/rover/tex_03.png           COPYONLY)
configure_file(testdata/rover/tex_03.png.meta      testdata/rover/tex_03.png.meta      COPYONLY)
configure_file(testdata/rover/tex_03_s.png         testdata/rover/tex_03_s.png         COPYONLY)
configure_file(testdata/rover/tex_03_s.png.meta    testdata/rover/tex_03_s.png.meta    COPYONLY)
configure_file(testdata/rover/tex_04.png           testdata/rover/tex_04.png           COPYONLY)
configure_file(testdata/rover/tex_04.png.meta      testdata/rover/tex_04.png.meta      COPYONLY)
configure_file(testdata/rover/tex_05.png           testdata/rover/tex_05.png           COPYONLY)
configure_file(testdata/rover/tex_05.png.meta      testdata/rover/tex_05.png.meta      COPYONLY)

configure_file(testdata/sibenik/copyright.txt      testdata/sibenik/copyright.txt      COPYONLY)
configure_file(testdata/sibenik/kamen-bump.png     testdata/sibenik/kamen-bump.png     COPYONLY)
configure_file(testdata/sibenik/kamen.png          testdata/sibenik/kamen.png          COPYONLY)
configure_file(testdata/sibenik/KAMEN-stup.png     testdata/sibenik/KAMEN-stup.png     COPYONLY)
configure_file(testdata/sibenik/mramor6x6-bump.png testdata/sibenik/mramor6x6-bump.png COPYONLY)
configure_file(testdata/sibenik/mramor6x6.png      testdata/sibenik/mramor6x6.png      COPYONLY)
configure_file(testdata/sibenik/sibenik.mtl        testdata/sibenik/sibenik.mtl        COPYONLY)
configure_file(testdata/sibenik/sibenik.obj        testdata/sibenik/sibenik.obj        COPYONLY)

#configure_file(testdata/sponza/sponza.mtl testdata/sponza/sponza.mtl COPYONLY)
#configure_file(testdata/sponza/sponza.obj testdata/sponza/sponza.obj COPYONLY)
#configure_file(testdata/sponza/copyright.txt testdata/sponza/copyright.txt COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_c_diff.png testdata/sponza/textures/sponza_column_c_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_thorn_diff.png testdata/sponza/textures/sponza_thorn_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_a_diff.png testdata/sponza/textures/sponza_column_a_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/lion2_bump.png testdata/sponza/textures/lion2_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_arch_diff.png testdata/sponza/textures/sponza_arch_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_b_spec.png testdata/sponza/textures/sponza_column_b_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_ceiling_a_spec.png testdata/sponza/textures/sponza_ceiling_a_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_a_spec.png testdata/sponza/textures/sponza_column_a_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_thorn_bump.png testdata/sponza/textures/sponza_thorn_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/lion.png testdata/sponza/textures/lion.png COPYONLY)
#configure_file(testdata/sponza/textures/floor_gloss.png testdata/sponza/textures/floor_gloss.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_bump.png testdata/sponza/textures/vase_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_hanging.png testdata/sponza/textures/vase_hanging.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_round.png testdata/sponza/textures/vase_round.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_fabric_blue_diff.png testdata/sponza/textures/sponza_fabric_blue_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_plant.png testdata/sponza/textures/vase_plant.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_roof_diff.png testdata/sponza/textures/sponza_roof_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/spnza_bricks_a_bump.png testdata/sponza/textures/spnza_bricks_a_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_curtain_diff.png testdata/sponza/textures/sponza_curtain_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_ceiling_a_diff.png testdata/sponza/textures/sponza_ceiling_a_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/background.png testdata/sponza/textures/background.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_curtain_green_diff.png testdata/sponza/textures/sponza_curtain_green_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_details_diff.png testdata/sponza/textures/sponza_details_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_details_spec.png testdata/sponza/textures/sponza_details_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_floor_a_diff.png testdata/sponza/textures/sponza_floor_a_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_b_bump.png testdata/sponza/textures/sponza_column_b_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/chain_texture_mask.png testdata/sponza/textures/chain_texture_mask.png COPYONLY)
#configure_file(testdata/sponza/textures/spnza_bricks_a_diff.png testdata/sponza/textures/spnza_bricks_a_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/spnza_bricks_a_spec.png testdata/sponza/textures/spnza_bricks_a_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_fabric_purple.png testdata/sponza/textures/sponza_fabric_purple.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_round_spec.png testdata/sponza/textures/vase_round_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_flagpole_spec.png testdata/sponza/textures/sponza_flagpole_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_arch_bump.png testdata/sponza/textures/sponza_arch_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/lion_bump.png testdata/sponza/textures/lion_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_c_spec.png testdata/sponza/textures/sponza_column_c_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_a_bump.png testdata/sponza/textures/sponza_column_a_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_fabric_diff.png testdata/sponza/textures/sponza_fabric_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/chain_texture_bump.png testdata/sponza/textures/chain_texture_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_floor_a_spec.png testdata/sponza/textures/sponza_floor_a_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_round_bump.png testdata/sponza/textures/vase_round_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_thorn_spec.png testdata/sponza/textures/sponza_thorn_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_thorn_mask.png testdata/sponza/textures/sponza_thorn_mask.png COPYONLY)
#configure_file(testdata/sponza/textures/chain_texture.png testdata/sponza/textures/chain_texture.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_plant_mask.png testdata/sponza/textures/vase_plant_mask.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_b_diff.png testdata/sponza/textures/sponza_column_b_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_dif.png testdata/sponza/textures/vase_dif.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_fabric_green_diff.png testdata/sponza/textures/sponza_fabric_green_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_fabric_spec.png testdata/sponza/textures/sponza_fabric_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_column_c_bump.png testdata/sponza/textures/sponza_column_c_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_arch_spec.png testdata/sponza/textures/sponza_arch_spec.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_curtain_blue_diff.png testdata/sponza/textures/sponza_curtain_blue_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/background_bump.png testdata/sponza/textures/background_bump.png COPYONLY)
#configure_file(testdata/sponza/textures/sponza_flagpole_diff.png testdata/sponza/textures/sponza_flagpole_diff.png COPYONLY)
#configure_file(testdata/sponza/textures/vase_plant_spec.png testdata/sponza/textures/vase_plant_spec.png COPYONLY)

configure_file(testdata/head256x256x109            testdata/head256x256x109            COPYONLY)

configure_file(testdata/skybox/top.jpg    testdata/skybox/top.jpg    COPYONLY)
configure_file(testdata/skybox/bottom.jpg testdata/skybox/bottom.jpg COPYONLY)
configure_file(testdata/skybox/front.jpg  testdata/skybox/front.jpg  COPYONLY)
configure_file(testdata/skybox/back.jpg   testdata/skybox/back.jpg   COPYONLY)
configure_file(testdata/skybox/left.jpg   testdata/skybox/left.jpg   COPYONLY)
configure_file(testdata/skybox/right.jpg  testdata/skybox/right.jpg  COPYONLY)

configure_file(testdata/testfont.ttf    testdata/testfont.ttf    COPYONLY)
configure_file(testdata/lorem_ipsum.txt testdata/lorem_ipsum.txt COPYONLY)

#configure_file(testdata/dragon.obj                testdata/dragon.obj                  COPYONLY)



# -------------------------------------
# Dependency Setup
# -------------------------------------
if (NOT MSVC)
	set(SL_EXTERNAL_LIBS
		${ASSIMP_LIBRARIES}
		${FREEIMAGE_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xext_LIB}
		pthread
	)
else()
	set(SL_EXTERNAL_LIBS
        ${ASSIMP_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
	)
endif()



# -------------------------------------
# Building and Linking Targets
# -------------------------------------
function(sl_add_test target sources)
    add_executable(${target} ${sources} ${ARGN})
    target_link_libraries(${target} softlight LightSky::Math LightSky::Game LightSky::Utils LightSky::Setup ${SL_EXTERNAL_LIBS})
endfunction(sl_add_test)

sl_add_test(sl_animation_test        sl_animation_test.cpp)
sl_add_test(sl_color_convert         sl_color_convert.cpp)
sl_add_test(sl_draw_test             sl_draw_test.cpp)
sl_add_test(sl_fullscreen_quad       sl_fullscreen_quad.cpp)
sl_add_test(sl_instancing_test       sl_instancing_test.cpp)
sl_add_test(sl_line_drawing          sl_line_drawing.cpp)
sl_add_test(sl_large_scene_test      sl_large_scene_test.cpp)
sl_add_test(sl_mesh_test             sl_mesh_test.cpp)
sl_add_test(sl_mrt_test              sl_mrt_test.cpp)
sl_add_test(sl_octree_test           sl_octree_test.cpp)
sl_add_test(sl_octree_rendering_test sl_octree_rendering_test.cpp)
sl_add_test(sl_packed_normal_test    sl_packed_normal_test.cpp)
sl_add_test(sl_scanline_offset_test  sl_scanline_offset_test.cpp)
sl_add_test(sl_sdf_image_test        sl_sdf_image_test.cpp sl_sdf_generator.hpp sl_sdf_generator.cpp)
sl_add_test(sl_scene_info_test       sl_scene_info_test.cpp)
sl_add_test(sl_screen_tile_test      sl_screen_tile_test.cpp)
sl_add_test(sl_shading_test          sl_shading_test.cpp)
sl_add_test(sl_skybox_test           sl_skybox_test.cpp)
sl_add_test(sl_text_test             sl_text_test.cpp)
sl_add_test(sl_vertex_chunking_test  sl_vertex_chunking_test.cpp)
sl_add_test(sl_vertex_info           sl_vertex_info.cpp)
sl_add_test(sl_volume_rendering_test sl_volume_rendering_test.cpp)
sl_add_test(sl_window_test           sl_window_test.cpp)
sl_add_test(sl_z_curve_test          sl_z_curve_test.cpp)
