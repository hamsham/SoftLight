
#include <iostream>

#include "lightsky/math/mat_utils.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_SceneGraph.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _line_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer*, math::vec4*)
{
    // Using a value of 1000 for the w-component to help with perspective correction.
    const math::vec3& vert = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    return math::ortho(0.f, 640.f, 0.f, 480.f, 0.01f, 100.f) * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader line_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode = SR_CULL_OFF;
    shader.shader = _line_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _line_frag_shader_impl(SR_FragmentParam& fragParam)
{
    fragParam.pOutputs[0] = ls::math::vec4{0.f, 1.f, 0.f, 1.f}; // green
    return true;
}



SR_FragmentShader line_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs = 1;
    shader.blend = SR_BLEND_OFF;
    shader.depthMask = SR_DEPTH_MASK_OFF;
    shader.depthTest = SR_DEPTH_TEST_OFF;
    shader.shader = _line_frag_shader_impl;

    return shader;
}



int main()
{
    int retCode = 0;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;

    retCode = context.num_threads(1);
    assert(retCode == 1);

    size_t fboId = context.create_framebuffer();
    size_t texId = context.create_texture();
    size_t depthId = context.create_texture();
    size_t vaoId = context.create_vao();
    size_t vboId = context.create_vbo();
    size_t iboId = context.create_ibo();

    pGraph->mMeshes.push_back(SR_Mesh());

    const SR_VertexShader&& vertShader = line_vert_shader();
    const SR_FragmentShader&& fragShader = line_frag_shader();
    size_t shaderId  = context.create_shader(vertShader,  fragShader);

    SR_VertexBuffer& vbo = context.vbo(vboId);
    float tri[3][3] = {
        {320.f, 120.f, 1.f},
        {160.f, 400.f, 1.f},
        {480.f, 400.f, 1.f}
    };

    const size_t numVboBytes = 3 * sr_bytes_per_vertex(SR_DataType::VERTEX_DATA_FLOAT, SR_Dimension::VERTEX_DIMENSION_3);
    retCode = vbo.init(numVboBytes);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }
    vbo.assign(tri, 0, numVboBytes);

    SR_IndexBuffer& ibo = context.ibo(iboId);
    unsigned char indices[3] = {0, 1, 2};
    retCode = ibo.init(3, SR_DataType::VERTEX_DATA_BYTE);
    if (retCode != 0)
    {
        std::cerr << "Error while creating an IBO: " << retCode << std::endl;
        abort();
    }
    ibo.assign(indices, 0, 3);

    SR_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    vao.set_index_buffer(iboId);
    retCode = vao.set_num_bindings(1);
    if (retCode != 1)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }
    vao.set_binding(0, 0, sizeof(float)*3, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, 640, 480, 1);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a color texture: " << retCode << std::endl;
        abort();
    }

    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, 640, 480, 1);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a depth texture: " << retCode << std::endl;
        abort();
    }

    SR_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    if (retCode != 0)
    {
        std::cerr << "Error while reserving FBO color buffers: " << retCode << std::endl;
        abort();
    }

    retCode = fbo.attach_color_buffer(0, tex);
    if (retCode != 0)
    {
        std::cerr << "Error while attaching a color buffer to an FBO: " << retCode << std::endl;
        abort();
    }

    retCode = fbo.attach_depth_buffer(depth);
    if (retCode != 0)
    {
        std::cerr << "Error while attaching a depth buffer to an FBO: " << retCode << std::endl;
        abort();
    }

    SR_Mesh& m = pGraph->mMeshes.front();
    m.elementBegin = 0;
    m.elementEnd = context.ibos().begin()->count();
    m.vaoId = vaoId;
    m.mode = RENDER_MODE_INDEXED_LINES;

    context.draw(m, shaderId, fboId);

    sr_img_save_ppm(640, 480, reinterpret_cast<const SR_ColorRGB8*>(tex.data()), "draw_test_image.ppm");

    return retCode;
}
