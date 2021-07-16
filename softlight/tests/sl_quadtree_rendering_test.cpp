
#include <fstream>
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Quadtree.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
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

namespace math = ls::math;
namespace utils = ls::utils;

typedef SL_Quadtree<int, 16> QuadtreeType;



/*-----------------------------------------------------------------------------
 * Shader data to render volumes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Uniforms to share across shader stages
--------------------------------------*/
struct QuadtreeUniforms
{
    math::vec2 origin;
    float      radius;
    math::vec4 color;
    math::mat4 mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _box_vert_shader(SL_VertexParam& param)
{
    const QuadtreeUniforms* pUniforms = param.pUniforms->as<QuadtreeUniforms>();
    const math::vec2&     vert      = *(param.pVbo->element<const math::vec2>(param.pVao->offset(0, param.vertId)));
    const math::vec4      worldPos  = math::vec4_cast(pUniforms->origin, 0.f, 1.f) + math::vec4_cast(vert, 0.f, 0.f) * pUniforms->radius;

    return pUniforms->mvpMatrix * worldPos;
}



SL_VertexShader box_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode = SL_CULL_OFF;
    shader.shader = _box_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
SL_FragmentShader box_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs = 1;
    shader.blend = SL_BLEND_ALPHA;
    shader.depthMask = SL_DEPTH_MASK_OFF;
    shader.depthTest = SL_DEPTH_TEST_OFF;
    shader.shader = [](SL_FragmentParam& fragParam) -> bool
    {
        fragParam.pOutputs[0] = fragParam.pUniforms->as<QuadtreeUniforms>()->color;
        return true;
    };

    return shader;
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SL_SceneGraph& graph)
{
    int retCode = 0;
    SL_Context& context = graph.mContext;
    constexpr unsigned numVerts = 6;
    constexpr size_t stride = sizeof(math::vec2);
    size_t numVboBytes = 0;

    size_t vboId = context.create_vbo();
    SL_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numVerts*stride);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SL_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    retCode = vao.set_num_bindings(1);
    if (retCode != 1)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    math::vec2 verts[numVerts];
    verts[0]  = math::vec2{-1.f, -1.f};
    verts[1]  = math::vec2{ 1.f, -1.f};
    verts[2]  = math::vec2{ 1.f,  1.f};
    verts[3]  = math::vec2{ 1.f,  1.f};
    verts[4]  = math::vec2{-1.f,  1.f};
    verts[5]  = math::vec2{-1.f, -1.f};

    // Create the vertex buffer
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(0, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_2, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    assert(numVboBytes == (numVerts*stride));

    graph.mMeshes.emplace_back(SL_Mesh());
    SL_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numVerts;
    mesh.mode = SL_RenderMode::RENDER_MODE_TRIANGLES;
    mesh.materialId = (uint32_t)-1;

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> init_context()
{
    int retCode = 0;
    utils::Pointer<SL_SceneGraph> pGraph  {new SL_SceneGraph{}};
    SL_Context&                   context = pGraph->mContext;
    size_t                        fboId   = context.create_framebuffer();
    size_t                        texId   = context.create_texture();
    size_t                        depthId = context.create_texture();

    context.num_threads(SL_TEST_MAX_THREADS);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SL_Framebuffer& fbo = context.framebuffer(fboId);
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

    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);

    const SL_VertexShader&&   boxVertShader = box_vert_shader();
    const SL_FragmentShader&& boxFragShader = box_frag_shader();

    size_t uboId = context.create_ubo();
    assert(uboId == 0);

    size_t boxShaderId = context.create_shader(boxVertShader, boxFragShader, uboId);
    assert(boxShaderId == 0);
    (void)boxShaderId;

    pGraph->update();

    if (retCode != 0)
    {
        abort();
    }

    return pGraph;
}


QuadtreeType init_quadtree(float randomPx, float randomPy)
{
    QuadtreeType quadtree{ls::math::vec2{0.f, 0.f}, 512.f};

    // insert the world node
    quadtree.insert(ls::math::vec2{0.f, 0.f}, 512.f, 0);

    // insert test nodes
    quadtree.insert(ls::math::vec2{-25.f,   3.f},   3.f, 1);
    quadtree.insert(ls::math::vec2{ 242.f,  3.f},   2.f, 2);
    quadtree.insert(ls::math::vec2{-6.f,   -64.f},  3.f, 3);
    quadtree.insert(ls::math::vec2{ 9.f,    426.f}, 5.f, 4);
    quadtree.insert(ls::math::vec2{-100.f, -129.f}, 3.f, 5);
    quadtree.insert(ls::math::vec2{-392.f, -37.f},  1.f, 6);
    quadtree.insert(ls::math::vec2{-52.f,   300.f}, 3.f, 7);
    quadtree.insert(ls::math::vec2{-25.f,   4.f},   1.f, 8);

    // insert random node
    quadtree.insert(ls::math::vec2{randomPx, randomPy}, 3.f, 9);

    /*
    std::cout
        << "\nTree Node: " << randomPx << ", " << randomPy
        << "\nTree breadth: " << quadtree.breadth()
        << "\nTree depth: " << quadtree.depth()
        << '\n';
    */

    return quadtree;
}



