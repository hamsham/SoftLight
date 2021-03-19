
#include <iostream>
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"

#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/StringUtils.h"
#include "lightsky/utils/Time.hpp"
#include "lightsky/utils/Tuple.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_ImgFile.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_PackedVertex.hpp"
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



/*-----------------------------------------------------------------------------
 * Vertex Structure for each instance
-----------------------------------------------------------------------------*/
struct alignas(alignof(ls::math::vec4)) SphereVert
{
    ls::math::vec3 pos;
    ls::math::vec2_t<ls::math::half> uv;
};

static_assert(sizeof(SphereVert) == sizeof(ls::math::vec4), "Cannot use preferred structure size for sphere instance vertices.");



/*-----------------------------------------------------------------------------
 * Structures to create uniform variables shared across all shader stages.
-----------------------------------------------------------------------------*/
struct InstanceUniforms
{
    const SL_Texture* pTexture;
    utils::UniqueAlignedArray<math::mat4> instanceMatrix;
    math::mat4        modelMatrix;
    math::mat4        vpMatrix;
};




/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(SL_VertexParam& param)
{
    const InstanceUniforms* pUniforms   = param.pUniforms->as<InstanceUniforms>();
    const SphereVert* const v           = param.pVbo->element<SphereVert>(param.pVao->offset(0, param.vertId));
    const math::vec4&&      vert        = math::vec4_cast(v->pos, 1.f);
    const math::vec4&&      uv          = (math::vec4)math::vec4_cast<math::half>(v->uv, math::half{0.f}, math::half{0.f});

    const size_t       instanceId  = param.instanceId;
    const math::mat4&  instanceMat = pUniforms->instanceMatrix[instanceId];
    const math::mat4&& modelMat    = instanceMat * pUniforms->modelMatrix;
    const math::vec4&& pos         = modelMat * vert;

    param.pVaryings[0] = vert;
    param.pVaryings[1] = uv;

    return pUniforms->vpMatrix * pos;
}



SL_VertexShader texture_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader(SL_FragmentParam& fragParam)
{
    #if 0
    const InstanceUniforms* pUniforms = fragParam.pUniforms->as<InstanceUniforms>();
    const math::vec4     uv       = fragParam.pVaryings[1];
    const SL_Texture*    pTexture = pUniforms->pTexture;
    math::vec4           albedo;

    // normalize the texture colors to within (0.f, 1.f)
    {
        const math::vec3_t<uint8_t>&& pixel8 = sl_sample_nearest<math::vec3_t<uint8_t>, SL_WrapMode::EDGE>(*pTexture, uv[0], uv[1]);
        const math::vec4_t<uint8_t>&& pixelF = math::vec4_cast<uint8_t>(pixel8, 255);
        albedo = color_cast<float, uint8_t>(pixelF);
    }

    fragParam.pOutputs[0] = math::min(albedo, math::vec4{1.f});

    #else

    const math::vec4 norm = math::vec4{1.f, 0.f, 0.f, 0.f};
    const math::vec4 pos = math::normalize(fragParam.pVaryings[0]);

    //const float rgb = math::step(0.f, math::dot(pos, norm));
    const float rgb = math::clamp(math::dot(pos, norm), 0.f, 1.f);

    fragParam.pOutputs[0] = math::vec4{rgb, rgb, rgb, 1.f};

    #endif

    return true;
}



SL_FragmentShader texture_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SL_BLEND_OFF;
    shader.depthTest   = SL_DEPTH_TEST_GREATER_EQUAL;
    shader.depthMask   = SL_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader;

    return shader;
}



/*-------------------------------------
 * Read the demo texture
-------------------------------------*/
int scene_load_texture(SL_SceneGraph& graph, const std::string& texPath)
{
    SL_ImgFile loader;
    size_t w = 0;
    size_t h = 0;

    const size_t texId = graph.mContext.create_texture();
    SL_Texture& tex = graph.mContext.texture(texId);

    if (loader.load(texPath.c_str()) != SL_ImgFile::FILE_LOAD_SUCCESS)
    {
        std::cerr << "Unable to load the demo texture \"" << texPath.c_str() << "\"." << std::endl;
        graph.mContext.destroy_texture(texId);
        return -1;
    }

    w = loader.width();
    h = loader.height();

    tex.init(SL_ColorDataType::SL_COLOR_RGB_8U, (uint16_t)w, (uint16_t)h, 6);
    tex.set_texels(0, 0, 0, (uint16_t)w, (uint16_t)h, 1, loader.data());

    SL_Material mat;
    sl_reset(mat);
    mat.pTextures[SL_MATERIAL_TEXTURE_AMBIENT] = &tex;
    graph.mMaterials.emplace_back(mat);

    return 0;
}



