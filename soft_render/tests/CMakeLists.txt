
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
#configure_file(testdata/bob/Bob.md5anim       testdata/bob/Bob.md5anim       COPYONLY)
#configure_file(testdata/bob/Bob.md5mesh       testdata/bob/Bob.md5mesh       COPYONLY)
#configure_file(testdata/bob/bob.source.txt    testdata/bob/bob.source.txt    COPYONLY)
#configure_file(testdata/bob/guard1_body.png   testdata/bob/guard1_body.png   COPYONLY)
#configure_file(testdata/bob/guard1_face.png   testdata/bob/guard1_face.png   COPYONLY)
#configure_file(testdata/bob/guard1_helmet.png testdata/bob/guard1_helmet.png COPYONLY)
#configure_file(testdata/bob/iron_grill.png    testdata/bob/iron_grill.png    COPYONLY)
#configure_file(testdata/bob/round_grill.png   testdata/bob/round_grill.png   COPYONLY)
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

configure_file(testdata/head256x256x109            testdata/head256x256x109            COPYONLY)



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
add_library(sr_test_common OBJECT test_common.hpp test_common.cpp)

function(sr_add_test target sources)
    add_executable(${target} ${sources} ${ARGN})
    target_link_libraries(${target} soft_render lsmath lsutils lssetup ${SR_EXTERNAL_LIBS})
endfunction(sr_add_test)

sr_add_test(color_convert        color_convert.cpp)
sr_add_test(line_drawing         line_drawing.cpp)
sr_add_test(sr_vertex_info       sr_vertex_info.cpp)
sr_add_test(window_test          $<TARGET_OBJECTS:sr_test_common> window_test.cpp)
sr_add_test(draw_test            $<TARGET_OBJECTS:sr_test_common> draw_test.cpp)
sr_add_test(mesh_test            $<TARGET_OBJECTS:sr_test_common> mesh_test.cpp)
sr_add_test(window_buffer_test   $<TARGET_OBJECTS:sr_test_common> window_buffer_test.cpp)
sr_add_test(volume_test          volume_test.cpp)
sr_add_test(scanline_offset_test scanline_offset_test.cpp)
sr_add_test(screen_tile_test     screen_tile_test.cpp)
sr_add_test(z_curve_test         z_curve_test.cpp)

target_compile_definitions(z_curve_test PRIVATE SR_TEXTURE_Z_ORDERING=1)
