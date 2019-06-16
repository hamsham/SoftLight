
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_Animation.hpp"
#include "soft_render/SR_AnimationChannel.hpp"
#include "soft_render/SR_AnimationPlayer.hpp"
#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_WindowBuffer.hpp"
#include "soft_render/SR_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */

#ifndef SR_TEST_MAX_THREADS
    #define SR_TEST_MAX_THREADS 4
#endif /* SR_TEST_MAX_THREADS */

namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Structures to create uniform variables shared across all shader stages.
-----------------------------------------------------------------------------*/
struct Light
{
    math::vec4 pos;
    math::vec4 ambient;
    math::vec4 diffuse;
    math::vec4 spot;
};



struct PointLight
{
    float constant;
    float linear;
    float quadratic;
    float padding;
};



struct SpotLight
{
    math::vec4 direction;
    float outerCutoff;
    float innerCutoff;
    float epsilon;
};



struct AnimUniforms : SR_UniformBuffer
{
    const SR_Texture* pTexture;

    math::mat4* pBones;

    math::vec4 camPos;
    math::mat4 modelMatrix;
    math::mat4 vpMatrix;
};



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _normal_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const AnimUniforms* pUniforms = static_cast<const AnimUniforms*>(uniforms);

    const math::vec3& vert = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec3& norm = *vbo.element<const math::vec3>(vao.offset(1, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4_cast(vert, 0.f);
    varyings[1] = pUniforms->modelMatrix * math::vec4_cast(norm, 0.f);

    return pUniforms->vpMatrix * pUniforms->modelMatrix * math::vec4_cast(vert, 1.f);
}



SR_VertexShader normal_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode    = SR_CULL_BACK_FACE;
    shader.shader      = _normal_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _normal_frag_shader_impl(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    const AnimUniforms* pUniforms     = static_cast<const AnimUniforms*>(uniforms);
    const math::vec4    pos           = varyings[0];
    const math::vec4    norm          = math::normalize(varyings[1]);

    const math::vec4 ambient = {0.1f, 0.1f, 0.1f, 1.f};

    // Light direction calculation
    math::vec4          lightDir   = math::normalize(pUniforms->camPos - pos);
    const float         lightAngle = 0.5f * math::dot(-lightDir, norm) + 0.5f;
    const math::vec4&&  diffuse    = math::vec4{1.f} * lightAngle;

    outputs[0] = math::min(ambient+diffuse, math::vec4{1.f});

    return true;
}



SR_FragmentShader normal_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _normal_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const AnimUniforms* pUniforms   = static_cast<const AnimUniforms*>(uniforms);
    const math::vec3&   vert        = *vbo.element<const math::vec3>( vao.offset(0, vertId));
    const math::vec2&   uv          = *vbo.element<const math::vec2>( vao.offset(1, vertId));
    const math::vec3&   norm        = *vbo.element<const math::vec3>( vao.offset(2, vertId));
#if 1
    const math::vec4i&  boneIds     = *vbo.element<const math::vec4i>(vao.offset(3, vertId));
    const math::vec4&   boneWeights = *vbo.element<const math::vec4>( vao.offset(4, vertId));

    const math::mat4*  pBones  = pUniforms->pBones;

    math::mat4 boneTrans = pBones[boneIds[0]] * boneWeights[0];
    boneTrans += pBones[boneIds[1]] * boneWeights[1];
    boneTrans += pBones[boneIds[2]] * boneWeights[2];
    boneTrans += pBones[boneIds[3]] * boneWeights[3];

    const math::mat4&& modelPos = pUniforms->modelMatrix * boneTrans;
#else
    const math::mat4& modelPos  = pUniforms->modelMatrix;
    const math::mat4& boneTrans = pUniforms->modelMatrix;
#endif
    varyings[0] = modelPos * math::vec4_cast(vert, 1.f);
    varyings[1] = math::vec4_cast(uv, 0.f, 0.f);
    varyings[2] = math::normalize(boneTrans * math::vec4_cast(norm, 0.f));

    return pUniforms->vpMatrix * modelPos * math::vec4_cast(vert, 1.f);
}



SR_VertexShader texture_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SR_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    const AnimUniforms*  pUniforms = static_cast<const AnimUniforms*>(uniforms);
    const math::vec4     pos       = varyings[0];
    const math::vec4     uv        = varyings[1];
    const math::vec4     norm      = varyings[2];
    const SR_Texture*    pTexture  = pUniforms->pTexture;
    const math::vec4     ambient   = {0.1f, 0.1f, 0.1f, 1.f};
    math::vec4           albedo;

    // normalize the texture colors to within (0.f, 1.f)
    if (pTexture->channels() == 3)
    {
        const math::vec3_t<uint8_t>&& pixel8 = pTexture->nearest<math::vec3_t<uint8_t>>(uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        albedo = color_cast<float, uint8_t>(pixelF);
    }
    else
    {
        const math::vec4_t<uint8_t>&& pixelF = pTexture->nearest<math::vec4_t<uint8_t>>(uv[0], uv[1]);
        albedo = color_cast<float, uint8_t>(pixelF);
    }

    // Light direction calculation
    math::vec4          lightDir   = math::normalize(pUniforms->camPos - pos);
    const float         lightAngle = 0.5f * math::dot(-lightDir, norm) + 0.5f;
    const math::vec4&&  diffuse    = math::vec4{1.f} * lightAngle;

    const math::vec4 rgba = albedo * (ambient+diffuse);
    outputs[0] = math::min(rgba, math::vec4{1.f});

    return true;
}



