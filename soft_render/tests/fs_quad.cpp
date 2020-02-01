
// Full-screen quad example using the "Compact YCoCg Frame Buffer" technique.

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Log.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Color.hpp"
#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_KeySym.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_RenderWindow.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_WindowBuffer.hpp"

namespace math = ls::math;
namespace utils = ls::utils;



#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 1280
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 720
#endif /* IMAGE_HEIGHT */



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
struct MeshTestUniforms
{
    const SR_Texture* pTexture;
    math::vec4        lightPos;
    SR_ColorRGBAf     lightCol;
    math::mat4        modelMatrix;
    math::mat4        mvpMatrix;
    bool              edgeFilter;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _mesh_test_vert_shader(SR_VertexParam& param)
{
    const MeshTestUniforms* pUniforms = param.pUniforms->as<MeshTestUniforms>();
    const math::vec3&       vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::vec2&       uv        = *(param.pVbo->element<const math::vec2>(param.pVao->offset(1, param.vertId)));
    const math::vec3&       norm      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(2, param.vertId)));

    param.pVaryings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
    param.pVaryings[1] = math::vec4{uv.v[0], uv.v[1], 0.f, 0.f};
    param.pVaryings[2] = math::normalize(pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f});

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader mesh_test_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode    = SR_CULL_BACK_FACE;
    shader.shader      = _mesh_test_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _mesh_test_frag_shader(SR_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const math::vec4        pos       = fragParams.pVaryings[0];
    const math::vec4        uv        = fragParams.pVaryings[1];
    const math::vec4        norm      = math::normalize(fragParams.pVaryings[2]);
    const SR_Texture*       albedo    = pUniforms->pTexture;
    math::vec4              pixel;

    // normalize the texture colors to within (0.f, 1.f)
    math::vec3_t<uint8_t>&& pixel8 = albedo->bilinear<math::vec3_t<uint8_t>>(uv[0], uv[1]);
    math::vec4_t<uint8_t> pixelF{pixel8[0], pixel8[1], pixel8[2], 200};
    pixel = color_cast<float, uint8_t>(pixelF);

    // Light direction calculation
    math::vec4&& lightDir = math::normalize(pUniforms->lightPos - pos);

    // Diffuse light calculation
    const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);
    const math::vec4&& composite = pixel + pUniforms->lightCol * lightAngle;
    const math::vec4&& output = math::clamp(composite, math::vec4{0.f}, math::vec4{1.f});
    const int amOdd = ((fragParams.x & 1) == (fragParams.y & 1));

    #if 1
        constexpr math::mat3 conv{
             0.25f, 0.5f,  0.25f,
             0.5f,  0.f,  -0.5f,
            -0.25f, 0.5f, -0.25f
        };

        math::vec3&& pixelYcocg = conv * math::vec3_cast(output);
        float c = amOdd ? pixelYcocg[2] : pixelYcocg[1];
        fragParams.pOutputs[0] = math::vec4{pixelYcocg[0], c, 0.f, 0.f};
    #else
        SR_ColorYCoCgAf&& pixelYcocg = ycocg_cast<float>(output);
        float c = amOdd ? pixelYcocg.cg : pixelYcocg.co;
        fragParams.pOutputs[0] = math::vec4{pixelYcocg.y, c, 0.f, 0.f};
    #endif

    return true;
}



SR_FragmentShader mesh_test_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _mesh_test_frag_shader;

    return shader;
}



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _ycocg_vert_shader(SR_VertexParam& param)
{
    const math::vec3& vert = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    return math::vec4_cast(vert, 1.f);
}



SR_VertexShader ycocg_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode    = SR_CULL_OFF;
    shader.shader      = _ycocg_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