/*-------------------------------------
 * Render a scene
-------------------------------------*/
void render_quadtree(SL_SceneGraph* pGraph, const QuadtreeType& quadtree, const math::mat4& vpMatrix, size_t renderableDepth, float testX, float testY)
{
    SL_Context&     context   = pGraph->mContext;
    QuadtreeUniforms* pUniforms = context.ubo(0).as<QuadtreeUniforms>();
    const size_t    maxDepth  = quadtree.depth();

    SL_ColorTypeHSV<float> color;
    color.s = 1.f;
    color.v = 1.f;

    quadtree.iterate_top_down([&](const QuadtreeType* pTree, size_t depth)->bool {
        const float percent = (float)(depth+1) / (float)(maxDepth+1);
        color.h = 360.f * percent;

        pUniforms->origin    = pTree->origin();
        pUniforms->radius    = pTree->radius();
        pUniforms->color     = math::vec4_cast(rgb_cast<float>(color), percent);
        pUniforms->mvpMatrix = vpMatrix;

        context.draw(pGraph->mMeshes.back(), 0, 0);

        return depth < renderableDepth;
    });

    {
        pUniforms->origin    = math::vec2{testX, testY};
        pUniforms->radius    = 3.f;
        pUniforms->color     = math::vec4{1.f, 1.f, 1.f, 0.5f};
        pUniforms->mvpMatrix = vpMatrix;

        context.draw(pGraph->mMeshes.back(), 0, 0);
    }
}



/*-----------------------------------------------------------------------------
 * main()
-----------------------------------------------------------------------------*/
int main()
{
    ls::utils::Pointer<SL_RenderWindow> pWindow    {SL_RenderWindow::create()};
    ls::utils::Pointer<SL_WindowBuffer> pRenderBuf {SL_WindowBuffer::create()};
    ls::utils::Pointer<SL_SceneGraph>   pGraph     {init_context()};
    QuadtreeType                        quadtree   {init_quadtree(1.f, 1.f)};
    SL_Context&                         context    = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float prevSeconds = 0.f;
    unsigned numThreads = context.num_threads();

    size_t maxDepth = quadtree.depth();
    size_t currDepth = maxDepth;

    math::mat4 vpMatrix;
    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    camTrans.look_at(math::vec3{0.f, 0.f, -1.f}, math::vec3{0.f}, math::vec3{0.f, -1.f, 0.f});
    camTrans.apply_transform();

    {
        const float worldDims = quadtree.radius();
        //constexpr float    viewAngle  = math::radians(45.f);
        //const math::mat4&& projMatrix = math::infinite_perspective(viewAngle, (float)pWindow->width() / (float)pWindow->height(), 0.001f);
        const math::mat4&& projMatrix = math::ortho(-worldDims, worldDims, -worldDims, worldDims);
        vpMatrix = projMatrix * camTrans.transform();
    }

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
    else
    {
        pWindow->set_keys_repeat(true); // non-text mode
        timer.start();
    }

    while (!shouldQuit)
    {
        pWindow->update();
        SL_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SL_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SL_KeySymbol keySym = evt.keyboard.keysym;
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
                        currDepth = currDepth ? (currDepth-1) : 0;
                        std::cout << "Setting renderable depth level to " << currDepth << '/' << maxDepth << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_RIGHT:
                        currDepth = math::min(currDepth+1, maxDepth);
                        std::cout << "Setting renderable depth level to " << currDepth << '/' << maxDepth << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_UP:
                        numThreads = ls::math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_DOWN:
                        numThreads = (numThreads > 1) ? (numThreads-1) : 1;
                        context.num_threads(numThreads);
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
        }
        else
        {
            timer.tick();
            const float tickTime = timer.tick_time().count();

            ++currFrames;
            ++totalFrames;
            currSeconds += tickTime;

            constexpr float loopTime = 30.f;
            constexpr float recipLoopTime = 1.f / loopTime;

            if (currSeconds - prevSeconds >= 1.f)
            {
                std::cout << "FPS: " << (float)currFrames/currSeconds << std::endl;
                prevSeconds = currSeconds;
            }

            if (currSeconds >= loopTime)
            {
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 1000)
                {
                    shouldQuit = true;
                }
            #endif

            const float radomPx = math::cos(ls::math::radians((currSeconds*recipLoopTime) * 360.f)) * 384.f;
            const float radomPy = math::sin(ls::math::radians((currSeconds*recipLoopTime) * 360.f)) * 384.f;
            quadtree = init_quadtree(radomPx, radomPy);
            maxDepth = quadtree.depth();
            currDepth = math::clamp<size_t>(currDepth, 0, maxDepth);

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                context.texture(0).init(SL_ColorDataType::SL_COLOR_RGBA_FLOAT, (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);
                context.texture(1).init(SL_ColorDataType::SL_COLOR_R_FLOAT,    (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);

                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);

            render_quadtree(pGraph.get(), quadtree, vpMatrix, currDepth, radomPx, radomPy);

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

