
#include <array>
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_ImgFile.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_Sampler.hpp"
#include "soft_render/SR_SceneGraph.hpp"
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
    #define SR_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SR_TEST_MAX_THREADS */

//#include "test_common.hpp"
namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Shader data to render volumes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Uniforms to share across shader stages
--------------------------------------*/
struct SkyUniforms
{
    math::mat4 vpMatrix;
    const SR_Texture* pCubeMap;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _sky_vert_shader(SR_VertexParam& param)
{
    const SkyUniforms* pUniforms = param.pUniforms->as<SkyUniforms>();
    const math::vec3   vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::vec3   uv        = *(param.pVbo->element<const math::vec3>(param.pVao->offset(1, param.vertId)));

    // too lazy to flip the cube vertices upside-down
    const math::vec4&& worldPos = pUniforms->vpMatrix * math::vec4_cast(vert, 1.f);
    const math::vec4&& outUv    = math::vec4_cast(uv, 0.f);

    param.pVaryings[0] = outUv;

    return math::vec4_cast(math::vec2_cast(worldPos), math::vec2{worldPos[3]});
}



SR_VertexShader sky_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode = SR_CULL_FRONT_FACE;
    shader.shader = _sky_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _sky_frag_shader(SR_FragmentParam& fragParam)
{
    const SkyUniforms* pUniforms = fragParam.pUniforms->as<SkyUniforms>();
    const math::vec4&  uv        = fragParam.pVaryings[0];
    const SR_Texture*  cubeTex   = pUniforms->pCubeMap;

    const math::vec3_t<uint8_t>&& albedo8 = sr_sample_bilinear<math::vec3_t<uint8_t>, SR_WrapMode::EDGE>(*cubeTex, uv[0], uv[1], uv[2]);

    // output composition
    fragParam.pOutputs[0] = color_cast<float, uint8_t>(math::vec4_cast<uint8_t>(albedo8, 255));

    return true;
}



SR_FragmentShader sky_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs = 1;
    shader.blend = SR_BLEND_OFF;
    shader.depthMask = SR_DEPTH_MASK_OFF;
    shader.depthTest = SR_DEPTH_TEST_ON;
    shader.shader = _sky_frag_shader;

    return shader;
}