/*-------------------------------------
 * Load a sphere mesh
-------------------------------------*/
int scene_load_sphere(SL_SceneGraph& graph, unsigned numStacks, unsigned numSectors, float radius = 0.5f)
{
    if (numSectors < 3 || numStacks < 3)
    {
        return -1;
    }

    if (numSectors % 3 || numStacks % 3)
    {
        return -2;
    }

    const float numSectorsf = (float)numSectors;
    const float numStacksf = (float)numStacks;
    const float sectorStep   = LS_TWO_PI / numSectorsf;
    const float stackStep = LS_PI / numStacksf;

    const unsigned numVerts   = (numSectors+1) * (numStacks+1);
    const unsigned stride     = (unsigned)sizeof(SphereVert);
    const unsigned numBytes   = numVerts * stride;
    unsigned       numIndices = (6*numSectors*(numStacks-1)) + (2*numSectors*numStacks) + (2*numSectors*(numStacks-1));
    //numIndices += 3 - (numIndices%3);

    int retCode = 0;
    SL_Context& context = graph.mContext;

    size_t vboId = context.create_vbo();
    SL_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numBytes);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t iboId = context.create_ibo();
    SL_IndexBuffer& ibo = context.ibo(iboId);
    retCode = ibo.init(numIndices, SL_DataType::VERTEX_DATA_INT);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a IBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SL_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    vao.set_index_buffer(iboId);
    retCode = vao.set_num_bindings(2);
    if (retCode != 2)
    {
        std::cerr << "Error while setting the number of VAO bindings: " << retCode << std::endl;
        abort();
    }

    SphereVert* pVerts = (SphereVert*)vbo.data();
    unsigned* pIndices = (unsigned*)ibo.data();

    for(unsigned i = 0, idx = 0; i <= numStacks; ++i)
    {
        const float stackAngle = LS_PI_OVER_2 - (float)i * stackStep;
        const float xy = radius * math::cos(stackAngle);
        const float z = -radius * math::sin(stackAngle);

        for(unsigned j = 0; j <= numSectors; ++j, ++idx)
        {
            const float sectorAngle = (float)j * sectorStep;
            const float x = xy * math::cos(sectorAngle);
            const float y = xy * math::sin(sectorAngle);

            pVerts[idx].pos = math::vec3{y, z, x};
            pVerts[idx].uv = (math::vec2_t<math::half>)math::vec2{(float)j / numSectorsf, (float)i / numStacksf};
        }
    }

    unsigned idx = 0;
    for (unsigned i = 0; i < numStacks; ++i)
    {
        unsigned k1 = i * (numSectors + 1);
        unsigned k2 = k1 + numSectors + 1;

        for (unsigned j = 0; j < numSectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                pIndices[idx++] = k2;
                pIndices[idx++] = k1;
                pIndices[idx++] = k1 + 1;
            }

            if (i != (numStacks - 1))
            {
                pIndices[idx++] = k2;
                pIndices[idx++] = k1 + 1;
                pIndices[idx++] = k2 + 1;
            }

            pIndices[idx++] = k2;
            pIndices[idx++] = k1;
            if (i != 0)
            {
                pIndices[idx++] = k1;
                pIndices[idx++] = k1 + 1;
            }
        }
    }

    //std::cout << "Sphere indices: " << idx << '/' << numIndices << " (" << numIndices%3 << ")." << std::endl;

    // Create the vertex buffer
    vao.set_binding(0, 0, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    vao.set_binding(1, sizeof(math::vec3), stride, SL_Dimension::VERTEX_DIMENSION_2, SL_DataType::VERTEX_DATA_SHORT);

    graph.mMeshes.emplace_back(SL_Mesh());
    SL_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numIndices; // - 3;
    mesh.mode = SL_RenderMode::RENDER_MODE_INDEXED_TRIANGLES;
    mesh.materialId = 0;

    ls::utils::Pointer<size_t[]> meshId{new(std::nothrow) size_t[1]};
    meshId[0] = graph.mNodeMeshes.size();

    SL_BoundingBox box;
    box.min_point(math::vec3{-radius});
    box.max_point(math::vec3{radius});
    graph.mMeshBounds.push_back(box);

    size_t dataId = graph.mMeshes.size() - 1;
    size_t nodeId = graph.mNodes.size();
    graph.mNodes.push_back(SL_SceneNode{SL_SceneNodeType::NODE_TYPE_MESH, nodeId, dataId, SCENE_NODE_ROOT_ID});
    graph.mBaseTransforms.emplace_back(math::mat4{1.f});
    graph.mCurrentTransforms.emplace_back(SL_Transform{math::mat4{1.f}, SL_TRANSFORM_TYPE_MODEL});
    graph.mModelMatrices.emplace_back(math::mat4{1.f});
    graph.mNodeNames.emplace_back("sphere");
    graph.mNodeMeshes.emplace_back(std::move(meshId));
    graph.mNumNodeMeshes.emplace_back(1);

    return 0;
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
void render_scene(SL_SceneGraph* pGraph, const math::mat4& vpMatrix, size_t maxInstances)
{
    SL_Context& context = pGraph->mContext;
    InstanceUniforms* pUniforms = context.ubo(0).as<InstanceUniforms>();

    for (SL_SceneNode& n : pGraph->mNodes)
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
            const size_t       nodeMeshId = meshIds[meshId];
            const SL_Mesh&     m          = pGraph->mMeshes[nodeMeshId];
            const SL_Material& material   = pGraph->mMaterials[m.materialId];

            pUniforms->pTexture = material.pTextures[SL_MATERIAL_TEXTURE_AMBIENT];

            context.draw_instanced(m, maxInstances, 0, 0);
        }
    }

}



