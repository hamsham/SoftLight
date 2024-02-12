
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
#include "softlight/SL_ColorHSX.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Octree.hpp"
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
    #define IMAGE_WIDTH 1280
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



template class SL_Octree<int, 16>;
typedef SL_Octree<int, 16> OctreeType;

template class SL_OctreeNode<int>;
typedef SL_OctreeNode<int> OctreeNodeType;



/*-----------------------------------------------------------------------------
 * Shader data to render volumes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Uniforms to share across shader stages
--------------------------------------*/
struct OctreeUniforms
{
    math::vec3 origin;
    float      radius;
    math::vec4 color;
    math::mat4 mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _box_vert_shader(SL_VertexParam& param)
{
    const OctreeUniforms* pUniforms = param.pUniforms->as<OctreeUniforms>();
    const math::vec3&     vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::vec4      worldPos  = math::vec4_cast(pUniforms->origin, 1.f) + math::vec4_cast(vert, 0.f) * pUniforms->radius;

    return pUniforms->mvpMatrix * worldPos;
}



SL_VertexShader box_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode = SL_CULL_BACK_FACE;
    //shader.cullMode = SL_CULL_OFF;
    shader.shader = _box_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _box_frag_shader(SL_FragmentParam& fragParam)
{
    fragParam.pOutputs[0] = fragParam.pUniforms->as<OctreeUniforms>()->color;

    return true;
}



SL_FragmentShader box_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs = 1;
    shader.blend = SL_BLEND_ALPHA;
    shader.depthMask = SL_DEPTH_MASK_OFF;
    shader.depthTest = SL_DEPTH_TEST_OFF;
    shader.shader = _box_frag_shader;

