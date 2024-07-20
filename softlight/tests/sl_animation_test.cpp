
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_Animation.hpp"
#include "softlight/SL_AnimationChannel.hpp"
#include "softlight/SL_AnimationPlayer.hpp"
#include "softlight/SL_BoundingBox.hpp"
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
#include "softlight/SL_Swapchain.hpp"
#include "softlight/SL_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */

#ifndef SL_TEST_MAX_THREADS
    #define SL_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SL_TEST_MAX_THREADS */

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
    const math::mat4* pBones;
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
math::vec4 _untextured_vert_shader_impl(SL_VertexParam& param)
{
    typedef Tuple<math::vec3, int32_t> Vertex;

    const AnimUniforms* pUniforms   = param.pUniforms->as<AnimUniforms>();
    const Vertex* const v           = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert        = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&  norm        = sl_unpack_vec4_10_10_10_2I(v->const_element<1>());

    const math::vec4&& pos = pUniforms->modelMatrix * vert;

    param.pVaryings[0] = pos;
    param.pVaryings[1] = pUniforms->modelMatrix * norm;

    return pUniforms->vpMatrix * pos;
}



SL_VertexShader untextured_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _untextured_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _untextured_frag_shader(SL_FragmentParam& fragParam)
{
    const AnimUniforms*  pUniforms = fragParam.pUniforms->as<AnimUniforms>();
    const math::vec4&    pos       = fragParam.pVaryings[0];
    const math::vec4&&   norm      = math::normalize(fragParam.pVaryings[1]);
    const math::vec4     ambient   = {0.5f, 0.5f, 0.5f, 1.f};

    // Light direction calculation
    math::vec4          lightDir   = math::normalize(pUniforms->camPos - pos);
    const float         lightAngle = 0.5f * math::dot(-lightDir, norm) + 0.5f;
    const math::vec4&&  diffuse    = math::vec4{1.f} * lightAngle;

    const math::vec4 rgba = ambient+diffuse;
    fragParam.pOutputs[0] = math::min(rgba, math::vec4{1.f});

    return true;
}



SL_FragmentShader untextured_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _untextured_frag_shader;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _textured_vert_shader_impl(SL_VertexParam& param)
{
    typedef Tuple<math::vec3, math::vec2h, int32_t> Vertex;

    const AnimUniforms* pUniforms   = param.pUniforms->as<AnimUniforms>();
    const Vertex* const v           = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert        = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec2h   uv          = v->const_element<1>();
    const math::vec4&&  norm        = sl_unpack_vec4_10_10_10_2I(v->const_element<2>());
    const math::mat4&   modelPos    = pUniforms->modelMatrix;

    const math::vec4&& pos = modelPos * vert;

    param.pVaryings[0] = pos;
    param.pVaryings[1] = math::vec4_cast((math::vec2)uv, 0.f, 0.f);
    param.pVaryings[2] = modelPos * norm;

    return pUniforms->vpMatrix * pos;
}



SL_VertexShader textured_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _textured_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _textured_skin_vert_shader_impl(SL_VertexParam& param)
{
    typedef Tuple<math::vec3, math::vec2h, int32_t, math::vec4s, math::vec4h> Vertex;

    const AnimUniforms* pUniforms   = param.pUniforms->as<AnimUniforms>();
    const math::mat4*   pBones      = pUniforms->pBones;

    const Vertex* const v           = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert        = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec2h   uv          = v->const_element<1>();
    const math::vec4&&  norm        = sl_unpack_vec4_10_10_10_2I(v->const_element<2>());
    const math::vec4s   boneIds     = v->const_element<3>();
    const math::vec4&&  boneWeights = (math::vec4)v->const_element<4>();

    math::mat4&& boneTrans =
        (pBones[boneIds[0]] * boneWeights[0]) +
        (pBones[boneIds[1]] * boneWeights[1]) +
        (pBones[boneIds[2]] * boneWeights[2]) +
        (pBones[boneIds[3]] * boneWeights[3]);

    const math::mat4&& modelPos = pUniforms->modelMatrix * boneTrans;
    const math::vec4&& pos = modelPos * vert;

    param.pVaryings[0] = pos;
    param.pVaryings[1] = math::vec4_cast((math::vec2)uv, 0.f, 0.f);
    param.pVaryings[2] = modelPos * norm;

    return pUniforms->vpMatrix * pos;
}