SR_FragmentShader texture_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader;

    return shader;
}



/*-------------------------------------
 *
-------------------------------------*/
void setup_animations(SR_SceneGraph& graph, SR_AnimationPlayer& animPlayer)
{
    std::vector<SR_Animation>& sceneAnims = graph.mAnimations;

    for (std::vector<SR_AnimationChannel>& animList : graph.mNodeAnims)
    {
        for (SR_AnimationChannel& track : animList)
        {
            track.mAnimMode = SR_AnimationFlag::SR_ANIM_FLAG_INTERPOLATE;
        }
    }

    std::cout << "Running " << sceneAnims.size() << " animations." << std::endl;

    animPlayer.set_play_state(SR_AnimationState::SR_ANIM_STATE_PLAYING);
    animPlayer.set_num_plays(SR_AnimationPlayer::PLAY_ONCE);
    animPlayer.set_time_dilation(1.f);
}



/*-------------------------------------
 * Animation updating
-------------------------------------*/
void update_animations(SR_SceneGraph& graph, SR_AnimationPlayer& animPlayer, unsigned& currentAnimId, float tickTime)
{
    if (graph.mAnimations.empty())
    {
        return;
    }

    // Play the current animation until it stops. Then move onto the next animation.
    if (animPlayer.is_stopped())
    {
        std::cout << "Completed animation " << currentAnimId << ". ";
        std::vector<SR_Animation>& animations = graph.mAnimations;
        currentAnimId = (currentAnimId + 1) % animations.size();

        // reset the transformations in graph graph to those at the beginning of an animation
        SR_Animation& initialState = graph.mAnimations[currentAnimId];
        initialState.init(graph);

        animPlayer.set_play_state(SR_AnimationState::SR_ANIM_STATE_PLAYING);
        animPlayer.set_num_plays(SR_AnimationPlayer::PLAY_ONCE);

        std::cout << "Now playing animation " << currentAnimId << '.' << std::endl;
    }

    animPlayer.tick(graph, currentAnimId, 1000u*tickTime);
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SR_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 10.f;

    if (pKeys[SR_KeySymbol::KEY_SYM_w] || pKeys[SR_KeySymbol::KEY_SYM_W])
    {
        camTrans.move(math::vec3{0.f, 0.f, camSpeed * tickTime}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_s] || pKeys[SR_KeySymbol::KEY_SYM_S])
    {
        camTrans.move(math::vec3{0.f, 0.f, -camSpeed * tickTime}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_e] || pKeys[SR_KeySymbol::KEY_SYM_E])
    {
        camTrans.move(math::vec3{0.f, camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_q] || pKeys[SR_KeySymbol::KEY_SYM_Q])
    {
        camTrans.move(math::vec3{0.f, -camSpeed * tickTime, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_d] || pKeys[SR_KeySymbol::KEY_SYM_D])
    {
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SR_KeySymbol::KEY_SYM_a] || pKeys[SR_KeySymbol::KEY_SYM_A])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
    }
}



