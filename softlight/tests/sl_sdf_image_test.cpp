
// Full-screen quad example using the "Compact YCoCg Frame Buffer" technique.

#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Log.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_WindowBuffer.hpp"
#include "softlight/SL_WindowEvent.hpp"

#include "sl_sdf_generator.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */

#ifndef SL_TEST_MAX_THREADS
    #define SL_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SL_TEST_MAX_THREADS */

#ifndef SL_BENCHMARK_SCENE
    #define SL_BENCHMARK_SCENE 0
#endif /* SL_BENCHMARK_SCENE */



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
struct MeshTestUniforms
{
    math::mat4 mvMatrix;
    math::mat4 mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _mesh_vert_shader(SL_VertexParam& param)
{
    typedef utils::Tuple<math::vec3, SL_PackedVertex_2_10_10_10> Vertex;

    const MeshTestUniforms* pUniforms = param.pUniforms->as<MeshTestUniforms>();
    const Vertex*           v         = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&      vert      = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&      norm      = (math::vec4)v->const_element<1>();

    param.pVaryings[0] = pUniforms->mvMatrix * norm;

    return pUniforms->mvpMatrix * vert;
}



SL_VertexShader mesh_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode    = SL_CULL_BACK_FACE;
    shader.shader      = _mesh_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _mesh_frag_shader(SL_FragmentParam& fragParams)
{
    const math::vec4&  norm       = math::normalize(fragParams.pVaryings[0]);
    const float        lightAngle = math::clamp(math::dot(math::vec4{0.f, 0.f, 1.f, 0.f}, norm), 0.f, 1.f);
    const math::vec4&& output     = math::vec4{1.f} * lightAngle;

    fragParams.pOutputs[0] = math::step(math::vec4{0.5f}, output);

    return true;
}



SL_FragmentShader mesh_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _mesh_frag_shader;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> mesh_test_create_context()
{
    int retCode = 0;

    SL_SceneFileLoader meshLoader;
    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};
    SL_Context& context = pGraph->mContext;

    size_t fboId = context.create_framebuffer();
    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    LS_ASSERT(retCode == 0);