/*-------------------------------------
 * Read a volume file
-------------------------------------*/
int read_skybox_files(SR_SceneGraph& graph, const std::array<std::string, 6>& cubeFiles)
{
    SR_ImgFile loader;
    size_t w = 0;
    size_t h = 0;
    size_t bpp = 0;
    int iter = 1;

    const size_t texId = graph.mContext.create_texture();
    SR_Texture& tex = graph.mContext.texture(texId);

    for (size_t i = 0; i < cubeFiles.size(); ++i)
    {
        const std::string& cubeFace = cubeFiles[i];
        
        if (loader.load(cubeFace.c_str()) != SR_ImgFile::FILE_LOAD_SUCCESS)
        {
            std::cerr << "Unable to load the cube map face \"" << cubeFace << "\"." << std::endl;
            graph.mContext.destroy_texture(texId);
            return -1;
        }

        if (iter == 1)
        {
            w = loader.width();
            h = loader.height();
            bpp = loader.bpp();
            tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, w, h, 6);
        }
        else
        {
            if (loader.width() != w
            || loader.height() != h
            || loader.bpp() != bpp)
            {
                std::cerr << "Image " << iter-1 << " contains incorrect image dimensions." << std::endl;
                graph.mContext.destroy_texture(texId);
                return -2;
            }
        }

        const SR_ColorRGB8* inTexels = (const SR_ColorRGB8*)loader.data();
        for (size_t z = (size_t)iter-1; z < (size_t)iter; ++z)
        {
            for (size_t y = 0; y < h; ++y)
            {
                for (size_t x = 0; x < w; ++x)
                {
                    tex.texel<SR_ColorRGB8>((uint16_t)x, (uint16_t)y, (uint16_t)z) = inTexels[x + y * h];
                }
            }
        }

        ++iter;
    }

    sr_img_save_ppm(w, h*6, (const SR_ColorRGB8*)tex.data(), "skybox.ppm");
    std::cout << "Successfully saved the image skybox.ppm" << std::endl;

    return 0;
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SR_SceneGraph& graph)
{
    int retCode = 0;
    SR_Context& context = graph.mContext;
    constexpr unsigned numVerts = 36;
    constexpr size_t stride = sizeof(math::vec3);
    size_t numVboBytes = 0;

    size_t vboId = context.create_vbo();
    SR_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numVerts*stride*2);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SR_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    retCode = vao.set_num_bindings(2);
    if (retCode != 2)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    {
        math::vec3 verts[numVerts];

        // front face
        verts[0] = math::vec3{-1.f, -1.f, 1.f};
        verts[1] = math::vec3{1.f, -1.f, 1.f};
        verts[2] = math::vec3{1.f, 1.f, 1.f};
        verts[3] = math::vec3{1.f, 1.f, 1.f};
        verts[4] = math::vec3{-1.f, 1.f, 1.f};
        verts[5] = math::vec3{-1.f, -1.f, 1.f};

        // right face
        verts[6] = math::vec3{1.f, -1.f, 1.f};
        verts[7] = math::vec3{1.f, -1.f, -1.f};
        verts[8] = math::vec3{1.f, 1.f, -1.f};
        verts[9] = math::vec3{1.f, 1.f, -1.f};
        verts[10] = math::vec3{1.f, 1.f, 1.f};
        verts[11] = math::vec3{1.f, -1.f, 1.f};

        // back face
        verts[12] = math::vec3{-1.f, 1.f, -1.f};
        verts[13] = math::vec3{1.f, 1.f, -1.f};
        verts[14] = math::vec3{1.f, -1.f, -1.f};
        verts[15] = math::vec3{1.f, -1.f, -1.f};
        verts[16] = math::vec3{-1.f, -1.f, -1.f};
        verts[17] = math::vec3{-1.f, 1.f, -1.f};

        // left face
        verts[18] = math::vec3{-1.f, -1.f, -1.f};
        verts[19] = math::vec3{-1.f, -1.f, 1.f};
        verts[20] = math::vec3{-1.f, 1.f, 1.f};
        verts[21] = math::vec3{-1.f, 1.f, 1.f};
        verts[22] = math::vec3{-1.f, 1.f, -1.f};
        verts[23] = math::vec3{-1.f, -1.f, -1.f};

        // bottom face
        verts[24] = math::vec3{-1.f, -1.f, -1.f};
        verts[25] = math::vec3{1.f, -1.f, -1.f};
        verts[26] = math::vec3{1.f, -1.f, 1.f};
        verts[27] = math::vec3{1.f, -1.f, 1.f};
        verts[28] = math::vec3{-1.f, -1.f, 1.f};
        verts[29] = math::vec3{-1.f, -1.f, -1.f};

        // top face
        verts[30] = math::vec3{-1.f, 1.f, 1.f};
        verts[31] = math::vec3{1.f, 1.f, 1.f};
        verts[32] = math::vec3{1.f, 1.f, -1.f};
        verts[33] = math::vec3{1.f, 1.f, -1.f};
        verts[34] = math::vec3{-1.f, 1.f, -1.f};
        verts[35] = math::vec3{-1.f, 1.f, 1.f};

        // Create the vertex buffer
        vbo.assign(verts, numVboBytes, sizeof(verts));
        vao.set_binding(0, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
        numVboBytes += sizeof(verts);
    }
    {
        math::vec3 uvs[numVerts];

        // front face
        uvs[0] = math::vec3{0.f, 0.f, 0.f};
        uvs[1] = math::vec3{1.f, 0.f, 0.f};
        uvs[2] = math::vec3{1.f, 1.f, 0.f};
        uvs[3] = math::vec3{1.f, 1.f, 0.f};
        uvs[4] = math::vec3{0.f, 1.f, 0.f};
        uvs[5] = math::vec3{0.f, 0.f, 0.f};

        // right face
        uvs[6]  = math::vec3{0.f, 0.f, 1.f/6.f};
        uvs[7]  = math::vec3{1.f, 0.f, 1.f/6.f};
        uvs[8]  = math::vec3{1.f, 1.f, 1.f/6.f};
        uvs[9]  = math::vec3{1.f, 1.f, 1.f/6.f};
        uvs[10] = math::vec3{0.f, 1.f, 1.f/6.f};
        uvs[11] = math::vec3{0.f, 0.f, 1.f/6.f};

        // back face
        uvs[12] = math::vec3{0.f, 0.f, 2.f/6.f};
        uvs[13] = math::vec3{1.f, 0.f, 2.f/6.f};
        uvs[14] = math::vec3{1.f, 1.f, 2.f/6.f};
        uvs[15] = math::vec3{1.f, 1.f, 2.f/6.f};
        uvs[16] = math::vec3{0.f, 1.f, 2.f/6.f};
        uvs[17] = math::vec3{0.f, 0.f, 2.f/6.f};

        // left face
        uvs[18] = math::vec3{0.f, 0.f, 3.f/6.f};
        uvs[19] = math::vec3{1.f, 0.f, 3.f/6.f};
        uvs[20] = math::vec3{1.f, 1.f, 3.f/6.f};
        uvs[21] = math::vec3{1.f, 1.f, 3.f/6.f};
        uvs[22] = math::vec3{0.f, 1.f, 3.f/6.f};
        uvs[23] = math::vec3{0.f, 0.f, 3.f/6.f};

        // bottom face
        uvs[24] = math::vec3{0.f, 0.f, 4.f/6.f};
        uvs[25] = math::vec3{1.f, 0.f, 4.f/6.f};
        uvs[26] = math::vec3{1.f, 1.f, 4.f/6.f};
        uvs[27] = math::vec3{1.f, 1.f, 4.f/6.f};
        uvs[28] = math::vec3{0.f, 1.f, 4.f/6.f};
        uvs[29] = math::vec3{0.f, 0.f, 4.f/6.f};

        // top face
        uvs[30] = math::vec3{0.f, 0.f, 5.f/6.f};
        uvs[31] = math::vec3{1.f, 0.f, 5.f/6.f};
        uvs[32] = math::vec3{1.f, 1.f, 5.f/6.f};
        uvs[33] = math::vec3{1.f, 1.f, 5.f/6.f};
        uvs[34] = math::vec3{0.f, 1.f, 5.f/6.f};
        uvs[35] = math::vec3{0.f, 0.f, 5.f/6.f};
        vbo.assign(uvs, numVboBytes, sizeof(uvs));
        vao.set_binding(1, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
        numVboBytes += sizeof(uvs);

        assert(numVboBytes == (numVerts * stride * 2));
    }

    graph.mMeshes.emplace_back(SR_Mesh());
    SR_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numVerts;
    mesh.mode = SR_RenderMode::RENDER_MODE_TRIANGLES;
    mesh.materialId = (uint32_t)-1;

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> init_sky_context()
{
    int retCode = 0;
    utils::Pointer<SR_SceneGraph> pGraph  {new SR_SceneGraph{}};
    SR_Context&                   context = pGraph->mContext;
    size_t                        fboId   = context.create_framebuffer();
    size_t                        texId   = context.create_texture();
    size_t                        depthId = context.create_texture();

    context.num_threads(SR_TEST_MAX_THREADS);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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

    std::array<std::string, 6> files{
        "testdata/skybox/front.jpg",
        "testdata/skybox/right.jpg",
        "testdata/skybox/back.jpg",
        "testdata/skybox/left.jpg",
        "testdata/skybox/bottom.jpg",
        "testdata/skybox/top.jpg",
    };

    retCode = read_skybox_files(*pGraph, files); // creates volume at texture index 2-7
    assert(retCode == 0);

    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);

    const SR_VertexShader&&   skyVertShader = sky_vert_shader();
    const SR_FragmentShader&& skyFragShader = sky_frag_shader();

    size_t uboId = context.create_ubo();
    SR_UniformBuffer& ubo = context.ubo(uboId);
    SkyUniforms* pUniforms = ubo.as<SkyUniforms>();

    pUniforms->pCubeMap = &context.texture(2);

    size_t volShaderId = context.create_shader(skyVertShader, skyFragShader, uboId);
    assert(volShaderId == 0);
    (void)volShaderId;

    pGraph->update();

    if (retCode != 0)
    {
        abort();
    }

    std::cout << "First frame rendered." << std::endl;

    return pGraph;
}



/*-------------------------------------
 * Render a scene
-------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context&  context   = pGraph->mContext;
    SkyUniforms* pUniforms = context.ubo(0).as<SkyUniforms>();
    pUniforms->vpMatrix    = vpMatrix;

    context.draw(pGraph->mMeshes.back(), 0, 0);
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



/*-----------------------------------------------------------------------------
 * main()
-----------------------------------------------------------------------------*/
int main()
{
    ls::utils::Pointer<SR_RenderWindow> pWindow    {std::move(SR_RenderWindow::create())};
    ls::utils::Pointer<SR_WindowBuffer> pRenderBuf {SR_WindowBuffer::create()};
    ls::utils::Pointer<SR_SceneGraph>   pGraph     {std::move(init_sky_context())};
    SR_Context&                         context    = pGraph->mContext;
    ls::utils::Pointer<bool[]>          pKeySyms   {new bool[1024]};

    std::fill_n(pKeySyms.get(), 1024, false);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    float currSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    unsigned numThreads = context.num_threads();

    math::mat4 vpMatrix;
    SR_Transform camTrans;
    camTrans.type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    camTrans.extract_transforms(math::look_at(math::vec3{0.f, 0.f, 0.f}, math::vec3{1.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);
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

    if (pRenderBuf->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Volume Rendering Test") != 0)
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

                    case SR_KeySymbol::KEY_SYM_UP:
                        numThreads = ls::math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SR_KeySymbol::KEY_SYM_DOWN:
                        numThreads = (numThreads > 1) ? (numThreads-1) : 1;
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
                    dx = ((float)mouse.dx / (float)pWindow->width()) * 0.25f;
                    dy = ((float)mouse.dy / (float)pWindow->height()) * -0.25f;
                    camTrans.rotate(math::vec3{dx, dy, 0.f});
                }
            }
        }
        else
        {
            timer.tick();
            const float tickTime = timer.tick_time().count();

            ++currFrames;
            currSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "FPS: " << (float)currFrames/currSeconds << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                constexpr float    viewAngle  = math::radians(60.f);
                const math::mat4&& projMatrix = math::infinite_perspective(viewAngle, (float)pWindow->width() / (float)pWindow->height(), 0.1f);

                // remove the camera's position
                vpMatrix = projMatrix * math::mat4{math::mat3{camTrans.transform()}};
            }

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                context.texture(0).init(SR_ColorDataType::SR_COLOR_RGBA_FLOAT, (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);
                context.texture(1).init(SR_ColorDataType::SR_COLOR_R_FLOAT,    (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);

                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
            }

            // RENDER!
            {
                pGraph->update();
                context.clear_depth_buffer(0, 0.0);
                render_scene(pGraph.get(), vpMatrix);
                context.blit(*pRenderBuf, 0);
                pWindow->render(*pRenderBuf);
            }
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

