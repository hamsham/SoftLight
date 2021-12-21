
#include <iostream>
#include <fstream>
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_Atlas.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_FontLoader.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_ImgFilePPM.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Plane.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_TextMeshLoader.hpp"
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
    #define SL_BENCHMARK_SCENE 1
#endif /* SL_BENCHMARK_SCENE */

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
struct TextUniforms
{
    math::mat4        mvpMatrix;
    math::vec4        camPos;
    const SL_Texture* pTexture;
};




/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(SL_VertexParam& param)
{
    typedef Tuple<math::vec3, math::vec2h> Vertex;

    const TextUniforms* pUniforms = param.pUniforms->as<TextUniforms>();
    const Vertex* const v         = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&  vert      = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&  uv        = (math::vec4)math::vec4_cast<math::half>(v->const_element<1>(), math::half{0.f}, math::half{0.f});

    param.pVaryings[0] = uv;

    return pUniforms->mvpMatrix * vert;
}



SL_VertexShader texture_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader(SL_FragmentParam& fragParam)
{
    const TextUniforms* pUniforms = fragParam.pUniforms->as<TextUniforms>();
    const math::vec4&   uv        = fragParam.pVaryings[0];
    const SL_Texture*   pTexture  = pUniforms->pTexture;
    const math::vec4    albedo    = {0.1f, 1.f, 0.25f, 1.f};

    const SL_ColorR8&& pixel8 = sl_sample_bilinear<SL_ColorR8, SL_WrapMode::EDGE>(*pTexture, uv[0], uv[1]);

    fragParam.pOutputs[0] = albedo * (float)pixel8.r * (1.f/255.f);

    return pixel8.r > 128;
}



SL_FragmentShader texture_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_PREMULTIPLED_ALPHA;
    shader.depthTest   = SL_DEPTH_TEST_OFF;
    shader.depthMask   = SL_DEPTH_MASK_OFF;
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
void render_scene(SL_SceneGraph* pGraph, const math::mat4& projection, unsigned w, unsigned h, const SL_Transform& camTrans)
{
    SL_Context&   context = pGraph->mContext;
    TextUniforms* pUniforms = context.ubo(0).as<TextUniforms>();
    SL_Plane      planes[6];

    std::vector<SL_Mesh> instances;
    instances.reserve(pGraph->mMeshes.size());

    const math::mat4 p = math::perspective(math::radians(60.f), (float)w/(float)h, 0.1f, 100.f);
    const math::mat4&& vp = projection * camTrans.transform();

    sl_extract_frustum_planes(p, planes);

    for (SL_SceneNode& n : pGraph->mNodes)
    {
        if (n.type != NODE_TYPE_MESH)
        {
            continue;
        }

        const math::mat4&  modelMat      = pGraph->mModelMatrices[n.nodeId];
        const math::mat4&& mv            = camTrans.transform() * modelMat;
        const size_t       numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];

        pUniforms->mvpMatrix = vp * modelMat;

        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];
        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SL_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SL_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SL_Material&    material   = pGraph->mMaterials[m.materialId];

            if (sl_is_visible(box, mv, planes))
            {
                pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT];
                //context.draw(m, 0, 0);
                instances.push_back(m);
            }
        }
    }

    context.draw_multiple(instances.data(), instances.size(), 0, 0);
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> create_context()
{
    int retCode = 0;
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

    retCode = fbo.attach_color_buffer(0, tex);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_depth_buffer(depth);
    LS_ASSERT(retCode == 0);

    fbo.clear_color_buffers();
    fbo.clear_depth_buffer();

    retCode = fbo.valid();
    LS_ASSERT(retCode == 0);

    const SL_VertexShader&&   texVertShader  = texture_vert_shader();
    const SL_FragmentShader&& texFragShader  = texture_frag_shader();

    size_t uboId = context.create_ubo();
    LS_ASSERT(uboId == 0);

    size_t texShaderId = context.create_shader(texVertShader, texFragShader, uboId);
    LS_ASSERT(texShaderId == 0);
    (void)texShaderId;

    // backbuffer and shader loading is complete. now load the text

    SL_FontLoader fontLoader;
    if (!fontLoader.load_file("testdata/testfont.ttf"))
    {
        std::cerr << "Failed to open the test text font." << std::endl;
        return pGraph;
    }

    SL_Atlas atlas;
    if (!atlas.init(pGraph->mContext, fontLoader))
    {
        std::cerr << "Failed to initialize a font atlas." << std::endl;
        return pGraph;
    }

    std::string buffer;
    if (0)
    {
        buffer = "Hello World!\nI'm a software renderer!";
    }
    else
    {
        std::ifstream fin{"testdata/lorem_ipsum.txt"};
        if (!fin.good())
        {
            std::cerr << "Failed to open the test text file." << std::endl;
            return pGraph;
        }

        size_t orig = 0, end = 0;
        fin.seekg(0, std::ios::end);
        end = fin.tellg();
        fin.seekg(0, std::ios::beg);
        buffer.resize(end - orig);
        fin.read(&buffer[0], buffer.size());
    }


    SL_TextMeshLoader textMeshLoader;
    SL_TextLoadOpts opts = sl_default_text_load_opts();
    opts.packUvs = true;

    retCode = textMeshLoader.load(buffer, atlas, opts, true);
    if (retCode != 0)
    {
        std::cerr << "Failed to load the test text mesh." << std::endl;
        return pGraph;
    }
    retCode = (int)pGraph->import(textMeshLoader.data());
    LS_ASSERT(retCode == 0);

    pGraph->update();

    retCode = sl_img_save_ppm<uint8_t>((uint16_t)atlas.texture()->width(), (uint16_t)atlas.texture()->height(), (const SL_ColorR8*)atlas.texture()->data(), "text_atlas.ppm");
    if (0 == retCode)
    {
        std::cout << "Successfully saved the image text_atlas.ppm" << std::endl;
    }
    else
    {
        std::cerr << "Error exporting the text atlas to a file: " << retCode << std::endl;
    }


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
    utils::Pointer<bool[]>          pKeySyms{new bool[1024]};

    std::fill_n(pKeySyms.get(), 1024, false);

    SL_Context& context = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    camTrans.look_at(math::vec3{30.f, -20.f, -55.f}, math::vec3{30.f, 40.f, 0.f}, math::vec3{0.f, -1.f, 0.f}, true);
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
                if (totalFrames >= 3600)
                {
                    shouldQuit = true;
                }
            #endif

            update_cam_position(camTrans, tickTime, pKeySyms);

            TextUniforms* pUniforms = context.ubo(0).as<TextUniforms>();
            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                const math::vec3 camTransPos = camTrans.position();
                pUniforms->camPos = math::vec4_cast(camTransPos, 1.f);
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);

            render_scene(pGraph.get(), projMatrix, pWindow->width(), pWindow->height(), camTrans);
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

    std::cout
        << "Rendered " << totalFrames << " frames in " << totalSeconds << " seconds ("
        << ((double)totalFrames/(double)totalSeconds) << " average fps)." << std::endl;

    return pWindow->destroy();
}
