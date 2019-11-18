
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
#
configure_file(testdata/bob/Bob.md5anim       testdata/bob/Bob.md5anim       COPYONLY)
configure_file(testdata/bob/Bob.md5mesh       testdata/bob/Bob.md5mesh       COPYONLY)
configure_file(testdata/bob/bob.source.txt    testdata/bob/bob.source.txt    COPYONLY)
configure_file(testdata/bob/guard1_body.png   testdata/bob/guard1_body.png   COPYONLY)
configure_file(testdata/bob/guard1_face.png   testdata/bob/guard1_face.png   COPYONLY)
configure_file(testdata/bob/guard1_helmet.png testdata/bob/guard1_helmet.png COPYONLY)
configure_file(testdata/bob/iron_grill.png    testdata/bob/iron_grill.png    COPYONLY)
configure_file(testdata/bob/round_grill.png   testdata/bob/round_grill.png   COPYONLY)
#
#configure_file(testdata/african_head/african_head.obj testdata/african_head/african_head.obj COPYONLY)
#configure_file(testdata/african_head/african_head.mtl testdata/african_head/african_head.mtl COPYONLY)
#configure_file(testdata/african_head/african_head.ppm testdata/african_head/african_head.ppm COPYONLY)
#
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

configure_file(testdata/dragon.obj                testdata/dragon.obj                  COPYONLY)



# -------------------------------------
# Dependency Setup
# -------------------------------------
if (NOT MSVC)
	set(SR_EXTERNAL_LIBS
		${ASSIMP_LIBRARIES}
		${FREEIMAGE_LIBRARIES}
        ${X11_LIBRARIES}
        ${X11_Xext_LIB}
		pthread
	)
else()
	set(SR_EXTERNAL_LIBS
        ${ASSIMP_LIBRARIES}
        ${FREEIMAGE_LIBRARIES}
	)
endif()



# -------------------------------------
# Building and Linking Targets
# -------------------------------------
# See if we can enable link-time optimizations
include(CheckIPOSupported OPTIONAL RESULT_VARIABLE CMAKE_ENABLE_LTO)
if (CMAKE_ENABLE_LTO)
	check_ipo_supported(RESULT ENABLE_LTO OUTPUT LTO_ERR)
	if(NOT ENABLE_LTO)
		message(WARNING "Cannot enable link-time optimization: ${LTO_ERR}")
	endif()
endif()

function(sr_add_test target sources)
    add_executable(${target} ${sources} ${ARGN})
    target_link_libraries(${target} soft_render LightSky::Math LightSky::Game LightSky::Utils LightSky::Setup ${SR_EXTERNAL_LIBS})

	if(ENABLE_LTO)
		set_property(TARGET ${target} PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
	endif()
endfunction(sr_add_test)

sr_add_test(animation_test          animation_test.cpp)
sr_add_test(color_convert           color_convert.cpp)
sr_add_test(draw_test               draw_test.cpp)
sr_add_test(line_drawing            line_drawing.cpp)
sr_add_test(live_scene_test         live_scene_test.cpp)
sr_add_test(mesh_test               mesh_test.cpp)
sr_add_test(scanline_offset_test    scanline_offset_test.cpp)
sr_add_test(scene_info_test         scene_info_test.cpp)
sr_add_test(screen_tile_test        screen_tile_test.cpp)
sr_add_test(sr_vertex_chunking_test sr_vertex_chunking_test.cpp)
sr_add_test(sr_vertex_info          sr_vertex_info.cpp)
sr_add_test(volume_test             volume_test.cpp)
sr_add_test(window_test             window_test.cpp)
sr_add_test(z_curve_test            z_curve_test.cpp)