SL_VertexShader textured_skin_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _textured_skin_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _textured_frag_shader(SL_FragmentParam& fragParam)
{
    const AnimUniforms*  pUniforms = fragParam.pUniforms->as<AnimUniforms>();
    const math::vec4&    pos       = fragParam.pVaryings[0];
    const math::vec4&    uv        = fragParam.pVaryings[1];
    const math::vec4&&   norm      = math::normalize(fragParam.pVaryings[2]);
    const SL_Texture*    pTexture  = pUniforms->pTexture;
    const math::vec4     ambient   = {0.5f, 0.5f, 0.5f, 1.f};
    math::vec4           albedo;

    // normalize the texture colors to within (0.f, 1.f)
    if (!pTexture)
    {
        albedo = math::vec4{1.f, 1.f, 1.f, 1.f};
    }
    else if (pTexture->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = sl_sample_bilinear<math::vec3_t<uint8_t>, SL_WrapMode::REPEAT>(*pTexture, uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        albedo = color_cast<float, uint8_t>(pixelF);
    }
    else
    {
        const math::vec4_t<uint8_t>&& pixelF = sl_sample_bilinear<math::vec4_t<uint8_t>, SL_WrapMode::REPEAT>(*pTexture, uv[0], uv[1]);
        albedo = color_cast<float, uint8_t>(pixelF);
    }

    // Light direction calculation
    math::vec4          lightDir   = math::normalize(pUniforms->camPos - pos);
    const float         lightAngle = 0.5f * math::dot(-lightDir, norm) + 0.5f;
    const math::vec4&&  diffuse    = math::vec4{1.f} * lightAngle;

    const math::vec4 rgba = albedo * (ambient+diffuse);
    fragParam.pOutputs[0] = math::min(rgba, math::vec4{1.f});

    return true;
}



SL_FragmentShader textured_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _textured_frag_shader;

    return shader;
}



/*-------------------------------------
 *
-------------------------------------*/
void setup_animations(SL_SceneGraph& graph, SL_AnimationPlayer& animPlayer)
{
    SL_AlignedVector<SL_Animation>& sceneAnims = graph.mAnimations;

    for (SL_AlignedVector<SL_AnimationChannel>& animList : graph.mNodeAnims)
    {
        for (SL_AnimationChannel& track : animList)
        {
            track.mAnimMode = SL_AnimationFlag::SL_ANIM_FLAG_INTERPOLATE;
        }
    }

    std::cout << "Running " << sceneAnims.size() << " animations." << std::endl;

    animPlayer.set_play_state(SL_AnimationState::SL_ANIM_STATE_PLAYING);
    animPlayer.set_num_plays(SL_AnimationPlayer::PLAY_ONCE);
    animPlayer.set_time_dilation(1.f);
}



