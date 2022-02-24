
// Full-screen quad example using the "Compact YCoCg Frame Buffer" technique.

#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Log.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Color.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_ImgFile.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_WindowBuffer.hpp"
#include "softlight/SL_WindowEvent.hpp"

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
    const SL_Texture* pTexture;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _mrt_vert_shader(SL_VertexParam& param)
{
    const SL_VertexArray*  pVao = param.pVao;
    const SL_VertexBuffer* pVbo = param.pVbo;
    const math::vec3*      vert = pVbo->element<math::vec3>(pVao->offset(0, param.vertId));
    const math::vec3*      uv   = pVbo->element<math::vec3>(pVao->offset(1, param.vertId));

    param.pVaryings[0] = math::vec4_cast(*uv, 0.f);

    return math::vec4_cast(*vert, 1.f);
}



SL_VertexShader mrt_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 1;
    shader.cullMode    = SL_CULL_BACK_FACE;
    shader.shader      = _mrt_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
inline LS_INLINE math::vec4 bumped_normal(const SL_Texture* bumpMap, const math::vec4& uv) noexcept
{
    const float stepX = 1.f / (float)bumpMap->width();
    const float stepY = 1.f / (float)bumpMap->height();

    // sample corners around the current pixel
    math::vec4_t<uint8_t>&& c8  = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0],       uv[1]),       255);
    math::vec4_t<uint8_t>&& n8  = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0],       uv[1]+stepY), 255);
    math::vec4_t<uint8_t>&& e8  = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]-stepX, uv[1]),       255);
    math::vec4_t<uint8_t>&& s8  = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0],       uv[1]-stepY), 255);;
    math::vec4_t<uint8_t>&& w8  = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]+stepX, uv[1]),       255);
    math::vec4_t<uint8_t>&& ne8 = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]-stepX, uv[1]+stepY), 255);
    math::vec4_t<uint8_t>&& se8 = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]-stepX, uv[1]-stepY), 255);
    math::vec4_t<uint8_t>&& sw8 = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]+stepX, uv[1]-stepY), 255);;
    math::vec4_t<uint8_t>&& nw8 = math::vec4_cast<uint8_t>(sl_sample_bilinear<SL_ColorRGB8, SL_WrapMode::REPEAT>(*bumpMap, uv[0]+stepX, uv[1]+stepY), 255);

    // gather luminance
    float&& c = math::length(color_cast<float, uint8_t>(c8));
    float&& n = math::length(color_cast<float, uint8_t>(n8));
    float&& e = math::length(color_cast<float, uint8_t>(e8));
    float&& s = math::length(color_cast<float, uint8_t>(s8));
    float&& w = math::length(color_cast<float, uint8_t>(w8));
    float&& ne = math::length(color_cast<float, uint8_t>(ne8));
    float&& se = math::length(color_cast<float, uint8_t>(se8));
    float&& sw = math::length(color_cast<float, uint8_t>(sw8));
    float&& nw = math::length(color_cast<float, uint8_t>(nw8));

    // sobel filter
    const float dX = (ne + 2.f * e + se) - (nw + 2.f * w + sw);
    const float dY = (sw + 2.f * s + se) - (nw + 2.f * n + ne);

    // use the current pixel's luminance to determine influence of surrounding
    // pixels
    const float dZ = c * 2.f - 1.f;

    return math::normalize(math::vec4{dZ, dY, dX, 0.f}) * 0.5f + 0.5f;
}



bool _mrt_frag_shader(SL_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const SL_Texture*       albedo    = pUniforms->pTexture;
    const math::vec4&       uv        = fragParams.pVaryings[0];
    math::vec3_t<uint8_t>&& pixel8    = sl_sample_nearest<SL_ColorRGB8, SL_WrapMode::EDGE>(*albedo, uv[0], uv[1]);
    const math::vec4&&      pixel     = color_cast<float, uint8_t>(math::vec4_cast<uint8_t>(pixel8, 255));

    fragParams.pOutputs[0] = pixel;
    fragParams.pOutputs[1] = bumped_normal(albedo, uv);

    return true;
}



