
#include <iostream>
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_PackedVertex.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_WindowBuffer.hpp"
#include "softlight/SL_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1024
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 1024
#endif /* IMAGE_HEIGHT */

#ifndef SL_TEST_MAX_THREADS
    #define SL_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SL_TEST_MAX_THREADS */

#ifndef SL_BENCHMARK_SCENE
    #define SL_BENCHMARK_SCENE 0
#endif /* SL_BENCHMARK_SCENE */

#ifndef DEFAULT_INSTANCES_X
    #define DEFAULT_INSTANCES_X 5
#endif

#ifndef DEFAULT_INSTANCES_Y
    #define DEFAULT_INSTANCES_Y 5
#endif

#ifndef DEFAULT_INSTANCES_Z
    #define DEFAULT_INSTANCES_Z 5
#endif

#ifndef DEFAULT_INSTANCES
    #define DEFAULT_INSTANCES (DEFAULT_INSTANCES_X*DEFAULT_INSTANCES_Y*DEFAULT_INSTANCES_Z)
#endif

namespace ls
{
namespace math
{
using vec4s = vec4_t<uint16_t>;
}
}

namespace math = ls::math;
namespace utils = ls::utils;

template <typename... data_t>
using Tuple = utils::Tuple<data_t...>;



/*-----------------------------------------------------------------------------
 * Structures to create uniform variables shared across all shader stages.
-----------------------------------------------------------------------------*/
struct AnimUniforms
{
    const SL_Texture* pTexture;
    math::vec4        ambient;
    math::vec4        diffuse;
    size_t            instanceId;
    utils::UniqueAlignedArray<math::mat4> instanceMatrix;
    math::mat4        modelMatrix;
    math::mat4        vpMatrix;
    math::vec4        camPos;
};




/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(SL_VertexParam& param)
{
    typedef Tuple<math::vec3, math::vec2, math::vec3> Vertex;

    const AnimUniforms* pUniforms   = param.pUniforms->as<AnimUniforms>();
    const Vertex* const v           = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert        = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&  uv          = math::vec4_cast(v->const_element<1>(), 0.f, 0.f);
    const math::vec4&&  norm        = math::vec4_cast(v->const_element<2>(), 0.f);

    const size_t       instanceId  = pUniforms->instanceId == SCENE_NODE_ROOT_ID ? param.instanceId : pUniforms->instanceId;
    const math::mat4&  instanceMat = pUniforms->instanceMatrix[instanceId];
    const math::mat4&& modelMat    = instanceMat * pUniforms->modelMatrix;
    const math::vec4&& pos         = modelMat * vert;
    const math::vec4&& n           = modelMat * norm;

    param.pVaryings[0] = pos;
    param.pVaryings[1] = uv;
    param.pVaryings[2] = n;

    return pUniforms->vpMatrix * pos;
}



SL_VertexShader texture_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader(SL_FragmentParam& fragParam)
{
    const AnimUniforms*  pUniforms = fragParam.pUniforms->as<AnimUniforms>();
    const math::vec4&    pos       = fragParam.pVaryings[0];
    const math::vec4&    uv        = fragParam.pVaryings[1];
    const math::vec4&&   norm      = math::normalize(fragParam.pVaryings[2]);
    const SL_Texture*    pTexture  = pUniforms->pTexture;
    const math::vec4&    ambient   = pUniforms->ambient;
    math::vec4           albedo;

    // normalize the texture colors to within (0.f, 1.f)
    {
        const math::vec3_t<uint8_t>&& pixel8 = sl_sample_bilinear<math::vec3_t<uint8_t>, SL_WrapMode::REPEAT>(*pTexture, uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        albedo = color_cast<float, uint8_t>(pixelF);
    }

    // Light direction calculation
    math::vec4          lightDir   = math::normalize(pUniforms->camPos - pos);
    const float         lightAngle = 0.5f * math::dot(-lightDir, norm) + 0.5f;
    const math::vec4&&  diffuse    = pUniforms->diffuse * lightAngle;
    const math::vec4&&  rgba       = albedo * (ambient+diffuse);

    fragParam.pOutputs[0] = math::min(rgba, math::vec4{1.f});

    return true;
}



SL_FragmentShader texture_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_PREMULTIPLED_ALPHA;
    shader.depthTest   = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader;

    return shader;
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 25.f;

    if (pKeys[SL_KeySymbol::KEY_SYM_w] || pKeys[SL_KeySymbol::KEY_SYM_W])
    {
        camTrans.move(math::vec3{0.f, 0.f, camSpeed * tickTime}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_s] || pKeys[SL_KeySymbol::KEY_SYM_S])
    {
        camTrans.move(math::vec3{0.f, 0.f, -camSpeed * tickTime}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_e] || pKeys[SL_KeySymbol::KEY_SYM_E])
    {
        camTrans.move(math::vec3{0.f, camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_q] || pKeys[SL_KeySymbol::KEY_SYM_Q])
    {
        camTrans.move(math::vec3{0.f, -camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_d] || pKeys[SL_KeySymbol::KEY_SYM_D])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_a] || pKeys[SL_KeySymbol::KEY_SYM_A])
    {
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }
}



