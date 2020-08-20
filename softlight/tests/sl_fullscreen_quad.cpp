
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
#include "softlight/SL_ImgFilePPM.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneFileLoader.hpp"
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
    #define SL_BENCHMARK_SCENE 1
#endif /* SL_BENCHMARK_SCENE */



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
struct MeshTestUniforms
{
    const SL_Texture* pTexture;
    math::vec4        lightPos;
    SL_ColorRGBAf     lightCol;
    math::mat4        modelMatrix;
    math::mat4        mvpMatrix;
    bool              edgeFilter;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _mesh_test_vert_shader(SL_VertexParam& param)
{
    typedef utils::Tuple<math::vec3, math::vec2, math::vec3> Vertex;
    const MeshTestUniforms* pUniforms = param.pUniforms->as<MeshTestUniforms>();
    const Vertex*           v         = param.pVbo->element<const Vertex>(param.pVao->offset(0, param.vertId));
    const math::vec4&&      vert      = math::vec4_cast(v->const_element<0>(), 1.f);
    const math::vec4&&      uv        = math::vec4_cast(v->const_element<1>(), 0.f, 0.f);
    const math::vec4&&      norm      = math::vec4_cast(v->const_element<2>(), 0.f);

    param.pVaryings[0] = pUniforms->modelMatrix * vert;
    param.pVaryings[1] = uv;
    param.pVaryings[2] = pUniforms->modelMatrix * norm;

    return pUniforms->mvpMatrix * vert;
}



SL_VertexShader mesh_test_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode    = SL_CULL_BACK_FACE;
    shader.shader      = _mesh_test_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _mesh_test_frag_shader(SL_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const math::vec4&       pos       = fragParams.pVaryings[0];
    const math::vec4&       uv        = fragParams.pVaryings[1];
    const math::vec4&       norm      = math::normalize(fragParams.pVaryings[2]);
    const SL_Texture*       albedo    = pUniforms->pTexture;

    // normalize the texture colors to within (0.f, 1.f)
    math::vec3_t<uint8_t>&& pixel8 = sl_sample_trilinear<SL_ColorRGB8, SL_WrapMode::EDGE>(*albedo, uv[0], uv[1]);
    math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
    math::vec4&&            pixel  = color_cast<float, uint8_t>(pixelF);

    // Light direction calculation
    math::vec4&& lightDir = math::normalize(pUniforms->lightPos - pos);

    // Diffuse light calculation
    const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);

    const math::vec4&& composite = pixel + pUniforms->lightCol * lightAngle;
    const math::vec4&& output = math::clamp(composite, math::vec4{0.f}, math::vec4{1.f});

    const int amOdd = ((fragParams.coord.x & 1) == (fragParams.coord.y & 1));

    SL_ColorYCoCgAf&& pixelYcocg  = ycocg_cast<float>(output);
    const float       chrominance = amOdd ? pixelYcocg.cg : pixelYcocg.co;

    fragParams.pOutputs[0] = math::vec4{pixelYcocg.y, chrominance, 0.f, 0.f};

    return true;
}



SL_FragmentShader mesh_test_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_ON;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _mesh_test_frag_shader;

    return shader;
}



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _ycocg_vert_shader(SL_VertexParam& param)
{
    const math::vec3& vert = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    return math::vec4_cast(vert, 1.f);
}



SL_VertexShader ycocg_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode    = SL_CULL_OFF;
    shader.shader      = _ycocg_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
inline LS_INLINE float filter_luminance(float a, const math::vec4& ax, const math::vec4& ay) noexcept
{
    constexpr float THRESH = 10.f/255.f;

    math::vec4&& lum = math::abs(ax - a);
    math::vec4&& w = math::vec4{1.f} - math::step(math::vec4{THRESH}, lum);

    float W = w[0] + w[1] + w[2] + w[3];

    int c = (W == 0.f);
    w[0] = c ? 1.f : w[0];
    W = c ? 1.f : (1.f/W);

    float d = math::dot(w, ay);
    return d * W;
}

