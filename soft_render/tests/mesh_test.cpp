
#include <iostream>
#include <iomanip> // std::setprecision
#include <limits>
#include <thread>

#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_Sampler.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1920
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 1080
#endif /* IMAGE_HEIGHT */

#ifndef SR_TEST_MAX_THREADS
    #define SR_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SR_TEST_MAX_THREADS */



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
struct MeshTestUniforms
{
    const SR_Texture* pTexture;
    math::vec4        lightPos;
    SR_ColorRGBAf     lightCol;
    math::mat4        modelMatrix;
    math::mat4        mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _mesh_test_vert_shader(SR_VertexParam& param)
{
    const MeshTestUniforms* pUniforms = param.pUniforms->as<MeshTestUniforms>();
    const math::vec3&       vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::vec2&       uv        = *(param.pVbo->element<const math::vec2>(param.pVao->offset(1, param.vertId)));
    const math::vec3&       norm      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(2, param.vertId)));

    param.pVaryings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
    param.pVaryings[1] = math::vec4{uv.v[0], uv.v[1], 0.f, 0.f};
    param.pVaryings[2] = math::normalize(pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f});

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader mesh_test_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode    = SR_CULL_OFF;
    shader.shader      = _mesh_test_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _mesh_test_frag_shader(SR_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const math::vec4        pos       = fragParams.pVaryings[0];
    const math::vec4        uv        = fragParams.pVaryings[1];
    const math::vec4        norm      = math::normalize(fragParams.pVaryings[2]);
    const SR_Texture*       albedo    = pUniforms->pTexture;
    math::vec4              pixel;

    // normalize the texture colors to within (0.f, 1.f)
    math::vec3_t<uint8_t>&& pixel8 = sr_sample_nearest<math::vec3_t<uint8_t>, SR_WrapMode::REPEAT>(*albedo, uv[0], uv[1]);
    math::vec4_t<uint8_t> pixelF{pixel8[0], pixel8[1], pixel8[2], 200};
    pixel = color_cast<float, uint8_t>(pixelF);

    // Light direction calculation
    math::vec4&& lightDir = math::normalize(pUniforms->lightPos - pos);

    // Diffuse light calculation
    const float lightAngle = math::max(0.5f + math::dot(lightDir, norm) * 0.5f, 0.f);

    // output composition
    pixel = pixel * pUniforms->lightCol * lightAngle;
    fragParams.pOutputs[0] = math::min(pixel, math::vec4{1.f});

    return true;
}



SR_FragmentShader mesh_test_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_PREMULTIPLED_ALPHA;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_OFF;
    shader.shader      = _mesh_test_frag_shader;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> mesh_test_create_context()
{
    int retCode = 0;

    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;

    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SR_TEST_MAX_THREADS);
    assert(retCode == (int)std::thread::hardware_concurrency());

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    assert(retCode == 0);

    retCode = fbo.attach_color_buffer(0, tex);
    assert(retCode == 0);

    retCode = fbo.attach_depth_buffer(depth);
    assert(retCode == 0);

    fbo.clear_color_buffers();
    fbo.clear_depth_buffer();

    retCode = fbo.valid();
    assert(retCode == 0);

    retCode = meshLoader.load("testdata/heart/heart.obj");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    // Always make sure the scene graph is updated before rendering
    pGraph->mCurrentTransforms[0].scale(math::vec3{1.f});
    pGraph->update();

    const SR_VertexShader&&   vertShader = mesh_test_vert_shader();
    const SR_FragmentShader&& fragShader = mesh_test_frag_shader();

    size_t uboId = context.create_ubo();
    SR_UniformBuffer& ubo = context.ubo(uboId);
    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();

    pUniforms->lightPos = math::vec4{20.f, 100.f, 20.f, 0.f};
    pUniforms->lightCol = math::vec4{1.f, 0.9f, 0.8f, 1.f};
    size_t testShaderId = context.create_shader(vertShader,  fragShader,  uboId);

    assert(testShaderId == 0);
    (void)testShaderId;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void mesh_test_render(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context&       context   = pGraph->mContext;
    MeshTestUniforms* pUniforms = context.ubo(0).as<MeshTestUniforms>();

    for (SR_SceneNode& n : pGraph->mNodes)
    {
        // Only mesh nodes should be sent for rendering.
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SR_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];
            pUniforms->pTexture = material.pTextures[SR_MATERIAL_TEXTURE_DIFFUSE];

            // NOTE: Always validate your IDs in production
            const size_t shaderId = 0;
            const size_t fboid    = 0;

            context.draw(m, shaderId, fboid);
        }
    }
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
int main()
{
    int retCode = 0;

    utils::Pointer<SR_SceneGraph> pGraph{std::move(mesh_test_create_context())};

    SR_Context& context = pGraph->mContext;

    SR_Texture& tex = context.texture(0);
    SR_Texture& depth = context.texture(1);

    const math::mat4&& viewMatrix = math::look_at(math::vec3{10.f, 30.f, 70.f}, math::vec3{0.f, 20.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(80.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    ls::utils::Clock<float> timer;
    timer.start();

    constexpr int numFrames = 600;
    for (int i = 0; i < numFrames; ++i)
    {
        context.clear_framebuffer(0, 0, SR_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);
        mesh_test_render(pGraph.get(), projMatrix*viewMatrix);
    }
    timer.tick();

    std::cout
        << "Rendered " << numFrames << " frames in "
        << std::setprecision(std::numeric_limits<float>::digits10) << timer.tick_time().count()
        << " seconds." << std::endl;

    retCode = sr_img_save_ppm(tex.width(), tex.height(), reinterpret_cast<const SR_ColorRGB8*>(tex.data()), "mesh_test_image.ppm");
    assert(retCode == 0);

    retCode = sr_img_save_ppm(depth.width(), depth.height(), reinterpret_cast<const SR_ColorRf*>(depth.data()), "mesh_test_depth.ppm");
    assert(retCode == 0);

    return retCode;
}