    return shader;
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SL_SceneGraph& graph)
{
    int retCode = 0;
    SL_Context& context = graph.mContext;
    constexpr unsigned numVerts = 36;
    constexpr size_t stride = sizeof(math::vec3);
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

    math::vec3 verts[numVerts];
    verts[0]  = math::vec3{-1.f, -1.f, 1.f};
    verts[1]  = math::vec3{1.f, -1.f, 1.f};
    verts[2]  = math::vec3{1.f, 1.f, 1.f};
    verts[3]  = math::vec3{1.f, 1.f, 1.f};
    verts[4]  = math::vec3{-1.f, 1.f, 1.f};
    verts[5]  = math::vec3{-1.f, -1.f, 1.f};
    verts[6]  = math::vec3{1.f, -1.f, 1.f};
    verts[7]  = math::vec3{1.f, -1.f, -1.f};
    verts[8]  = math::vec3{1.f, 1.f, -1.f};
    verts[9]  = math::vec3{1.f, 1.f, -1.f};
    verts[10] = math::vec3{1.f, 1.f, 1.f};
    verts[11] = math::vec3{1.f, -1.f, 1.f};
    verts[12] = math::vec3{-1.f, 1.f, -1.f};
    verts[13] = math::vec3{1.f, 1.f, -1.f};
    verts[14] = math::vec3{1.f, -1.f, -1.f};
    verts[15] = math::vec3{1.f, -1.f, -1.f};
    verts[16] = math::vec3{-1.f, -1.f, -1.f};
    verts[17] = math::vec3{-1.f, 1.f, -1.f};
    verts[18] = math::vec3{-1.f, -1.f, -1.f};
    verts[19] = math::vec3{-1.f, -1.f, 1.f};
    verts[20] = math::vec3{-1.f, 1.f, 1.f};
    verts[21] = math::vec3{-1.f, 1.f, 1.f};
    verts[22] = math::vec3{-1.f, 1.f, -1.f};
    verts[23] = math::vec3{-1.f, -1.f, -1.f};
    verts[24] = math::vec3{-1.f, -1.f, -1.f};
    verts[25] = math::vec3{1.f, -1.f, -1.f};
    verts[26] = math::vec3{1.f, -1.f, 1.f};
    verts[27] = math::vec3{1.f, -1.f, 1.f};
    verts[28] = math::vec3{-1.f, -1.f, 1.f};
    verts[29] = math::vec3{-1.f, -1.f, -1.f};
    verts[30] = math::vec3{-1.f, 1.f, 1.f};
    verts[31] = math::vec3{1.f, 1.f, 1.f};
    verts[32] = math::vec3{1.f, 1.f, -1.f};
    verts[33] = math::vec3{1.f, 1.f, -1.f};
    verts[34] = math::vec3{-1.f, 1.f, -1.f};
    verts[35] = math::vec3{-1.f, 1.f, 1.f};

    // Create the vertex buffer
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(0, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    LS_ASSERT(numVboBytes == (numVerts*stride));

    {
        SL_Mesh mesh;
        mesh.vaoId = vaoId;
        mesh.elementBegin = 0;
        mesh.elementEnd = numVerts;
        mesh.mode = SL_RenderMode::RENDER_MODE_TRIANGLES;
        mesh.materialId = (uint32_t)-1;

        SL_BoundingBox box;
        box.min_point(math::vec3{-1.f});
        box.max_point(math::vec3{1.f});

        graph.insert_mesh(mesh, box);
    }

    {
        constexpr size_t meshId = 0;
        const SL_Transform&& transform{math::mat4{1.f}, SL_TRANSFORM_TYPE_MODEL};
        graph.insert_mesh_node(SCENE_NODE_ROOT_ID, "octree_mesh", 1, &meshId, transform);
    }

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

    retCode = scene_load_cube(*pGraph);
    LS_ASSERT(retCode == 0);

    const SL_VertexShader&&   boxVertShader = box_vert_shader();
    const SL_FragmentShader&& boxFragShader = box_frag_shader();

    size_t uboId = context.create_ubo();
    LS_ASSERT(uboId == 0);

    size_t boxShaderId = context.create_shader(boxVertShader, boxFragShader, uboId);
    LS_ASSERT(boxShaderId == 0);
    (void)boxShaderId;

    pGraph->update();

    if (retCode != 0)
    {
        abort();
    }

    return pGraph;
}


OctreeType init_octree()
{
    OctreeType octree{ls::math::vec3{0.f, 0.f, 0.f}, 512.f};

    // insert the world node
    octree.insert(ls::math::vec3{0.f, 0.f, 0.f}, 512.f, 0);

    octree.insert(ls::math::vec3{-25.f,   3.f,   -10.f},   3.f, 1);
    octree.insert(ls::math::vec3{ 242.f,  3.f,    18.f},   2.f, 2);
    octree.insert(ls::math::vec3{-6.f,   -64.f,  -181.f},  3.f, 3);
    octree.insert(ls::math::vec3{ 9.f,    426.f, -10.f},   5.f, 4);
    octree.insert(ls::math::vec3{-100.f, -129.f,  10.f},   3.f, 5);
    octree.insert(ls::math::vec3{-392.f, -37.f,  -210.f},  1.f, 6);
    octree.insert(ls::math::vec3{-52.f,   300.f,  457.f},  3.f, 7);
    octree.insert(ls::math::vec3{-25.f,   4.f,   -9.f},    1.f, 8);

    std::cout
        << "\nTree breadth: " << octree.breadth()
        << "\nTree depth: " << octree.depth()
        << '\n'
        << std::endl;

    return std::move(octree);
}



/*-------------------------------------
 * Render a scene
-------------------------------------*/
void render_octree(SL_SceneGraph* pGraph, const OctreeNodeType& octree, const math::mat4& vpMatrix, size_t renderableDepth)
{
    SL_Context&     context   = pGraph->mContext;
    OctreeUniforms* pUniforms = context.ubo(0).as<OctreeUniforms>();
    const size_t    maxDepth  = octree.depth();

    SL_ColorTypeHSV<float> color;
    color.s = 1.f;
    color.v = 1.f;

    octree.iterate_top_down([&](const OctreeNodeType* pTree, size_t depth)->bool {
        const float percent = (float)(depth+1) / (float)(maxDepth+1);
        color.h = percent;

        pUniforms->origin    = math::vec3_cast(pTree->origin());
        pUniforms->radius    = pTree->radius();
        pUniforms->color     = math::vec4_cast(rgb_cast<float>(color), percent);
        pUniforms->mvpMatrix = vpMatrix;

        context.draw(pGraph->mMeshes.back(), 0, 0);

        return depth < renderableDepth;
    });
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 1000.f;

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
        camTrans.move(math::vec3{camSpeed * tickTime, 0.f, 0.f}, false);
    }

    if (pKeys[SL_KeySymbol::KEY_SYM_a] || pKeys[SL_KeySymbol::KEY_SYM_A])
    {
        camTrans.move(math::vec3{-camSpeed * tickTime, 0.f, 0.f}, false);
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
    OctreeType&&                        octree     = init_octree();
    SL_Context&                         context    = pGraph->mContext;
    ls::utils::Pointer<bool[]>          pKeySyms   {new bool[65536]};

    std::fill_n(pKeySyms.get(), 65536 , false);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    unsigned totalFrames = 0;
    float currSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    bool autorotate = true;
    unsigned numThreads = context.num_threads();

    (void)totalFrames;

    const size_t maxDepth = octree.depth();
    size_t currDepth = maxDepth;

    math::mat4 vpMatrix;
    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    camTrans.look_at(math::vec3{-768.f}, math::vec3{0.f}, math::vec3{0.f, -1.f, 0.f}, false);

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

            if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_BUTTON_DOWN)
            {
                autorotate = false;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_BUTTON_UP)
            {
                autorotate = true;
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_MOVED && !autorotate)
            {
                SL_MousePosEvent& mouse = evt.mousePos;
                dx = (float)mouse.dx / (float)pWindow->width();
                dy = (float)mouse.dy / (float)pWindow->height();
                camTrans.rotate(math::vec3{2.f*dx, -2.f*dy, 0.f});
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_KEY_DOWN)
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

            if (currSeconds >= 0.5f)
            {
                std::cout << "FPS: " << (float)currFrames/currSeconds << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            #if SL_BENCHMARK_SCENE
                if (totalFrames >= 1200)
                {
                    shouldQuit = true;
                }
            #endif

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (autorotate)
            {
                camTrans.rotate(math::vec3{tickTime*0.5f, 0.f, 0.f});
            }

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                constexpr float    viewAngle  = math::radians(45.f);
                const math::mat4&& projMatrix = math::infinite_perspective(viewAngle, (float)pWindow->width() / (float)pWindow->height(), 0.001f);
                vpMatrix = projMatrix * camTrans.transform();
            }

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                context.texture(0).init(context.texture(0).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);
                context.texture(1).init(context.texture(1).type(), (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);

                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.0, 0.0, 0.0, 1.0}, 0.0);

            render_octree(pGraph.get(), octree, vpMatrix, currDepth);

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

    pRenderBuf->terminate();

    return pWindow->destroy();
}