/*-------------------------------------
 * Render the Scene
-------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context&    context   = pGraph->mContext;
    AnimUniforms*  pUniforms = static_cast<AnimUniforms*>(context.shader(0).uniforms().get());

    pUniforms->pBones = pGraph->mModelMatrices.data();

    for (SR_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->vpMatrix    = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SR_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];

            if (pGraph->mContext.vao(m.vaoId).num_bindings() < 3)
            {
                continue;
            }

            pUniforms->pTexture = material.pTextures[0];

            // Use the textureless shader if needed
            const size_t shaderId = (size_t)(material.pTextures[0] == nullptr);
            context.draw(m, shaderId, 0);
        }
    }
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> create_context()
{
    int retCode = 0;

    #ifdef LS_ARCH_X86
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _mm_setcsr(_mm_getcsr() | 0x8040); // denormals-are-zero
    #endif


    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;
    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SR_TEST_MAX_THREADS);
    assert(retCode == SR_TEST_MAX_THREADS);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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

    //retCode = meshLoader.load("testdata/rover/testmesh.dae");
    retCode = meshLoader.load("testdata/bob/Bob.md5mesh");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    pGraph->mCurrentTransforms[0].scale(math::vec3{0.5f});
    pGraph->mCurrentTransforms[0].rotate(math::vec3{LS_DEG2RAD(90.f), 0.f, 0.f});

    pGraph->copy_node(0);
    pGraph->mCurrentTransforms[36].scale(math::vec3{2.f});
    pGraph->mCurrentTransforms[36].move(math::vec3{10.f, 0.f, 0.f});

    pGraph->update();

    const SR_VertexShader&&   normVertShader = normal_vert_shader();
    const SR_FragmentShader&& normFragShader = normal_frag_shader();
    const SR_VertexShader&&   texVertShader  = texture_vert_shader();
    const SR_FragmentShader&& texFragShader  = texture_frag_shader();

    // Uniform variables don't always align properly because of the vtable
    std::shared_ptr<AnimUniforms>  pUniforms{(AnimUniforms*)ls::utils::aligned_malloc(sizeof(AnimUniforms)), [](AnimUniforms* p)->void {ls::utils::aligned_free(p);}};

    size_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  pUniforms);
    size_t normShaderId = context.create_shader(normVertShader, normFragShader, pUniforms);

    assert(texShaderId == 0);
    assert(normShaderId == 1);
    (void)texShaderId;
    (void)normShaderId;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
int main()
{
    utils::Pointer<SR_RenderWindow> pWindow{std::move(SR_RenderWindow::create())};
    utils::Pointer<SR_WindowBuffer> pRenderBuf{SR_WindowBuffer::create()};
    utils::Pointer<SR_SceneGraph>   pGraph{std::move(create_context())};
    utils::Pointer<bool[]>          pKeySyms{new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    SR_Context& context = pGraph->mContext;
    SR_AnimationPlayer animPlayer;
    //unsigned currentAnimId = 0;

    setup_animations(*pGraph, animPlayer);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;

    unsigned numThreads = context.num_threads();

    SR_Transform camTrans;
    camTrans.set_type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    //camTrans.extract_transforms(math::look_at(math::vec3{75.f}, math::vec3{0.f, 10.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
    camTrans.extract_transforms(math::look_at(math::vec3{3.f, 0.f, 10.f}, math::vec3{0.f, 4.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
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
        SR_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SR_WinEventType::WIN_EVENT_MOVED)
            {
                std::cout << "Window moved: " << evt.window.x << 'x' << evt.window.y << std::endl;
            }

            if (evt.type == SR_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
                projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
            }

            if (evt.type == SR_WinEventType::WIN_EVENT_KEY_DOWN)
            {
                const SR_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = true;
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SR_KeySymbol keySym = evt.keyboard.keysym;
                pKeySyms[keySym] = false;

                switch (keySym)
                {
                    case SR_KeySymbol::KEY_SYM_SPACE:
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

                    case SR_KeySymbol::KEY_SYM_LEFT:
                        pWindow->set_size(IMAGE_WIDTH / 2, IMAGE_HEIGHT / 2);
                        break;

                    case SR_KeySymbol::KEY_SYM_RIGHT:
                        pWindow->set_size(IMAGE_WIDTH, IMAGE_HEIGHT);
                        break;

                    case SR_KeySymbol::KEY_SYM_UP:
                        numThreads = math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_DOWN:
                        numThreads = math::max(numThreads - 1u, 1u);
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_F1:
                        pWindow->set_mouse_capture(!pWindow->is_mouse_captured());
                        pWindow->set_keys_repeat(!pWindow->keys_repeat()); // no text mode
                        std::cout << "Mouse Capture: " << pWindow->is_mouse_captured() << std::endl;
                        break;

                    case SR_KeySymbol::KEY_SYM_ESCAPE:
                        std::cout << "Escape button pressed. Exiting." << std::endl;
                        shouldQuit = true;
                        break;

                    default:
                        break;
                }
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_CLOSING)
            {
                std::cout << "Window close event caught. Exiting." << std::endl;
                shouldQuit = true;
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_MOUSE_MOVED)
            {
                if (pWindow->is_mouse_captured())
                {
                    SR_MousePosEvent& mouse = evt.mousePos;
                    dx = ((float)mouse.dx / (float)pWindow->width()) * 0.05f;
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
                //std::cout << "MS/F: " << 1000.f*(currSeconds/(float)currFrames) << std::endl;
                std::cout << "FPS: " << ((float)currFrames/currSeconds) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                AnimUniforms* pUniforms = static_cast<AnimUniforms*>(context.shader(1).uniforms().get());
                const math::vec3 camTransPos = camTrans.get_position();
                pUniforms->camPos = math::vec4_cast(camTransPos, 1.f);
            }

            const math::mat4&& vpMatrix = projMatrix * camTrans.get_transform();

            //update_animations(*pGraph, animPlayer, currentAnimId, tickTime);
            pGraph->update();

            context.framebuffer(0).clear_color_buffer(0, SR_ColorRGBA{128, 128, 128, 1});
            context.framebuffer(0).clear_depth_buffer();
            render_scene(pGraph.get(), vpMatrix);

            context.blit(*pRenderBuf, 0);
            pWindow->render(*pRenderBuf);
        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            std::cout << "Window close state encountered. Exiting." << std::endl;
            shouldQuit = true;
        }
    }

    pRenderBuf->terminate();

    return pWindow->destroy();
}