/*-------------------------------------
 * Render the Scene
-------------------------------------*/
void render_scene(SL_SceneGraph* pGraph, const math::mat4& vpMatrix, bool useInstancing, size_t maxInstances)
{
    SL_Context& context = pGraph->mContext;
    AnimUniforms* pUniforms = context.ubo(0).as<AnimUniforms>();

    for (size_t nodeId = 0; nodeId < pGraph->mNodes.size(); ++nodeId)
    {
        const SL_SceneNode& n = pGraph->mNodes[nodeId];

        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->vpMatrix    = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t       nodeMeshId = meshIds[meshId];
            const SL_Mesh&     m          = pGraph->mMeshes[nodeMeshId];
            const SL_Material& material   = pGraph->mMaterials[m.materialId];

            pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_DIFFUSE];
            pUniforms->ambient = material.ambient;
            pUniforms->diffuse = material.diffuse;

            if (useInstancing)
            {
                pUniforms->instanceId = SCENE_NODE_ROOT_ID;
                context.draw_instanced(m, maxInstances, 0, 0);
            }
            else
            {
                for (size_t i = maxInstances; i--;)
                {
                    pUniforms->instanceId = i;
                    context.draw(m, 0, 0);
                }
            }
        }
    }
}



/*-----------------------------------------------------------------------------
 * Update the number of instances
-----------------------------------------------------------------------------*/
void update_instance_count(utils::Pointer<SL_SceneGraph>& pGraph, size_t instancesX, size_t instancesY, size_t instancesZ)
{
    SL_Context& context = pGraph->mContext;
    AnimUniforms* pUniforms = context.ubo(0).as<AnimUniforms>();

    size_t instanceCount = instancesX * instancesY * instancesZ;
    pUniforms->instanceMatrix = utils::make_unique_aligned_array<math::mat4>(instanceCount);

    for (size_t z = 0; z < instancesZ; ++z)
    {
        for (size_t y = 0; y < instancesY; ++y)
        {
            for (size_t x = 0; x < instancesX; ++x)
            {
                SL_Transform tempTrans;
                tempTrans.scale(0.125f);
                tempTrans.position(math::vec3{(float)x, (float)y, (float)z} * 25.f);
                tempTrans.apply_transform();

                const size_t index = x + (instancesX * y + (instancesX * instancesY * z));
                pUniforms->instanceMatrix[index] = tempTrans.transform();
            }
        }
    }
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> create_context()
{
    int retCode = 0;

    SL_SceneFileLoader meshLoader;
    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};
    SL_Context& context = pGraph->mContext;
    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SL_TEST_MAX_THREADS);
    LS_ASSERT(retCode == (int)SL_TEST_MAX_THREADS);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(1);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_color_buffer(0, tex.view());
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_depth_buffer(depth.view());
    LS_ASSERT(retCode == 0);

    fbo.clear_color_buffers();
    fbo.clear_depth_buffer();

    retCode = fbo.valid();
    LS_ASSERT(retCode == 0);

    SL_SceneLoadOpts opts = sl_default_scene_load_opts();
    opts.packUvs = false;
    opts.packNormals = false;
    opts.genSmoothNormals = true;
    retCode = meshLoader.load("testdata/heart/heart.obj", opts);
    LS_ASSERT(retCode != 0);

    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 0);

    pGraph->update();

    const SL_VertexShader&&   texVertShader  = texture_vert_shader();
    const SL_FragmentShader&& texFragShader  = texture_frag_shader();

    size_t uboId = context.create_ubo();
    LS_ASSERT(uboId == 0);
    update_instance_count(pGraph, DEFAULT_INSTANCES_X, DEFAULT_INSTANCES_Y, DEFAULT_INSTANCES_Z);

    size_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  uboId);
    LS_ASSERT(texShaderId == 0);
    (void)texShaderId;

    (void)retCode;
    return pGraph;
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    utils::Pointer<SL_RenderWindow> pWindow{std::move(SL_RenderWindow::create())};
    utils::Pointer<SL_WindowBuffer> pRenderBuf{SL_WindowBuffer::create()};
    utils::Pointer<SL_SceneGraph>   pGraph{std::move(create_context())};
    utils::Pointer<bool[]>          pKeySyms{new bool[65536]};

    std::fill_n(pKeySyms.get(), 65536, false);

    SL_Context& context = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    bool useInstancing = true;
    unsigned instancesX = DEFAULT_INSTANCES_X;
    unsigned instancesY = DEFAULT_INSTANCES_Y;
    unsigned instancesZ = DEFAULT_INSTANCES_Z;
    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    math::vec3 viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 30.f;
    camTrans.look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}, true);
    math::mat4 projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    if (shouldQuit)
    {
        return shouldQuit;
    }

    if (!pWindow->run())
    {
        std::cerr << "Unable to run the test window!" << std::endl;
        pWindow->destroy();
        return -1;
    }

    if (pRenderBuf->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Mesh Test") != 0)
    {
        return -2;
    }

    pWindow->set_keys_repeat(false); // text mode
    timer.start();

    while (!shouldQuit)
    {
        pWindow->update();
        SL_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SL_WinEventType::WIN_EVENT_MOVED)
            {
                std::cout << "Window moved: " << evt.window.x << 'x' << evt.window.y << std::endl;
            }

            if (evt.type == SL_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
                context.texture(0).init(context.texture(0).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                context.texture(1).init(context.texture(1).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
            }

            if (evt.type == SL_WinEventType::WIN_EVENT_KEY_DOWN)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = false;

                switch (keySym)
                {
                    case SL_KeySymbol::KEY_SYM_SPACE:
                        if (pWindow->state() == WindowStateInfo::WINDOW_RUNNING)
                        {
                            std::cout << "Space button pressed. Pausing." << std::endl;
                            pWindow->pause();
                        }
                        else
                        {
                            std::cout << "Space button pressed. Resuming." << std::endl;
                            pWindow->run();
                            timer.start();
                        }
                        break;

                    case SL_KeySymbol::KEY_SYM_LEFT:
                        pWindow->set_size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
                        break;

                    case SL_KeySymbol::KEY_SYM_RIGHT:
                        pWindow->set_size(IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;

                    case SL_KeySymbol::KEY_SYM_UP:
                        numThreads = math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_DOWN:
                        numThreads = math::max(numThreads - 1u, 1u);
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_F1:
                        pWindow->set_mouse_capture(!pWindow->is_mouse_captured());
                        pWindow->set_keys_repeat(!pWindow->keys_repeat()); // no text mode
                        std::cout << "Mouse Capture: " << pWindow->is_mouse_captured() << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_F2:
                        useInstancing = !useInstancing;
                        std::cout << "Instancing State: " << useInstancing << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_1:
                        instancesX = math::max<unsigned>(1, instancesX-1);
                        instancesY = math::max<unsigned>(1, instancesY-1);
                        instancesZ = math::max<unsigned>(1, instancesZ-1);
                        update_instance_count(pGraph, instancesX, instancesY, instancesZ);
                        viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 30.f;
                        camTrans.look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}, true);
                        std::cout << "Instance count decreased to (" << instancesX << 'x' << instancesY << 'x' << instancesZ << ") = " << instancesX*instancesY*instancesZ << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_2:
                        instancesX = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesX+1);
                        instancesY = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesY+1);
                        instancesZ = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesZ+1);
                        update_instance_count(pGraph, instancesX, instancesY, instancesZ);
                        viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 30.f;
                        camTrans.look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}, true);
                        std::cout << "Instance count decreased to (" << instancesX << 'x' << instancesY << 'x' << instancesZ << ") = " << instancesX*instancesY*instancesZ << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_ESCAPE:
                        std::cout << "Escape button pressed. Exiting." << std::endl;
                        shouldQuit = true;
                        break;

                    default:
                        break;
                }
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_CLOSING)
            {
                std::cout << "Window close event caught. Exiting." << std::endl;
                shouldQuit = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_MOVED)
            {
                if (pWindow->is_mouse_captured())
                {
                    SL_MousePosEvent& mouse = evt.mousePos;
                    dx = ((float)mouse.dx / (float)pWindow->width()) * -0.05f;
                    dy = ((float)mouse.dy / (float)pWindow->height()) * -0.05f;
                    camTrans.rotate(math::vec3{dx, dy, 0.f});
                }
            }
        }
        else
        {
            timer.tick();
            const float tickTime = timer.tick_time().count();

            ++currFrames;
            ++totalFrames;
            currSeconds += tickTime;
            totalSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "MS/F: " << utils::to_str(1000.f*(currSeconds/(float)currFrames)) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 5000)
                {
                    shouldQuit = true;
                }
            #endif

            update_cam_position(camTrans, tickTime, pKeySyms);

            AnimUniforms* pUniforms = context.ubo(0).as<AnimUniforms>();
            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                const math::vec3 camTransPos = camTrans.position();
                pUniforms->camPos = math::vec4_cast(camTransPos, 1.f);
            }

            for (size_t i = instancesX*instancesY*instancesZ; i--;) {
                pUniforms->instanceMatrix[i] = math::rotate(pUniforms->instanceMatrix[i], math::vec3{0.f, 1.f, 0.f}, tickTime);
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);
            const math::mat4&& vpMatrix = projMatrix * camTrans.transform();

            render_scene(pGraph.get(), vpMatrix, useInstancing, instancesX*instancesY*instancesZ);
            context.blit(pRenderBuf->texture().view(), 0);
            pWindow->render(*pRenderBuf);
        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            std::cout << "Window close state encountered. Exiting." << std::endl;
            shouldQuit = true;
        }
    }

    context.ubo(0).as<AnimUniforms>()->instanceMatrix.reset();
    pRenderBuf->terminate();

    std::cout
        << "Rendered " << totalFrames << " frames in " << totalSeconds << " seconds ("
        << ((double)totalFrames/(double)totalSeconds) << " average fps)." << std::endl;

    return pWindow->destroy();
}