#if 1
inline LS_INLINE float filter_luminance(float a, const math::vec2& a1, const math::vec2& a2, const math::vec2& a3, const math::vec2& a4) noexcept
{
    constexpr float THRESH = 1.f/255.f;

    math::vec4&& lum = math::abs(math::vec4{a1[0], a2[0], a3[0], a4[0]} - a);
    math::vec4&& w = math::vec4{1.f} - math::step(math::vec4{THRESH}, lum);

    float W = w[0] + w[1] + w[2] + w[3];

    int c = (W == 0.f);
    w[0] = c ? 1.f : w[0];
    W = c ? 1.f : (1.f/W);

    float d = w[0]*a1[1] + w[1]*a2[1] + w[2]*a3[1] + w[3]*a4[1];
    return d * W;
}

//edge-directed reconstruction:
inline LS_INLINE float adjust_chroma(const SR_Texture* tex, uint16_t x, uint16_t y, float lum) noexcept
{
    const uint16_t w  = tex->width();
    const uint16_t h  = tex->height();
    const uint16_t x0 = (x < (w-1)) ? (x+1) : (x-1);
    const uint16_t x1 = (x > 0)     ? (x-1) : (x+1);
    const uint16_t y0 = (y < (h-1)) ? (y+1) : (y-1);
    const uint16_t y1 = (y > 0)     ? (y-1) : (y+1);

    constexpr math::vec2 norm{1.f / 254.f};

    math::vec2&& a0 = norm * (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x0, y); //coords + vec2(1.0 / 1024.0, 0.0));
    math::vec2&& a1 = norm * (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x1, y); //coords - vec2(1.0 / 1024.0, 0.0));
    math::vec2&& a2 = norm * (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x, y0); //coords + vec2(0.0, 1.0 / 512.0));
    math::vec2&& a3 = norm * (math::vec2)tex->texel<math::vec2_t<uint8_t>>(x, y1); //coords - vec2(0.0, 1.0 / 512.0));

    return filter_luminance(lum, a0, a1, a2, a3);
}
#endif

bool _ycocg_frag_shader(SR_FragmentParam& fragParams)
{
    const MeshTestUniforms* pUniforms = fragParams.pUniforms->as<MeshTestUniforms>();
    const SR_Texture*       albedo    = pUniforms->pTexture;
    const uint16_t x0 = fragParams.x;
    const uint16_t y0 = fragParams.y;
    const bool amOdd = ((x0 & 1) == (y0 & 1));
    constexpr float norm255 = 1.f / 254.f;
    const math::vec2&& pixel0 = (math::vec2)albedo->texel<math::vec2_t<uint8_t>>(x0, y0) * norm255;

    float y = pixel0[0];
    float co, cg;

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

    // multiply by a matrix versus directly convert
    #if 1
        constexpr math::mat3 conv{
            1.f,  1.f, -1.f,
            1.f,  0.f,  1.f,
            1.f, -1.f, -1.f
        };

        const math::vec3&& pixelRgb = conv * math::vec3{y, co, cg};
    #else
        const math::vec3&& pixelRgb = rgb_cast<float>(SR_ColorYCoCgf{y, co, cg});
    #endif

    fragParams.pOutputs[0] = math::vec4_cast(pixelRgb, 1.f);

    return true;
}



SR_FragmentShader ycocg_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_OFF;
    shader.depthMask   = SR_DEPTH_MASK_OFF;
    shader.shader      = _ycocg_frag_shader;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Create a Full-screen quad
