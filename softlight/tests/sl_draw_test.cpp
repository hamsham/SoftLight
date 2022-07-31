
#include <iostream>
#include <thread>

#include "lightsky/math/mat_utils.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_ImgFilePPM.hpp"
#include "softlight/SL_SceneGraph.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
struct ColoredVertex
{
    math::vec4 pos;
    math::vec4 color;
};

/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _line_vert_shader_impl(SL_VertexParam& param)
{
    const ColoredVertex& vert = *(param.pVbo->element<const ColoredVertex>(param.pVao->offset(0, param.vertId)));

    param.pVaryings[0] = vert.color;

    return vert.pos;
}



SL_VertexShader line_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _line_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _line_frag_shader_impl(SL_FragmentParam& fragParam)
{
    fragParam.pOutputs[0] = fragParam.pVaryings[0]; // green
    return true;
}



SL_FragmentShader line_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs = 1;
    shader.blend = SL_BLEND_OFF;
    shader.depthMask = SL_DEPTH_MASK_OFF;
    shader.depthTest = SL_DEPTH_TEST_OFF;
    shader.shader = _line_frag_shader_impl;

    return shader;
}



int main()
{
    int retCode = 0;
    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};
    SL_Context& context = pGraph->mContext;

    retCode = context.num_threads(1);
    LS_ASSERT(retCode == 1);

    size_t fboId = context.create_framebuffer();
    size_t texId = context.create_texture();
    size_t depthId = context.create_texture();
    size_t vaoId = context.create_vao();
    size_t vboId = context.create_vbo();
    size_t iboId = context.create_ibo();

    pGraph->mMeshes.push_back(SL_Mesh());

    const SL_VertexShader&& vertShader = line_vert_shader();
    const SL_FragmentShader&& fragShader = line_frag_shader();
    size_t shaderId  = context.create_shader(vertShader,  fragShader);

    SL_VertexBuffer& vbo = context.vbo(vboId);
    ColoredVertex tri[3] = {
        {{-0.5f, -0.5f, 0.f, 1.f}, {1.f, 0.f, 0.f, 1.f}},
        {{ 0.f,   0.5f, 0.f, 1.f}, {0.f, 1.f, 0.f, 1.f}},
        {{ 0.5f, -0.5f, 0.f, 1.f}, {0.f, 0.f, 1.f, 1.f}}
    };

    const size_t numVboBytes = sizeof(tri);
    retCode = vbo.init(numVboBytes);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }
    vbo.assign(tri, 0, numVboBytes);

    SL_IndexBuffer& ibo = context.ibo(iboId);
    unsigned char indices[3] = {0, 2, 1};
    retCode = ibo.init(3, SL_DataType::VERTEX_DATA_BYTE, indices);
    if (retCode != 0)
    {
        std::cerr << "Error while creating an IBO: " << retCode << std::endl;
        abort();
    }

    SL_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    vao.set_index_buffer(iboId);
    retCode = vao.set_num_bindings(1);
    if (retCode != 1)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    vao.set_binding(0, 0,                   sizeof(ColoredVertex), SL_Dimension::VERTEX_DIMENSION_4, SL_DataType::VERTEX_DATA_FLOAT);
    vao.set_binding(1, sizeof(math::vec4),  sizeof(ColoredVertex), SL_Dimension::VERTEX_DIMENSION_4, SL_DataType::VERTEX_DATA_FLOAT);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGB_8U, 640, 480, 1);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a color texture: " << retCode << std::endl;
        abort();
    }

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_FLOAT, 640, 480, 1);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a depth texture: " << retCode << std::endl;
        abort();
    }

    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    if (retCode != 0)
    {
        std::cerr << "Error while reserving FBO color buffers: " << retCode << std::endl;
        abort();
    }

    retCode = fbo.attach_color_buffer(0, tex.view());
    if (retCode != 0)
    {
        std::cerr << "Error while attaching a color buffer to an FBO: " << retCode << std::endl;
        abort();
    }
    fbo.clear_color_buffer(0, math::vec3_t<uint8_t>{255, 0, 255});

    retCode = fbo.attach_depth_buffer(depth.view());
    if (retCode != 0)
    {
        std::cerr << "Error while attaching a depth buffer to an FBO: " << retCode << std::endl;
        abort();
    }
    fbo.clear_depth_buffer(0.f);

    SL_Mesh& m = pGraph->mMeshes.front();
    m.elementBegin = 0;
    m.elementEnd = context.ibos().begin()->count();
    m.vaoId = vaoId;
    m.mode = RENDER_MODE_INDEXED_TRIANGLES;

    context.num_threads(std::thread::hardware_concurrency());
    context.draw(m, shaderId, fboId);

    sl_img_save_ppm(640, 480, reinterpret_cast<const SL_ColorRGB8*>(tex.data()), "draw_test_image.ppm");

    return retCode;
}