/*-------------------------------------
 * Animation updating
-------------------------------------*/
void update_animations(SL_SceneGraph& graph, SL_AnimationPlayer& animPlayer, unsigned& currentAnimId, int64_t tickTime)
{
    if (graph.mAnimations.empty())
    {
        return;
    }

    // Play the current animation until it stops. Then move onto the next animation.
    if (animPlayer.is_stopped())
    {
        std::cout << "Completed animation " << currentAnimId << ". ";
        SL_AlignedVector<SL_Animation>& animations = graph.mAnimations;
        currentAnimId = (currentAnimId + 1) % animations.size();

        // reset the transformations in graph graph to those at the beginning of an animation
        SL_Animation& initialState = graph.mAnimations[currentAnimId];
        initialState.init(graph);

        animPlayer.set_play_state(SL_AnimationState::SL_ANIM_STATE_PLAYING);
        animPlayer.set_num_plays(SL_AnimationPlayer::PLAY_ONCE);

        std::cout << "Now playing animation " << currentAnimId << '.' << std::endl;
    }

    animPlayer.tick(graph, currentAnimId, tickTime);
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 50.f;

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
void render_scene(SL_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SL_Context& context = pGraph->mContext;
    AnimUniforms* pUniforms = context.ubo(0).as<AnimUniforms>();

    pUniforms->vpMatrix = vpMatrix;

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

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SL_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SL_Material&    material   = pGraph->mMaterials[m.materialId];
            const SL_VertexArray& vao = context.vao(m.vaoId);

            if (!(m.mode & SL_RenderMode::RENDER_MODE_TRIANGLES))
            {
                continue;
            }

            pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_DIFFUSE];

            const size_t skeletonIndex = pGraph->mMeshSkeletons[nodeMeshId].index;
            const size_t skeletonCount = pGraph->mMeshSkeletons[nodeMeshId].count;

            if (skeletonCount > 0)
            {
                pUniforms->pBones = pGraph->mModelMatrices.data() + skeletonIndex;
                context.draw(m, 2, 0); // pos, uv, norm, bone IDs, bone weights
            }
            else
            {
                pUniforms->pBones = nullptr;
                if (vao.num_bindings() == 3)
                {
                    // pos, uv, norm
                    context.draw(m, 1, 0);
                }
                else
                {
                    // pos, norm
                    context.draw(m, 0, 0);
                }
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
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_HALF, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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
    opts.packUvs = true;
    opts.packNormals = true;
    opts.packBoneIds = true;
    opts.packBoneWeights = true;
    opts.genSmoothNormals = true;

    retCode = meshLoader.load("testdata/bob/Bob.md5mesh", opts);
    LS_ASSERT(retCode != 0);

    meshLoader.data().mCurrentTransforms[1].rotate(math::vec3{0.f, 0.f, LS_PI_OVER_4});
    meshLoader.data().mCurrentTransforms[1].position(math::vec3{-30.f, -30.f, 0.f});
    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 0);

    retCode = meshLoader.load("testdata/rover/testmesh.dae", opts);
    LS_ASSERT(retCode != 0);

    meshLoader.data().mCurrentTransforms[0].rotate(math::vec3{0.f, 0.f, LS_PI_OVER_2});
    meshLoader.data().mCurrentTransforms[0].position(math::vec3{0.f, 0.f, -50.f});
    meshLoader.data().mCurrentTransforms[0].scale(math::vec3{20.f});
    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode != 0);

    pGraph->update();

    const SL_VertexShader&&   noTexVertShader  = untextured_vert_shader();
    const SL_FragmentShader&& noTexFragShader  = untextured_frag_shader();

    const SL_VertexShader&&   texVertShader    = textured_vert_shader();
    const SL_FragmentShader&& texFragShader    = textured_frag_shader();

    const SL_VertexShader&&   texSkinVertShader = textured_skin_vert_shader();

    size_t uboId = context.create_ubo();
    LS_ASSERT(uboId == 0);

    size_t noTexShaderId  = context.create_shader(noTexVertShader,  noTexFragShader,  uboId);
    LS_ASSERT(noTexShaderId == 0);
    (void)noTexShaderId;

    size_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  uboId);
    LS_ASSERT(texShaderId == 1);
    (void)texShaderId;

    size_t skinTexShaderId  = context.create_shader(texSkinVertShader,  texFragShader,  uboId);
    LS_ASSERT(skinTexShaderId == 2);
    (void)skinTexShaderId;

    (void)retCode;
    return pGraph;
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    utils::Pointer<SL_RenderWindow> pWindow{std::move(SL_RenderWindow::create())};
    utils::Pointer<SL_Swapchain>    pSwapchain{SL_Swapchain::create(pWindow->backend())};
    utils::Pointer<SL_SceneGraph>   pGraph{std::move(create_context())};
    utils::Pointer<bool[]>          pKeySyms{new bool[65536]};

    std::fill_n(pKeySyms.get(), 65536, false);

    SL_Context& context = pGraph->mContext;
    SL_AnimationPlayer animPlayer;
    unsigned currentAnimId = 0;

    setup_animations(*pGraph, animPlayer);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<int64_t, std::milli> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;

    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    camTrans.look_at(math::vec3{75.f}, math::vec3{0.f, 30.f, 0.f}, math::vec3{0.f, 1.f, 0.f}, true);
    math::mat4 projMatrix = math::infinite_perspective(math::radians(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

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

    if (pSwapchain->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Mesh Test") != 0)
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
                pSwapchain->terminate();
                pSwapchain->init(*pWindow, pWindow->width(), pWindow->height());
                context.texture(0).init(context.texture(0).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());
                context.texture(1).init(context.texture(1).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height());

                SL_Framebuffer& fbo = context.framebuffer(0);
                fbo.attach_color_buffer(0, context.texture(0).view());
                fbo.attach_depth_buffer(context.texture(1).view());

                projMatrix = math::infinite_perspective(math::radians(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_DOWN)
            {
                const SL_KeySymbol keySym = evt.keyboard.keySym;
                pKeySyms[keySym] = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keySym;
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
            const int64_t tickTime = timer.tick_time().count();

            ++currFrames;
            ++totalFrames;

            float tickTimef = (float)tickTime / 1000.f;
            currSeconds += tickTimef;
            totalSeconds += tickTimef;

            if (currSeconds >= 0.5f)
            {
                std::cout << "MS/F: " << utils::to_str(1000.f*(currSeconds/(float)currFrames)) << std::endl;
                //std::cout << "FPS: " << utils::to_str((float)currFrames/currSeconds) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTimef, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                AnimUniforms* pUniforms = context.ubo(0).as<AnimUniforms>();
                const math::vec3 camTransPos = camTrans.position();
                pUniforms->camPos = math::vec4_cast(-camTransPos, 1.f);
            }

            const math::mat4&& vpMatrix = projMatrix * camTrans.transform();

            update_animations(*pGraph, animPlayer, currentAnimId, tickTime);
            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);
            render_scene(pGraph.get(), vpMatrix);

            context.blit(pSwapchain->texture().view(), 0);
            pWindow->render(*pSwapchain);
        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            std::cout << "Window close state encountered. Exiting." << std::endl;
            shouldQuit = true;
        }
    }

    pSwapchain->terminate();

    std::cout
        << "Rendered " << totalFrames << " frames in " << totalSeconds << " seconds ("
        << ((double)totalFrames/(double)totalSeconds) << " average fps)." << std::endl;

    return pWindow->destroy();
}