-----------------------------------------------------------------------------*/
int load_quad_into_scene(SR_SceneGraph& graph)
{
    SR_Context&        context     = graph.mContext;
    int                retCode     = 0;
    constexpr unsigned numVerts    = 4;
    constexpr size_t   numBindings = 2;
    constexpr size_t   stride      = sizeof(math::vec3);
    size_t             numVboBytes = 0;
    size_t             vaoId       = context.create_vao();
    size_t             vboId       = context.create_vbo();
    size_t             iboId       = context.create_ibo();
    SR_VertexArray&    vao         = context.vao(vaoId);
    SR_VertexBuffer&   vbo         = context.vbo(vboId);
    SR_IndexBuffer&    ibo         = context.ibo(iboId);

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
    vao.set_binding(0, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    // Ensure UVs are only between 0-1.
    for (size_t i = 0; i < numVerts; ++i)
    {
        verts[i] = 0.5f + verts[i] * 0.5f;
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(1, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);
    LS_ASSERT(numVboBytes == (numVerts*stride*numBindings));

    int indices[6] = {
        0, 1, 2,
        2, 3, 0
    };
    ibo.init(6, SR_DataType::VERTEX_DATA_INT, indices);
    vao.set_index_buffer(0);

    graph.mNodes.emplace_back(SR_SceneNode{});
    SR_SceneNode& node = graph.mNodes.back();
    node.type = SR_SceneNodeType::NODE_TYPE_MESH;
    node.animListId = SCENE_NODE_ROOT_ID;
    node.dataId = 0;
    node.nodeId = 0;

    graph.mMeshBounds.emplace_back(SR_BoundingBox());
    graph.mMeshBounds.back().compare_and_update(math::vec3{-1.f, -1.f, 0.f});
    graph.mMeshBounds.back().compare_and_update(math::vec3{1.f, 1.f, 0.f});

    SR_Texture& tex = context.texture(1);
    graph.mMaterials.emplace_back(SR_Material{});
    SR_Material& mat = graph.mMaterials.back();
    mat.pTextures[0] = &tex;

    graph.mBaseTransforms.emplace_back(math::mat4{1.f});
    graph.mCurrentTransforms.emplace_back(SR_Transform{});
    graph.mCurrentTransforms.back().extract_transforms(graph.mBaseTransforms.back());
    graph.mModelMatrices.emplace_back(math::mat4{1.f});

    graph.mMeshes.emplace_back(SR_Mesh());
    SR_Mesh& mesh     = graph.mMeshes.back();
    mesh.vaoId        = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd   = 6;
    mesh.mode         = SR_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
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
utils::Pointer<SR_SceneGraph> mesh_test_create_context()
{
    int retCode = 0;

    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;

    size_t depthId = context.create_texture();
    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    // FBO 0, compact YCoCg buffer
    {
        size_t texId = context.create_texture();
        size_t fboId = context.create_framebuffer();
        SR_Texture& tex = context.texture(texId);
        retCode = tex.init(SR_ColorDataType::SR_COLOR_RG_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        assert(retCode == 0);

        SR_Framebuffer& fboYcocg = context.framebuffer(fboId);
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
        SR_Texture& tex = context.texture(texId);
        retCode = tex.init(SR_ColorDataType::SR_COLOR_RGB_8U, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
        assert(retCode == 0);

        SR_Framebuffer& fboRgb = context.framebuffer(fboId);
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

    const SR_VertexShader&&   vertShader0 = mesh_test_vert_shader();
    const SR_FragmentShader&& fragShader0 = mesh_test_frag_shader();
    const SR_VertexShader&&   vertShader1 = ycocg_vert_shader();
    const SR_FragmentShader&& fragShader1 = ycocg_frag_shader();

    size_t uboId = context.create_ubo();
    SR_UniformBuffer& ubo = context.ubo(uboId);
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
void mesh_test_render(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context&       context   = pGraph->mContext;
    MeshTestUniforms* pUniforms = context.ubo(0).as<MeshTestUniforms>();

    for (size_t i = 1; i < pGraph->mNodes.size(); ++i)
    {
        SR_SceneNode& n = pGraph->mNodes[i];

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
            const SR_Mesh&        m          = pGraph->mMeshes[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];
            pUniforms->pTexture = material.pTextures[0];

            // NOTE: Always validate your IDs in production
            const size_t shaderId = 0;
            const size_t fboid    = 0;

            context.draw(m, shaderId, fboid);
        }
    }

    const SR_Mesh&        m          = pGraph->mMeshes[0];
    const SR_Material&    material   = pGraph->mMaterials[0];
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

    utils::Pointer<SR_RenderWindow>  pWindow{std::move(SR_RenderWindow::create())};
    utils::Pointer<SR_WindowBuffer>  pRenderBuf{SR_WindowBuffer::create()};
    utils::Pointer<SR_SceneGraph>    pGraph{std::move(mesh_test_create_context())};
    ls::utils::Clock<float>          timer;
    SR_Transform                     viewMatrix;
    SR_WindowEvent                   evt;
    math::mat4         projMatrix     = math::infinite_perspective(LS_DEG2RAD(80.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);
    SR_Context&        context        = pGraph->mContext;
    SR_Texture&        tex            = context.texture(2);
    SR_Texture&        depth          = context.texture(0);
    int                shouldQuit     = 0;
    int                numFrames      = 0;
    float              secondsCounter = 0.f;
    float              tickTime       = 0.f;

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
    else if (pRenderBuf->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Mesh Test") != 0)
    {
        LS_LOG_ERR("Unable to resize the test window buffer!");
        pWindow->destroy();
        return -2;
    }
    else
    {
        viewMatrix.set_type(SR_TransformType::SR_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
        viewMatrix.extract_transforms(math::look_at(math::vec3{10.f, 30.f, 70.f}, math::vec3{0.f, 20.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
        viewMatrix.apply_transform();

        pWindow->set_keys_repeat(false); // text mode
        pWindow->set_mouse_capture(false);

        timer.start();
    }

    while (!shouldQuit)
    {
        pWindow->update();

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

            if (evt.type == SR_WinEventType::WIN_EVENT_RESIZED)
            {
                std::cout<< "Window resized: " << evt.window.width << 'x' << evt.window.height << std::endl;
                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());

                context.texture(0).init(context.texture(0).type(), pWindow->width(), pWindow->height());
                context.texture(1).init(context.texture(1).type(), pWindow->width(), pWindow->height());
                context.texture(2).init(context.texture(2).type(), pWindow->width(), pWindow->height());

                projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)pWindow->width()/(float)pWindow->height(), 0.01f);
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_KEY_UP)
            {
                const SR_KeySymbol keySym = evt.keyboard.keysym;
                if (keySym == SR_KeySymbol::KEY_SYM_ESCAPE)
                {
                    LS_LOG_MSG("Escape button pressed. Exiting.");
                    shouldQuit = true;
                }
                else if (keySym == SR_KeySymbol::KEY_SYM_1)
                {
                    SR_UniformBuffer& ubo = context.ubo(0);
                    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();
                    pUniforms->edgeFilter = true;
                }
                else if (keySym == SR_KeySymbol::KEY_SYM_2)
                {
                    SR_UniformBuffer& ubo = context.ubo(0);
                    MeshTestUniforms* pUniforms = ubo.as<MeshTestUniforms>();
                    pUniforms->edgeFilter = false;
                }
            }
            else if (evt.type == SR_WinEventType::WIN_EVENT_CLOSING)
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

            context.framebuffer(0).clear_color_buffers();
            context.framebuffer(0).clear_depth_buffer();
            //context.framebuffer(1).clear_color_buffers();
            //context.framebuffer(1).clear_depth_buffer();
            mesh_test_render(pGraph.get(), projMatrix*viewMatrix.get_transform());

            context.blit(*pRenderBuf, 2);
            pWindow->render(*pRenderBuf);

            ++numFrames;

            if (secondsCounter >= 1.f)
            {
                LS_LOG_MSG("FPS: ", utils::to_str((float)numFrames / secondsCounter));
                numFrames = 0;
                secondsCounter = 0.f;
            }

        }

        // All events handled. Now check on the state of the window.
        if (pWindow->state() == WindowStateInfo::WINDOW_CLOSING)
        {
            LS_LOG_MSG("Window close state encountered. Exiting.");
            shouldQuit = true;
        }
    }

    retCode = sr_img_save_ppm(tex.width(), tex.height(), reinterpret_cast<const SR_ColorRGB8*>(tex.data()), "ycocg_test_image.ppm");
    assert(retCode == 0);

    retCode = sr_img_save_ppm(depth.width(), depth.height(), reinterpret_cast<const SR_ColorRf*>(depth.data()), "ycocg_test_depth.ppm");
    assert(retCode == 0);

    pRenderBuf->terminate();
    return pWindow->destroy();
}