    size_t texRgbId = context.create_texture();
    SL_Texture& texRgb = context.texture(texRgbId);
    retCode = texRgb.init(SL_ColorDataType::SL_COLOR_R_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_color_buffer(0, texRgb.view());
    LS_ASSERT(retCode == 0);

    size_t depthId  = context.create_texture();
    SL_Texture& texDepth = context.texture(depthId);
    retCode = texDepth.init(SL_ColorDataType::SL_COLOR_R_HALF, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_depth_buffer(texDepth.view());
    LS_ASSERT(retCode == 0);

    context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);

    retCode = fbo.valid();
    LS_ASSERT(retCode == 0);

    // texture 2
    size_t sdfId  = context.create_texture();
    SL_Texture& texSdf = context.texture(sdfId);
    retCode = texSdf.init(SL_ColorDataType::SDFDataType, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    // texture 3
    size_t sdfId1  = context.create_texture();
    SL_Texture& texSdf1 = context.texture(sdfId1);
    retCode = texSdf1.init(SL_ColorDataType::SDFScratchDataType, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_SceneLoadOpts&& opts = sl_default_scene_load_opts();
    opts.packNormals = true;
    retCode = meshLoader.load("testdata/towerG.obj", opts);
    LS_ASSERT(retCode != 0);

    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 0);

    // Always make sure the scene graph is updated before rendering
    pGraph->mCurrentTransforms[1].scale(math::vec3{4.f});
    pGraph->update();

    const SL_VertexShader&&   vertShader = mesh_vert_shader();
    const SL_FragmentShader&& fragShader = mesh_frag_shader();

    size_t uboId = context.create_ubo();
    SL_UniformBuffer& ubo = context.ubo(uboId);
    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();

    pUniforms->mvMatrix = math::mat4{1.f};
    pUniforms->mvpMatrix = math::mat4{1.f};
    size_t testShaderId = context.create_shader(vertShader, fragShader, uboId);

    LS_ASSERT(testShaderId == 0);
    (void)testShaderId;

    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void mesh_test_render(SL_SceneGraph* pGraph, const math::mat4& projectionMat, const math::mat4& viewMat)
{
    const math::mat4&& vpMatrix = projectionMat * viewMat;
    SL_Context&       context   = pGraph->mContext;
    MeshTestUniforms* pUniforms = context.ubo(0).as<MeshTestUniforms>();

    for (size_t i = 1; i < pGraph->mNodes.size(); ++i)
    {
        SL_SceneNode& n = pGraph->mNodes[i];

        // Only mesh nodes should be sent for rendering.
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[i];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->mvMatrix = viewMat * modelMat;
        pUniforms->mvpMatrix = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t   nodeMeshId = meshIds[meshId];
            const SL_Mesh& m          = pGraph->mMeshes[nodeMeshId];

            // NOTE: Always validate your IDs in production
            constexpr size_t shaderId = 0;
            constexpr size_t fboid    = 0;

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
    (void)retCode;

    utils::Pointer<SL_RenderWindow> pWindow{SL_RenderWindow::create()};
    utils::Pointer<SL_WindowBuffer> pRenderBuf{SL_WindowBuffer::create()};
    if (pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT))
    {
        LS_LOG_ERR("Unable to initialize a window.");
        return -1;
    }
    else if (!pWindow->run())
    {
        LS_LOG_ERR("Unable to run the test window!");
        pWindow->destroy();
        return -2;
    }
    else if (pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height()) != 0 || pWindow->set_title("Mesh Test") != 0)
    {
        LS_LOG_ERR("Unable to resize the test window buffer!");
        pWindow->destroy();
        return -3;
    }

    pWindow->set_keys_repeat(false); // text mode
    pWindow->set_mouse_capture(false);

    utils::Pointer<SL_SceneGraph>    pGraph{mesh_test_create_context()};
    ls::utils::Clock<float>          timer;
    SL_Transform                     viewMatrix;
    SL_WindowEvent                   evt;
    math::mat4         projMatrix     = math::infinite_perspective(LS_DEG2RAD(80.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
    SL_Context&        context        = pGraph->mContext;
    int                shouldQuit     = 0;
    int                numFrames      = 0;
    int                totalFrames    = 0;
    float              secondsCounter = 0.f;
    float              tickTime       = 0.f;

    (void)totalFrames;

    viewMatrix.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    viewMatrix.look_at(math::vec3{0.f, 40.f, 70.f}, math::vec3{0.f, 40.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    viewMatrix.apply_transform();

    timer.start();

    context.num_threads(SL_TEST_MAX_THREADS);

    while (!shouldQuit)
    {
        pWindow->update();

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SL_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());

                context.texture(0).init(context.texture(0).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                context.texture(1).init(context.texture(1).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                context.texture(2).init(context.texture(2).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());

                projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                if (keySym == SL_KeySymbol::KEY_SYM_ESCAPE)
                {
                    LS_LOG_MSG("Escape button pressed. Exiting.");
                    shouldQuit = true;
                }
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_CLOSING)
            {
                LS_LOG_MSG("Window close event caught. Exiting.");
                shouldQuit = true;
            }
        }
        else
        {
            timer.tick();
            tickTime = timer.tick_time().count();
            secondsCounter += tickTime;

            viewMatrix.rotate(math::vec3{-0.5f*tickTime, 0.f, 0.f});
            viewMatrix.apply_transform();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);

            mesh_test_render(pGraph.get(), projMatrix, viewMatrix.transform());

            sl_create_sdf(context.texture(0), context.texture(2), context.texture(3));
            context.blit(pRenderBuf->texture().view(), 2);
            pWindow->render(*pRenderBuf);

            ++numFrames;
            ++totalFrames;

            if (secondsCounter >= 1.f)
            {
                LS_LOG_MSG("FPS: ", utils::to_str((float)numFrames / secondsCounter));
                numFrames = 0;
                secondsCounter = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 3600)
                {
                    shouldQuit = true;
                }
            #endif
        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            LS_LOG_MSG("Window close state encountered. Exiting.");
            shouldQuit = true;
        }
    }

    pRenderBuf->terminate();
    return pWindow->destroy();
}
