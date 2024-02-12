
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
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_SceneGraph.hpp"
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
    #define IMAGE_HEIGHT 1024
#endif /* IMAGE_HEIGHT */

#ifndef SL_TEST_MAX_THREADS
    #define SL_TEST_MAX_THREADS (ls::math::max<unsigned>(std::thread::hardware_concurrency(), 2u) - 1u)
#endif /* SL_TEST_MAX_THREADS */

//#include "test_common.hpp"
namespace math = ls::math;
namespace utils = ls::utils;



/*-----------------------------------------------------------------------------
 * Shader data to render volumes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Uniforms to share across shader stages
--------------------------------------*/
struct VolumeUniforms
{
    const SL_Texture* pCubeMap;
    const SL_Texture* pOpacityMap;
    const SL_Texture* pColorMap;
    math::vec4        spacing;
    math::vec4        camPos;
    math::mat4        mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _volume_vert_shader(SL_VertexParam& param)
{
    const VolumeUniforms* pUniforms = param.pUniforms->as<VolumeUniforms>();
    const math::vec4      spacing   = pUniforms->spacing;
    const math::vec3&     vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::mat4&&    modelMat  = math::scale(math::mat4{1.f}, math::vec3_cast(spacing));
    const math::vec4&&    pos       = modelMat * math::vec4_cast(vert, 1.f);

    param.pVaryings[0] = pos;
    param.pVaryings[1] = pos - pUniforms->camPos;

    return pUniforms->mvpMatrix * pos;
}



SL_VertexShader volume_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _volume_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
inline LS_INLINE bool intersect_ray_box(
    const math::vec4& rayPos,
    const math::vec4& rayDir,
    const math::vec4& spacing,
    float& texNear,
    float& texFar) noexcept
{
    const math::vec4&& invR    = math::rcp(rayDir);
    const math::vec4&& tbot    = invR * (-spacing-rayPos);
    const math::vec4&& ttop    = invR * (spacing-rayPos);

    const math::vec4&& tmin    = math::min(ttop, tbot);
    const math::vec2   minXX   = {tmin[0], tmin[0]};
    const math::vec2   minYZ   = {tmin[1], tmin[2]};
    const math::vec2&& nearVal = math::max(minXX, minYZ);
    texNear = math::max(0.f, nearVal[0], nearVal[1]);

    const math::vec4&& tmax    = math::max(ttop, tbot);
    const math::vec2   maxXX   = {tmax[0], tmax[0]};
    const math::vec2   maxYZ   = {tmax[1], tmax[2]};
    const math::vec2&& farVal  = math::min(maxXX, maxYZ);
    texFar = math::min(farVal[0], farVal[1]);

    return texNear <= texFar;
}



template <unsigned step = 64>
inline LS_INLINE math::vec4 calc_normal(const SL_Texture& tex, const math::vec4& p) noexcept
{
    constexpr float stepLen = 1.f / (float)step;
    const math::vec4&& a = p - math::vec4{stepLen, 0.f, 0.f, 0.f};
    const math::vec4&& b = p - math::vec4{0.f, stepLen, 0.f, 0.f};
    const math::vec4&& c = p - math::vec4{0.f, 0.f, stepLen, 0.f};

    return (math::normalize((math::vec4)math::vec4_t<unsigned int>{
        sl_sample_trilinear<SL_ColorRType<unsigned char>, SL_WrapMode::EDGE, SL_TexelOrder::ORDERED>(tex, a[0], a[1], a[2]).r,
        sl_sample_trilinear<SL_ColorRType<unsigned char>, SL_WrapMode::EDGE, SL_TexelOrder::ORDERED>(tex, b[0], b[1], b[2]).r,
        sl_sample_trilinear<SL_ColorRType<unsigned char>, SL_WrapMode::EDGE, SL_TexelOrder::ORDERED>(tex, c[0], c[1], c[2]).r,
        0u
    }) * 2.f) - math::vec4{1.f, 1.f, 1.f, 0.f};
}



inline LS_INLINE bool can_skip_render(const SL_Texture& volumeTex, const math::vec4& ray, math::vec4 rayPos) noexcept
{
    constexpr unsigned numTestSteps = 32;
    const math::vec4&& rayStep = ray * (1.f/(float)numTestSteps);

    for (unsigned i = 0; i < numTestSteps; ++i)
    {
        const SL_ColorR8&& intensity = sl_sample_nearest<SL_ColorR8, SL_WrapMode::EDGE>(volumeTex, rayPos[0], rayPos[1], rayPos[2]);

        if (intensity.r > 16)
        {
            return false;
        }

        rayPos -= rayStep;
    }

    return true;
}



bool _volume_frag_shader(SL_FragmentParam& fragParam)
{
    constexpr unsigned numSteps = 256;
    constexpr float step = 1.f / (float)numSteps;

    const VolumeUniforms* pUniforms = fragParam.pUniforms->as<VolumeUniforms>();
    const math::vec4&     spacing   = pUniforms->spacing;
    const math::vec4&&    scaling   = math::rcp(spacing);
    const SL_Texture&     volumeTex = *pUniforms->pCubeMap;
    const SL_Texture&     alphaTex  = *pUniforms->pOpacityMap;
    const SL_Texture&     colorTex  = *pUniforms->pColorMap;
    const math::vec4&     pos       = fragParam.pVaryings[0] * scaling;
    const math::vec4&&    rayDir    = math::normalize(fragParam.pVaryings[1]);
    float                 nearPos;
    float                 farPos;

    const bool intersectInternalBox = intersect_ray_box(pos, rayDir, spacing, nearPos, farPos);
    if (!intersectInternalBox)
    {
        return false;
    }

    const math::vec4&& rayFar    = (pos + rayDir * farPos + 1.f) * 0.5f;
    const math::vec4&& rayNear   = (pos + rayDir * nearPos + 1.f) * 0.5f;
    const math::vec4&& ray       = rayFar - rayNear;
    const math::vec4&& rayStep   = ray * step;
    math::vec4         rayPos    = rayFar;
    math::vec4         dstTexel  = {0.f};
    unsigned           intensity;
    float              srcAlpha;

    // Test pixels with minimal filtering before attempting to do anything
    // more expensive
    if (can_skip_render(volumeTex, ray, rayFar))
    {
        return false;
    }

    (void)colorTex;

    for (unsigned i = 0; (i < numSteps) && (dstTexel[3] < 1.f); ++i)
    {
        intensity = sl_sample_trilinear<SL_ColorR8, SL_WrapMode::EDGE>(volumeTex, rayPos[0], rayPos[1], rayPos[2]).r;

        if (intensity > 16)
        {
            // regular opacity (doesn't take ray steps into account).
            srcAlpha = alphaTex.texel<float>(intensity) * step * 100.f;
            if (srcAlpha > 0.f)
            {
                const math::vec4&& norm      = calc_normal<numSteps>(volumeTex, rayPos);
                const float        luminance = 2.f * math::clamp(math::dot(norm, math::vec4{1.f, 0.f, 1.f, 0.f}), 0.f, 1.f);
                const math::vec3&& volColor  = colorTex.texel<SL_ColorRGBf>(intensity) * luminance;
                const math::vec4&& srcRGBA   = math::vec4_cast(volColor, 1.f) * srcAlpha;

                dstTexel = math::fmadd(dstTexel, math::vec4{1.f}-srcAlpha, srcRGBA);
            }
        }

        rayPos -= rayStep;
    }

    // output composition
    fragParam.pOutputs[0] = math::clamp(dstTexel, math::vec4{0.f}, math::vec4{1.f});

    return dstTexel[3] > 0.f;
}



SL_FragmentShader volume_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs = 1;
    shader.blend = SL_BLEND_PREMULTIPLED_ALPHA;
    shader.depthMask = SL_DEPTH_MASK_OFF;
    shader.depthTest = SL_DEPTH_TEST_OFF;
    shader.shader = _volume_frag_shader;