/*-----------------------------------------------------------------------------
 * Update the number of instances
-----------------------------------------------------------------------------*/
void update_instance_count(utils::Pointer<SL_SceneGraph>& pGraph, size_t instancesX, size_t instancesY, size_t instancesZ)
{
    SL_Context& context = pGraph->mContext;
    InstanceUniforms* pUniforms = context.ubo(0).as<InstanceUniforms>();

    size_t instanceCount = instancesX * instancesY * instancesZ;
    pUniforms->instanceMatrix = utils::make_unique_aligned_array<math::mat4>(instanceCount);

    for (size_t z = 0; z < instancesZ; ++z)
    {
        for (size_t y = 0; y < instancesY; ++y)
        {
            for (size_t x = 0; x < instancesX; ++x)
            {
                SL_Transform tempTrans;
                tempTrans.position(math::vec3{(float)x, (float)y, (float)z} * 10.f);
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

    utils::Pointer<SL_SceneGraph> pGraph{new SL_SceneGraph{}};
    SL_Context& context = pGraph->mContext;
    size_t fboId   = context.create_framebuffer();
    size_t texId   = context.create_texture();
    size_t depthId = context.create_texture();

    retCode = context.num_threads(SL_TEST_MAX_THREADS);
    assert(retCode == (int)SL_TEST_MAX_THREADS);

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

    retCode = scene_load_texture(*pGraph, "testdata/earth.png");
    assert(retCode == 0);

    retCode = scene_load_sphere(*pGraph, 9, 18, 5.f);
    assert(retCode == 0);

    pGraph->update();

    const SL_VertexShader&&   texVertShader  = texture_vert_shader();
    const SL_FragmentShader&& texFragShader  = texture_frag_shader();

    size_t uboId = context.create_ubo();
    assert(uboId == 0);
    update_instance_count(pGraph, DEFAULT_INSTANCES_X, DEFAULT_INSTANCES_Y, DEFAULT_INSTANCES_Z);

    size_t texShaderId  = context.create_shader(texVertShader,  texFragShader,  uboId);
    assert(texShaderId == 0);
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
    utils::Pointer<bool[]>          pKeySyms{new bool[1024]};

    std::fill_n(pKeySyms.get(), 1024, false);

    SL_Context& context = pGraph->mContext;

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    utils::Clock<float> timer;
    unsigned currFrames = 0;
    float currSeconds = 0.f;
    float totalSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    unsigned instancesX = DEFAULT_INSTANCES_X;
    unsigned instancesY = DEFAULT_INSTANCES_Y;
    unsigned instancesZ = DEFAULT_INSTANCES_Z;
    unsigned numThreads = context.num_threads();

    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y);
    math::vec3 viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 15.f;
    camTrans.extract_transforms(math::look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
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

                    case SL_KeySymbol::KEY_SYM_1:
                        instancesX = math::max<unsigned>(1, instancesX-1);
                        instancesY = math::max<unsigned>(1, instancesY-1);
                        instancesZ = math::max<unsigned>(1, instancesZ-1);
                        update_instance_count(pGraph, instancesX, instancesY, instancesZ);
                        viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 15.f;
                        camTrans.extract_transforms(math::look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
                        std::cout << "Instance count decreased to (" << instancesX << 'x' << instancesY << 'x' << instancesZ << ") = " << instancesX*instancesY*instancesZ << std::endl;
                        break;

                    case SL_KeySymbol::KEY_SYM_2:
                        instancesX = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesX+1);
                        instancesY = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesY+1);
                        instancesZ = math::min<unsigned>(std::numeric_limits<unsigned>::max(), instancesZ+1);
                        update_instance_count(pGraph, instancesX, instancesY, instancesZ);
                        viewPos = math::vec3{(float)instancesX, (float)instancesY, (float)instancesZ} * 15.f;
                        camTrans.extract_transforms(math::look_at(viewPos, math::vec3{0.f, 0.f, 0.f}, math::vec3{0.f, 1.f, 0.f}));
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
            currSeconds += tickTime;
            totalSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "MS/F: " << utils::to_str(1000.f*(currSeconds/(float)currFrames)) << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            InstanceUniforms* pUniforms = context.ubo(0).as<InstanceUniforms>();
            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();
            }

            for (size_t i = instancesX*instancesY*instancesZ; i--;) {
                pUniforms->instanceMatrix[i] = math::rotate(pUniforms->instanceMatrix[i], math::vec3{0.f, 1.f, 0.f}, tickTime);
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);
            const math::mat4&& vpMatrix = projMatrix * camTrans.transform();

            render_scene(pGraph.get(), vpMatrix, instancesX*instancesY*instancesZ);
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

    context.ubo(0).as<InstanceUniforms>()->instanceMatrix.reset();
    pRenderBuf->terminate();

    return pWindow->destroy();
}
