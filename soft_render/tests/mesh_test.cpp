
#include <iostream>
#include <iomanip> // std::setprecision
#include <limits>

#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"

#include "test_common.hpp"



#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1000
#endif /* IMAGE_WIDTH  */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 1000
#endif /* IMAGE_HEIGHT  */



int main()
{
    int retCode = 0;

    utils::Pointer<SR_SceneGraph> pGraph{std::move(create_context())};

    SR_Context& context = pGraph->mContext;

    retCode = context.num_threads(4);
    assert(retCode == 4);

    SR_Texture& tex = context.texture(0);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Texture& depth = context.texture(1);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    const math::mat4&& viewMatrix = math::look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(45.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    ls::utils::Clock<float> timer;
    timer.start();

    constexpr int numFrames = 600;
    for (int i = 0; i < numFrames; ++i)
    {
        context.framebuffer(0).clear_color_buffers();
        context.framebuffer(0).clear_depth_buffer();
        render_scene(pGraph.get(), projMatrix*viewMatrix);
    }
    timer.tick();

    std::cout
        << "Rendered " << numFrames << " frames in "
        << std::setprecision(std::numeric_limits<float>::digits10) << timer.tick_time().count()
        << " seconds." << std::endl;

    retCode = sr_img_save_ppm(IMAGE_WIDTH, IMAGE_HEIGHT, reinterpret_cast<const SR_ColorRGB8*>(tex.data()), "mesh_test_image.ppm");
    assert(retCode == 0);

    retCode = sr_img_save_ppm(IMAGE_WIDTH, IMAGE_HEIGHT, reinterpret_cast<const SR_ColorRf*>(depth.data()), "mesh_test_depth.ppm");
    assert(retCode == 0);

    return retCode;
}