    return shader;
}



/*-------------------------------------
 * Read a volume file
-------------------------------------*/
int read_volume_file(SL_SceneGraph& graph)
{
    const unsigned w = 256;
    const unsigned h = 256;
    const unsigned d = 109;
    const std::string volFile = "testdata/head256x256x109";

    std::ifstream fin{volFile, std::ios::in | std::ios::binary};
    if (!fin.good())
    {
        return -1;
    }

    const size_t texId = graph.mContext.create_texture();
    SL_Texture&  pTex  = graph.mContext.texture(texId);

    if (0 != pTex.init(SL_COLOR_R_8U, w, h, d))
    {
        return -2;
    }

    constexpr size_t numTexels = w*h*d;
    constexpr size_t numBytes = sizeof(char) * numTexels;

    ls::utils::Pointer<char[]> tempBuf{new char[numTexels]};

    fin.read(tempBuf.get(), numBytes);
    fin.close();

    pTex.set_texels(0, 0, 0, (uint16_t)w, (uint16_t)h, (uint16_t)d, tempBuf.get());

    return 0;
}



/*-------------------------------------
 * Load a cube mesh
-------------------------------------*/
int scene_load_cube(SL_SceneGraph& graph, const math::vec3 spacing = math::vec3{1.f, 1.f, 1.f})
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
    
    verts[0]  = math::vec3{-spacing[0], -spacing[1],  spacing[2]};
    verts[1]  = math::vec3{ spacing[0], -spacing[1],  spacing[2]};
    verts[2]  = math::vec3{ spacing[0],  spacing[1],  spacing[2]};
    verts[3]  = math::vec3{ spacing[0],  spacing[1],  spacing[2]};
    verts[4]  = math::vec3{-spacing[0],  spacing[1],  spacing[2]};
    verts[5]  = math::vec3{-spacing[0], -spacing[1],  spacing[2]};
    verts[6]  = math::vec3{ spacing[0], -spacing[1],  spacing[2]};
    verts[7]  = math::vec3{ spacing[0], -spacing[1], -spacing[2]};
    verts[8]  = math::vec3{ spacing[0],  spacing[1], -spacing[2]};
    verts[9]  = math::vec3{ spacing[0],  spacing[1], -spacing[2]};
    verts[10] = math::vec3{ spacing[0],  spacing[1],  spacing[2]};
    verts[11] = math::vec3{ spacing[0], -spacing[1],  spacing[2]};
    verts[12] = math::vec3{-spacing[0],  spacing[1], -spacing[2]};
    verts[13] = math::vec3{ spacing[0],  spacing[1], -spacing[2]};
    verts[14] = math::vec3{ spacing[0], -spacing[1], -spacing[2]};
    verts[15] = math::vec3{ spacing[0], -spacing[1], -spacing[2]};
    verts[16] = math::vec3{-spacing[0], -spacing[1], -spacing[2]};
    verts[17] = math::vec3{-spacing[0],  spacing[1], -spacing[2]};
    verts[18] = math::vec3{-spacing[0], -spacing[1], -spacing[2]};
    verts[19] = math::vec3{-spacing[0], -spacing[1],  spacing[2]};
    verts[20] = math::vec3{-spacing[0],  spacing[1],  spacing[2]};
    verts[21] = math::vec3{-spacing[0],  spacing[1],  spacing[2]};
    verts[22] = math::vec3{-spacing[0],  spacing[1], -spacing[2]};
    verts[23] = math::vec3{-spacing[0], -spacing[1], -spacing[2]};
    verts[24] = math::vec3{-spacing[0], -spacing[1], -spacing[2]};
    verts[25] = math::vec3{ spacing[0], -spacing[1], -spacing[2]};
    verts[26] = math::vec3{ spacing[0], -spacing[1],  spacing[2]};
    verts[27] = math::vec3{ spacing[0], -spacing[1],  spacing[2]};
    verts[28] = math::vec3{-spacing[0], -spacing[1],  spacing[2]};
    verts[29] = math::vec3{-spacing[0], -spacing[1], -spacing[2]};
    verts[30] = math::vec3{-spacing[0],  spacing[1],  spacing[2]};
    verts[31] = math::vec3{ spacing[0],  spacing[1],  spacing[2]};
    verts[32] = math::vec3{ spacing[0],  spacing[1], -spacing[2]};
    verts[33] = math::vec3{ spacing[0],  spacing[1], -spacing[2]};
    verts[34] = math::vec3{-spacing[0],  spacing[1], -spacing[2]};
    verts[35] = math::vec3{-spacing[0],  spacing[1],  spacing[2]};

    // Create the vertex buffer
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(0, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes = sizeof(verts);

    LS_ASSERT(numVboBytes == (numVerts*stride));

    {
        SL_Mesh mesh;
        mesh.vaoId = vaoId;
        mesh.elementBegin = 0;
        mesh.elementEnd = 36;
        mesh.mode = SL_RenderMode::RENDER_MODE_TRIANGLES;
        mesh.materialId = 0;

        SL_BoundingBox box;
        box.min_point(math::vec3{-spacing});
        box.max_point(math::vec3{spacing});

        graph.insert_mesh(mesh, box);
    }

    {
        constexpr size_t meshId = 0;
        const SL_Transform&& transform{math::mat4{1.f}, SL_TRANSFORM_TYPE_MODEL};
        graph.insert_mesh_node(SCENE_NODE_ROOT_ID, "ct_volume", 1, &meshId, transform);
    }

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the Transfer Functions
-----------------------------------------------------------------------------*/
int create_opacity_map(SL_SceneGraph& graph)
{
    SL_Context&            context    = graph.mContext;
    const size_t           texId      = context.create_texture();
    SL_Texture&            opacityTex = context.texture(texId);

    const uint16_t w = 256;
    const uint16_t h = 1;
    const uint16_t d = 1;

    if (0 != opacityTex.init(SL_COLOR_R_FLOAT, w, h, d))
    {
        std::cerr << "Error: Unable to allocate memory for the opacity transfer functions." << std::endl;
        return 1;
    }

    ls::utils::fast_memset(opacityTex.data(), 0, opacityTex.width()*opacityTex.height()*opacityTex.bpp());

    const auto add_transfer_func = [&opacityTex](const uint16_t begin, const uint16_t end, const float opacity)->void
    {
        for (uint16_t i = begin; i < end; ++i)
        {
            opacityTex.texel<float>(i) = opacity;
        }
    };

    add_transfer_func(0,   15,  0.f);
    add_transfer_func(16,  31,  0.1f); // fat/skin
    add_transfer_func(32,  47,  0.1f); // skin
    add_transfer_func(48,  63,  0.25f); // soft tissue & brain
    add_transfer_func(64,  79,  0.5f); // cartilage & brain crevices
    add_transfer_func(80,  95,  0.2f); // brain crevices & bone
    add_transfer_func(96,  111, 0.05f); // bone
    add_transfer_func(112, 127, 0.05f); // bone
    add_transfer_func(128, 143, 0.05f); // bone
    add_transfer_func(144, 159, 0.05f); // bone
    add_transfer_func(160, 175, 0.05f); // bone
    add_transfer_func(176, 191, 0.05f); // bone
    add_transfer_func(192, 207, 0.05f); // bone
    add_transfer_func(208, 223, 0.05f); // bone
    add_transfer_func(224, 239, 0.05f); // bone
    add_transfer_func(240, 255, 0.05f); // bone

    return 0;
}



int create_color_map(SL_SceneGraph& graph)
{
    SL_Context&            context    = graph.mContext;
    const size_t           texId      = context.create_texture();
    SL_Texture&            colorTex   = context.texture(texId);

    const uint16_t w = 256;
    const uint16_t h = 1;
    const uint16_t d = 1;

    if (0 != colorTex.init(SL_COLOR_RGB_FLOAT, w, h, d))
    {
        std::cerr << "Error: Unable to allocate memory for the color transfer functions." << std::endl;
        return 1;
    }

    ls::utils::fast_memset(colorTex.data(), 0, colorTex.width()*colorTex.height()*colorTex.bpp());

    const auto add_transfer_func = [&colorTex](const uint16_t begin, const uint16_t end, const SL_ColorRGBType<float> color)->void
    {
        for (uint16_t i = begin; i < end; ++i)
        {
            colorTex.texel<SL_ColorRGBf>(i) = color;
        }
    };

    add_transfer_func(16, 47,  SL_ColorRGBType<float>{0.6f,  0.65f, 0.65f});
    add_transfer_func(48, 79,  SL_ColorRGBType<float>{0.2f,  0.2f, 0.6f});
    add_transfer_func(80, 96,  SL_ColorRGBType<float>{0.1f,  0.3f, 0.4f});
    add_transfer_func(96, 255, SL_ColorRGBType<float>{0.6f,  0.6f, 0.6f});

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SL_SceneGraph> init_volume_context()
{
    int retCode = 0;
    utils::Pointer<SL_SceneGraph> pGraph  {new SL_SceneGraph{}};
    SL_Context&                   context = pGraph->mContext;
    size_t                        fboId   = context.create_framebuffer();
    size_t                        texId   = context.create_texture();
    size_t                        depthId = context.create_texture();

    context.num_threads(SL_TEST_MAX_THREADS);

    SL_Texture& tex = context.texture(texId);
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_FLOAT, IMAGE_WIDTH/2, IMAGE_HEIGHT/2, 1);
    LS_ASSERT(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_HALF, IMAGE_WIDTH/2, IMAGE_HEIGHT/2, 1);
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

    retCode = read_volume_file(*pGraph); // creates volume at texture index 2
    LS_ASSERT(retCode == 0);

    retCode = create_opacity_map(*pGraph); // creates volume at texture index 3
    LS_ASSERT(retCode == 0);

    retCode = create_color_map(*pGraph); // creates volume at texture index 4
    LS_ASSERT(retCode == 0);

    retCode = scene_load_cube(*pGraph, math::vec3{1.f, 1.f, 1.f});
    LS_ASSERT(retCode == 0);

    const SL_VertexShader&&   volVertShader = volume_vert_shader();
    const SL_FragmentShader&& volFragShader = volume_frag_shader();

    size_t uboId = context.create_ubo();
    SL_UniformBuffer& ubo = context.ubo(uboId);
    VolumeUniforms* pUniforms = ubo.as<VolumeUniforms>();

    pUniforms->pCubeMap = &context.texture(2);
    pUniforms->pOpacityMap = &context.texture(3);
    pUniforms->pColorMap = &context.texture(4);

    size_t volShaderId = context.create_shader(volVertShader, volFragShader, uboId);
    LS_ASSERT(volShaderId == 0);
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
void render_volume(SL_SceneGraph* pGraph, const SL_Transform& viewMatrix, const math::mat4& vpMatrix)
{
    SL_Context&        context   = pGraph->mContext;
    VolumeUniforms*    pUniforms = context.ubo(0).as<VolumeUniforms>();
    const math::vec3&& camPos    = viewMatrix.absolute_position();
    const math::mat4   modelMat  = math::mat4{1.f};
    pUniforms->spacing           = {1.f, 1.f, 1.f, 1.f};
    pUniforms->camPos            = math::vec4{camPos[0], camPos[1], camPos[2], 0.f};
    pUniforms->mvpMatrix         = vpMatrix * modelMat;

    context.draw(pGraph->mMeshes.back(), 0, 0);
}



/*-------------------------------------
 * Update the camera's position
-------------------------------------*/
void update_cam_position(SL_Transform& camTrans, float tickTime, utils::Pointer<bool[]>& pKeys)
{
    const float camSpeed = 1.f;

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
    ls::utils::Pointer<SL_RenderWindow> pWindow    {std::move(SL_RenderWindow::create())};
    ls::utils::Pointer<SL_Swapchain>    pSwapchain {SL_Swapchain::create()};
    ls::utils::Pointer<SL_SceneGraph>   pGraph     {std::move(init_volume_context())};
    SL_Context&                         context    = pGraph->mContext;
    ls::utils::Pointer<bool[]>          pKeySyms   {new bool[65536]};

    std::fill_n(pKeySyms.get(), 65536 , false);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    float currSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    bool autorotate = true;
    unsigned numThreads = context.num_threads();

    math::mat4 vpMatrix;
    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    camTrans.look_at(math::vec3{-2.f, -1.f, -2.f}, math::vec3{0.f}, math::vec3{0.f, -1.f, 0.f}, false);

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

    if (pSwapchain->init(*pWindow, IMAGE_WIDTH, IMAGE_HEIGHT) != 0 || pWindow->set_title("Volume Rendering Test") != 0)
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
            }
            else if (evt.type == SL_WinEventType::WIN_EVENT_MOUSE_BUTTON_DOWN)
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
            currSeconds += tickTime;

            if (currSeconds >= 0.5f)
            {
                std::cout << "FPS: " << (float)currFrames/currSeconds << std::endl;
                currFrames = 0;
                currSeconds = 0.f;
            }

            update_cam_position(camTrans, tickTime, pKeySyms);

            if (autorotate)
            {
                camTrans.rotate(math::vec3{tickTime*0.5f, 0.f, 0.f});
            }

            if (camTrans.is_dirty())
            {
                camTrans.apply_transform();

                constexpr float    viewAngle  = math::radians(60.f);
                const math::mat4&& projMatrix = math::infinite_perspective(viewAngle, (float)pWindow->width() / (float)pWindow->height(), 0.001f);
                //const math::mat4&& projMatrix = math::ortho(-4.f, 4.f, -3.f, 3.f, 0.01f, 100.f);

                vpMatrix = projMatrix * camTrans.transform();
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);

            render_volume(pGraph.get(), camTrans, vpMatrix);

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

    return pWindow->destroy();
}