//edge-directed reconstruction:
inline LS_INLINE float adjust_chroma(const SL_Texture* tex, uint16_t x, uint16_t y, float lum) noexcept
{
    const uint16_t w  = tex->width();
    const uint16_t h  = tex->height();
    const uint16_t x0 = (x < (w-1)) ? (x+1) : (x-1);
    const uint16_t x1 = (x > 0)     ? (x-1) : (x+1);
    const uint16_t y0 = (y < (h-1)) ? (y+1) : (y-1);
    const uint16_t y1 = (y > 0)     ? (y-1) : (y+1);

    const math::vec2&& a0 = (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x0, y); //coords + vec2(1.0 / 1024.0, 0.0));
    const math::vec2&& a1 = (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x1, y); //coords - vec2(1.0 / 1024.0, 0.0));
    const math::vec2&& a2 = (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x, y0); //coords + vec2(0.0, 1.0 / 512.0));
    const math::vec2&& a3 = (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x, y1); //coords - vec2(0.0, 1.0 / 512.0));

    const math::vec4 norm{1.f / 255.f};
    const math::vec4&& ax = norm * math::vec4{a0[0], a1[0], a2[0], a3[0]};
    const math::vec4&& ay = norm * math::vec4{a0[1], a1[1], a2[1], a3[1]};

    return filter_luminance(lum, ax, ay);
}

bool _ycocg_frag_shader(SL_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const SL_Texture*       albedo    = pUniforms->pTexture;
    const uint16_t          x0        = fragParams.coord.x;
    const uint16_t          y0        = fragParams.coord.y;
    const bool              amOdd     = ((x0 & 1) == (y0 & 1));
    constexpr float         norm255   = 1.f / 255.f;
    const math::vec2&&      pixel0    = (math::vec2)albedo->texel<math::vec2_t<uint8_t>>(x0, y0) * norm255;

    float y = pixel0[0];
    float co, cg;

    // early-out for black pixels
    if (y == 0.f)
    {
        fragParams.pOutputs[0] = math::vec4{0.f, 0.f, 0.f, 1.f};
        return true;
    }

    if (pUniforms->edgeFilter)
    {
        co = pixel0[1];
        cg = adjust_chroma(albedo, x0, y0, y);//+norm255);
    }
    else
    {
        const uint16_t x1 = x0 > 0 ? (x0 - 1) : x0;
        const math::vec2&& pixel1 = (math::vec2)albedo->texel<math::vec2_t<uint8_t>>(x1, y0) * norm255;
        co = pixel0[1];
        cg = pixel1[1];
    }

    if (amOdd)
    {
        const float temp = co;
        co = cg;
        cg = temp;
    }

    fragParams.pOutputs[0] = rgb_cast<float>(SL_ColorYCoCgAf{y, co, cg, 1.f});

    return true;
}