SL_FragmentShader mrt_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 1;
    shader.numOutputs  = 2;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_OFF;
    shader.depthMask   = SL_DEPTH_MASK_OFF;
    shader.shader      = _mrt_frag_shader;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Create a Full-screen quad
-----------------------------------------------------------------------------*/
int load_quad_into_scene(SL_SceneGraph& graph)
{
    SL_Context&        context     = graph.mContext;
    int                retCode     = 0;
    constexpr unsigned numVerts    = 4;
    constexpr size_t   numBindings = 2;
    constexpr size_t   stride      = sizeof(math::vec3);
    size_t             numVboBytes = 0;
    size_t             vaoId       = context.create_vao();
    size_t             vboId       = context.create_vbo();
    size_t             iboId       = context.create_ibo();
    SL_VertexArray&    vao         = context.vao(vaoId);
    SL_VertexBuffer&   vbo         = context.vbo(vboId);
    SL_IndexBuffer&    ibo         = context.ibo(iboId);

    retCode = vbo.init(numVerts*stride*numBindings);
    if (retCode != 0)
    {
        LS_LOG_ERR("Error while creating a VBO: ", retCode);
        abort();
    }

    vao.set_vertex_buffer(vboId);
    retCode = vao.set_num_bindings(numBindings);
    if (retCode != numBindings)
    {
        LS_LOG_ERR("Error while setting the number of VAO bindings: ", retCode);
        abort();
    }

    // Create the vertex buffer
    math::vec3 verts[numVerts];
    verts[0] = math::vec3{-1.f, -1.f, 0.f};
    verts[1] = math::vec3{-1.f,  1.f, 0.f};
    verts[2] = math::vec3{ 1.f,  1.f, 0.f};
    verts[3] = math::vec3{ 1.f, -1.f, 0.f};
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(0, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    // Ensure UVs are only between 0-1.
    for (size_t i = 0; i < numVerts; ++i)
    {
        verts[i] = 0.5f * verts[i] + 0.5f;
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(1, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);
    LS_ASSERT(numVboBytes == (numVerts*stride*numBindings));

    int indices[6] = {
        2, 1, 0,
        0, 3, 2
    };
    ibo.init(6, SL_DataType::VERTEX_DATA_INT, indices);
    vao.set_index_buffer(0);

    SL_Mesh mesh;
    mesh.vaoId        = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd   = 6;
    mesh.mode         = SL_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
    mesh.materialId   = ~0u;

    SL_BoundingBox box;
    box.min_point(math::vec3{-1.f, -1.f, 0.f});
    box.max_point(math::vec3{1.f, 1.f, 0.f});

    size_t subMeshId = graph.insert_mesh(mesh, box);
    graph.insert_mesh_node(SCENE_NODE_ROOT_ID, "FS_Quad", 1, &subMeshId, SL_Transform{});

    return 0;
}



/*-------------------------------------
 * Read a texture file
-------------------------------------*/
int read_input_texture(SL_SceneGraph& graph, const std::string& texFile)
{
    SL_ImgFile loader;
    size_t w = 0;
    size_t h = 0;

    const size_t texId = graph.mContext.create_texture();
    SL_Texture& tex = graph.mContext.texture(texId);

    if (loader.load(texFile.c_str()) != SL_ImgFile::FILE_LOAD_SUCCESS)
    {
        std::cerr << "Unable to load the cube map face \"" << texFile.c_str() << "\"." << std::endl;
        graph.mContext.destroy_texture(texId);
        return -1;
    }

    w = loader.width();
    h = loader.height();
    tex.init(SL_ColorDataType::SL_COLOR_RGB_8U, (uint16_t)w, (uint16_t)h, 1);

    tex.set_texels(0, 0, 0, (uint16_t)w, (uint16_t)h, 1, loader.data());

    SL_ImgFile outImg;
    outImg.load_memory_stream(tex.data(), tex.type(), tex.width(), tex.height()*tex.depth());
    outImg.save("normal_map.png", SL_ImgFileType::IMG_FILE_PNG);

    //sl_img_save_ppm((uint16_t)w, (uint16_t)h*6, (const SL_ColorRGB8*)tex.data(), "skybox.ppm");
    std::cout << "Successfully saved the image normal_map.png" << std::endl;

    return 0;
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

    size_t depthId   = context.create_texture();
    size_t texUvId   = context.create_texture();
    size_t texNormId = context.create_texture();
    size_t fboId     = context.create_framebuffer();

    SL_Texture& texDepth = context.texture(depthId);
    retCode = texDepth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Texture& texUv = context.texture(texUvId);
    retCode = texUv.init(SL_ColorDataType::SL_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Texture& texNorm = context.texture(texNormId);
    retCode = texNorm.init(SL_ColorDataType::SL_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    LS_ASSERT(retCode == 0);

    SL_Framebuffer& fbo = context.framebuffer(fboId);
    retCode = fbo.reserve_color_buffers(2);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_color_buffer(0, texUv);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_color_buffer(1, texNorm);
    LS_ASSERT(retCode == 0);

    retCode = fbo.attach_depth_buffer(texDepth);
    LS_ASSERT(retCode == 0);

    constexpr std::array<size_t, 2> attachIds{0, 1};
    constexpr std::array<SL_ColorRGBAd, 2> colors{
        SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0},
        SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}
    };
    context.clear_framebuffer(0, attachIds, colors, 0.0);

    retCode = fbo.valid();
    LS_ASSERT(retCode == 0);

    retCode = load_quad_into_scene(*pGraph);
    LS_ASSERT(retCode == 0);

    retCode = read_input_texture(*pGraph, "testdata/earth.png");
    LS_ASSERT(retCode == 0);

    retCode = (int)pGraph->import(meshLoader.data());
    LS_ASSERT(retCode == 1);

    // Always make sure the scene graph is updated before rendering
    pGraph->update();

    const SL_VertexShader&&   vertShader = mrt_vert_shader();
    const SL_FragmentShader&& fragShader = mrt_frag_shader();

    size_t uboId = context.create_ubo();

    size_t testShaderId = context.create_shader(vertShader, fragShader, uboId);

    LS_ASSERT(testShaderId == 0);
    (void)testShaderId;

    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void mesh_test_render(SL_SceneGraph* pGraph)
{
    SL_Context&        context   = pGraph->mContext;
    MeshTestUniforms*  pUniforms = context.ubo(0).as<MeshTestUniforms>();

    const SL_SceneNode& n = pGraph->mNodes[0];
    const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
    const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

    for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
    {
        const size_t          nodeMeshId = meshIds[meshId];
        const SL_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
        pUniforms->pTexture = &context.texture(3);

        // NOTE: Always validate your IDs in production
        const size_t shaderId = 0;
        const size_t fboid    = 0;

        context.draw(m, shaderId, fboid);
    }
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
int main()
{
    int retCode = 0;
    (void)retCode;

    utils::Pointer<SL_RenderWindow> pWindow{std::move(SL_RenderWindow::create())};
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

    utils::Pointer<SL_SceneGraph>    pGraph{std::move(mesh_test_create_context())};
    ls::utils::Clock<float>          timer;
    SL_WindowEvent                   evt;
    SL_Context&        context        = pGraph->mContext;
    int                shouldQuit     = 0;
    int                numFrames      = 0;
    int                totalFrames    = 0;
    float              secondsCounter = 0.f;
    float              tickTime       = 0.f;
    unsigned           activeColor    = 1;

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
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
                if (keySym == SL_KeySymbol::KEY_SYM_ESCAPE)
                {
                    LS_LOG_MSG("Escape button pressed. Exiting.");
                    shouldQuit = true;
                }
                else if (keySym == SL_KeySymbol::KEY_SYM_LEFT)
                {
                    activeColor = (activeColor-1 >= 1) ? (activeColor-1) : 2;
                }
                else if (keySym == SL_KeySymbol::KEY_SYM_RIGHT)
                {
                    activeColor = (activeColor+1 <= 2) ? (activeColor+1) : 1;
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

            constexpr std::array<size_t, 2> attachIds{0, 1};
            constexpr std::array<SL_ColorRGBAd, 2> colors{
                SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0},
                SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}
            };
            context.clear_framebuffer(0, attachIds, colors, 0.0);

            mesh_test_render(pGraph.get());

            context.blit(*pRenderBuf, activeColor);
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