SL_FragmentShader ycocg_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_OFF;
    shader.depthMask   = SL_DEPTH_MASK_OFF;
    shader.shader      = _ycocg_frag_shader;

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
        verts[i] = 0.5f + verts[i] * 0.5f;
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(1, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);
    LS_ASSERT(numVboBytes == (numVerts*stride*numBindings));

    int indices[6] = {
        0, 1, 2,
        2, 3, 0
    };
    ibo.init(6, SL_DataType::VERTEX_DATA_INT, indices);
    vao.set_index_buffer(0);

    graph.mNodes.emplace_back(SL_SceneNode{});
    SL_SceneNode& node = graph.mNodes.back();
    node.type = SL_SceneNodeType::NODE_TYPE_MESH;
    node.animListId = SCENE_NODE_ROOT_ID;
    node.dataId = 0;
    node.nodeId = 0;

    graph.mMeshBounds.emplace_back(SL_BoundingBox());
    graph.mMeshBounds.back().compare_and_update(math::vec3{-1.f, -1.f, 0.f});
    graph.mMeshBounds.back().compare_and_update(math::vec3{1.f, 1.f, 0.f});

    SL_Texture& tex = context.texture(1);
    graph.mMaterials.emplace_back(SL_Material{});
    SL_Material& mat = graph.mMaterials.back();
    mat.pTextures[0] = &tex;

    graph.mBaseTransforms.emplace_back(math::mat4{1.f});
    graph.mCurrentTransforms.emplace_back(SL_Transform{});
    graph.mCurrentTransforms.back().extract_transforms(graph.mBaseTransforms.back());
    graph.mModelMatrices.emplace_back(math::mat4{1.f});

    graph.mMeshes.emplace_back(SL_Mesh());
    SL_Mesh& mesh     = graph.mMeshes.back();
    mesh.vaoId        = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd   = 6;
    mesh.mode         = SL_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
    mesh.materialId   = 0;

    graph.mNodeNames.emplace_back("FS_Quad");
    graph.mNumNodeMeshes.emplace_back(1);
    graph.mNodeMeshes.emplace_back(utils::Pointer<size_t[]>{new size_t[1]});
    graph.mNodeMeshes.back()[0] = 0;

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

    size_t depthId = context.create_texture();
    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_16U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    // FBO 0, compact YCoCg buffer
    {
        size_t texId = context.create_texture();
        size_t fboId = context.create_framebuffer();
        SL_Texture& tex = context.texture(texId);
        retCode = tex.init(SL_ColorDataType::SL_COLOR_RG_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        assert(retCode == 0);

        SL_Framebuffer& fboYcocg = context.framebuffer(fboId);
        retCode = fboYcocg.reserve_color_buffers(1);
        assert(retCode == 0);

        retCode = fboYcocg.attach_color_buffer(0, tex);
        assert(retCode == 0);

        retCode = fboYcocg.attach_depth_buffer(depth);
        assert(retCode == 0);

        fboYcocg.clear_color_buffers();
        fboYcocg.clear_depth_buffer();

        retCode = fboYcocg.valid();
        assert(retCode == 0);
    }

    // FBO 1, decompressed RGB
    {
        size_t texId = context.create_texture();
        size_t fboId = context.create_framebuffer();
        SL_Texture& tex = context.texture(texId);
        retCode = tex.init(SL_ColorDataType::SL_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        assert(retCode == 0);

        SL_Framebuffer& fboRgb = context.framebuffer(fboId);
        retCode = fboRgb.reserve_color_buffers(1);
        assert(retCode == 0);

        retCode = fboRgb.attach_color_buffer(0, tex);
        assert(retCode == 0);

        retCode = fboRgb.attach_depth_buffer(depth);
        assert(retCode == 0);

        fboRgb.clear_color_buffers();
        fboRgb.clear_depth_buffer();

        retCode = fboRgb.valid();
        assert(retCode == 0);
    }

    retCode = load_quad_into_scene(*pGraph);
    assert(retCode == 0);

    //retCode = meshLoader.load("testdata/heart/heart.obj");
    retCode = meshLoader.load("testdata/african_head/african_head.obj");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    // Always make sure the scene graph is updated before rendering
    pGraph->mCurrentTransforms[1].move(math::vec3{0.f, 30.f, 0.f});
    pGraph->mCurrentTransforms[1].scale(math::vec3{5.f});
    pGraph->update();

    const SL_VertexShader&&   vertShader0 = mesh_test_vert_shader();
    const SL_FragmentShader&& fragShader0 = mesh_test_frag_shader();
    const SL_VertexShader&&   vertShader1 = ycocg_vert_shader();
    const SL_FragmentShader&& fragShader1 = ycocg_frag_shader();

    size_t uboId = context.create_ubo();
    SL_UniformBuffer& ubo = context.ubo(uboId);
    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();

    pUniforms->lightPos = math::vec4{20.f, 100.f, 20.f, 0.f};
    pUniforms->lightCol = math::vec4{0.125f, 0.09f, 0.08f, 1.f};
    pUniforms->modelMatrix = math::mat4{1.f};
    pUniforms->mvpMatrix = math::mat4{1.f};
    pUniforms->edgeFilter = true;
    size_t testShaderId0 = context.create_shader(vertShader0,  fragShader0,  uboId);
    size_t testShaderId1 = context.create_shader(vertShader1,  fragShader1,  uboId);

    assert(testShaderId0 == 0);
    (void)testShaderId0;

    assert(testShaderId1 == 1);
    (void)testShaderId1;
    (void)retCode;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void mesh_test_render(SL_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
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

        const math::mat4& modelMat = pGraph->mModelMatrices[n.nodeId];
        const size_t numNodeMeshes = pGraph->mNumNodeMeshes[n.dataId];
        const utils::Pointer<size_t[]>& meshIds = pGraph->mNodeMeshes[n.dataId];

        pUniforms->modelMatrix = modelMat;
        pUniforms->mvpMatrix   = vpMatrix * modelMat;

        for (size_t meshId = 0; meshId < numNodeMeshes; ++meshId)
        {
            const size_t          nodeMeshId = meshIds[meshId];
            const SL_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SL_Material&    material   = pGraph->mMaterials[m.materialId];
            pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT];

            // NOTE: Always validate your IDs in production
            const size_t shaderId = 0;
            const size_t fboid    = 0;

            context.draw(m, shaderId, fboid);
        }
    }

    const SL_Mesh&        m          = pGraph->mMeshes[0];
    const SL_Material&    material   = pGraph->mMaterials[0];
    pUniforms->pTexture = material.pTextures[m.materialId];

    // NOTE: Always validate your IDs in production
    const size_t shaderId = 1;
    const size_t fboid    = 1;

    context.draw(m, shaderId, fboid);
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
    SL_Transform                     viewMatrix;
    SL_WindowEvent                   evt;
    math::mat4         projMatrix     = math::infinite_perspective(LS_DEG2RAD(80.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
    SL_Context&        context        = pGraph->mContext;
    SL_Texture&        tex            = context.texture(2);
    SL_Texture&        depth          = context.texture(0);
    int                shouldQuit     = 0;
    int                numFrames      = 0;
    int                totalFrames    = 0;
    float              secondsCounter = 0.f;
    float              tickTime       = 0.f;

    viewMatrix.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    viewMatrix.extract_transforms(math::look_at(math::vec3{10.f, 30.f, 70.f}, math::vec3{0.f, 20.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
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

                context.texture(0).init(context.texture(0).type(), pWindow->width(), pWindow->height());
                context.texture(1).init(context.texture(1).type(), pWindow->width(), pWindow->height());
                context.texture(2).init(context.texture(2).type(), pWindow->width(), pWindow->height());

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
                else if (keySym == SL_KeySymbol::KEY_SYM_1)
                {
                    SL_UniformBuffer& ubo = context.ubo(0);
                    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();
                    pUniforms->edgeFilter = true;
                }
                else if (keySym == SL_KeySymbol::KEY_SYM_2)
                {
                    SL_UniformBuffer& ubo = context.ubo(0);
                    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();
                    pUniforms->edgeFilter = false;
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

            mesh_test_render(pGraph.get(), projMatrix* viewMatrix.transform());

            context.blit(*pRenderBuf, 2);
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

    retCode = sl_img_save_ppm(tex.width(), tex.height(), reinterpret_cast<const SL_ColorRGB8*>(tex.data()), "ycocg_test_image.ppm");
    assert(retCode == 0);

    retCode = sl_img_save_ppm(depth.width(), depth.height(), reinterpret_cast<const SL_ColorRf*>(depth.data()), "ycocg_test_depth.ppm");
    assert(retCode == 0);

    pRenderBuf->terminate();
    return pWindow->destroy();
}
