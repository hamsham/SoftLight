
#include <fstream>
#include <iostream>
#include <memory> // std::move()
#include <thread>

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"
#include "lightsky/math/PerlinNoise.h"

#include "lightsky/utils/Copy.h"
#include "lightsky/utils/Pointer.h"
#include "lightsky/utils/Time.hpp"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Context.hpp"
#include "softlight/SL_ImgFilePPM.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_KeySym.hpp"
#include "softlight/SL_Material.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_RenderWindow.hpp"
#include "softlight/SL_Sampler.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/SL_Transform.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"
#include "softlight/SL_WindowBuffer.hpp"
#include "softlight/SL_WindowEvent.hpp"

#ifndef IMAGE_WIDTH
    #define IMAGE_WIDTH 800
#endif /* IMAGE_WIDTH */

#ifndef IMAGE_HEIGHT
    #define IMAGE_HEIGHT 600
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
    float             viewAngle;
    math::vec2        windowSize;
    const SL_Texture* pCubeMap;
    const SL_Texture* pOpacityMap;
    const SL_Texture* pColorMap;
    math::vec4        spacing;
    math::vec4        camPos;
    math::mat4        viewMatrix;
    math::mat4        mvpMatrix;
};



/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _volume_vert_shader(SL_VertexParam& param)
{
    const VolumeUniforms* pUniforms = param.pUniforms->as<VolumeUniforms>();
    const math::vec3&     vert      = *(param.pVbo->element<const math::vec3>(param.pVao->offset(0, param.vertId)));
    const math::vec4      spacing   = pUniforms->spacing;
    const math::vec4      worldPos  = math::vec4{vert[0], vert[1], vert[2], 1.f};

    return pUniforms->mvpMatrix * (worldPos * spacing);
}



SL_VertexShader volume_vert_shader()
{
    SL_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode = SL_CULL_BACK_FACE;
    shader.shader = _volume_vert_shader;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
inline bool intersect_ray_box(
    const math::vec4& rayPos,
    const math::vec4& rayDir,
    float& texNear,
    float& texFar) noexcept
{
    const math::vec4&& invR    = math::rcp(rayDir);
    const math::vec4&& tbot    = invR * (math::vec4{-1.f}-rayPos);
    const math::vec4&& ttop    = invR * (math::vec4{1.f}-rayPos);

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



inline math::vec4 calc_normal(const SL_Texture* tex, const math::vec4& p, float stepLen) noexcept
{
    const math::vec4&& a = p + math::vec4{stepLen, 0.f, 0.f, 0.f};
    const math::vec4&& b = p + math::vec4{0.f, stepLen, 0.f, 0.f};
    const math::vec4&& c = p + math::vec4{0.f, 0.f, stepLen, 0.f};

    return math::normalize((math::vec4)math::vec4_t<int>{
        sl_sample_nearest<SL_ColorR8, SL_WrapMode::EDGE, SL_TEXELS_ORDERED>(*tex, a[0], a[1], a[2]).r,
        sl_sample_nearest<SL_ColorR8, SL_WrapMode::EDGE, SL_TEXELS_ORDERED>(*tex, b[0], b[1], b[2]).r,
        sl_sample_nearest<SL_ColorR8, SL_WrapMode::EDGE, SL_TEXELS_ORDERED>(*tex, c[0], c[1], c[2]).r,
        0
    });
}



bool _volume_frag_shader(SL_FragmentParam& fragParam)
{
    constexpr float       step      = 1.f / 128.f;

    const VolumeUniforms* pUniforms = fragParam.pUniforms->as<VolumeUniforms>();
    const float           focalLen  = math::rcp<float>(math::tan(pUniforms->viewAngle * 0.5f));
    const math::vec2&&    winDimens = (math::vec2)math::vec2i{fragParam.coord.x, fragParam.coord.y} * math::rcp(pUniforms->windowSize);
    const SL_Texture*     volumeTex = pUniforms->pCubeMap;
    const SL_Texture*     alphaTex  = pUniforms->pOpacityMap;
    const SL_Texture*     colorTex  = pUniforms->pColorMap;
    const math::vec4&     spacing   = pUniforms->spacing;
    const math::vec4&     camPos    = pUniforms->camPos;
    const math::vec4&&    viewDir   = math::vec4{2.f * winDimens[0] - 1.f, 2.f * winDimens[1] - 1.f, -focalLen, 0.f} / spacing;
    math::vec4&&          rayDir    = viewDir * pUniforms->viewMatrix;
    float                 nearPos;
    float                 farPos;

    if (!intersect_ray_box(camPos, rayDir, nearPos, farPos))
    {
        return false;
    }

    math::vec4&& rayStart  = (camPos + rayDir * nearPos + 1.f) * 0.5f;
    math::vec4&& rayStop   = (camPos + rayDir * farPos + 1.f) * 0.5f;
    math::vec4&& ray       = rayStop - rayStart;
    float        rayLen    = math::length(ray);
    math::vec4&& rayStep   = (ray / rayLen) * step;
    math::vec4   rayPos    = rayStart;
    math::vec4   dstTexel  = {0.f};
    unsigned     intensity;
    float        srcAlpha;

    while (dstTexel < 1.f && rayLen > 0.f)
    {
        // Get a pixel with minimal filtering before attempting to do anything more expensive
        intensity = sl_sample_trilinear<SL_ColorR8, SL_WrapMode::EDGE, SL_TEXELS_ORDERED>(*volumeTex, rayPos[0], rayPos[1], rayPos[2]).r;

        // regular opacity (doesn't take ray steps into account).
        srcAlpha = alphaTex->raw_texel<float>(intensity);

        if (intensity >= 17 && srcAlpha > 0.f)
        {
            const math::vec4&& norm = calc_normal(volumeTex, rayPos, step);
            const float diffuse = math::clamp(math::dot(norm, math::vec4{0.f, 0.f, 1.f, 0.f}), 0.f, 1.f);

            // corrected opacity, from:
            // https://github.com/chrislu/schism/blob/master/projects/examples/ex_volume_ray_cast/src/renderer/shader/volume_ray_cast.glslf
            //srcAlpha = 1.f - math::pow(1.f - srcAlpha, math::length(rayPos-rayStart));

            const float    blend     = ((1.f-dstTexel[3]) * srcAlpha);
            SL_ColorRGBf   volColor  = colorTex->raw_texel<SL_ColorRGBf>(intensity); // 0 - 255
            math::vec4&&   composite = math::vec4_cast(volColor, 1.f) * diffuse * blend;

            dstTexel += composite;
        }

        rayLen -= step;
        rayPos += rayStep;
    }

    // output composition
    fragParam.pOutputs[0] = math::clamp(dstTexel, math::vec4{0.f}, math::vec4{1.f});

    return dstTexel[3] > 0.f;
}



SL_FragmentShader volume_frag_shader()
{
    SL_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs = 1;
    shader.blend = SL_BLEND_PREMULTIPLED_ALPHA;
    shader.depthMask = SL_DEPTH_MASK_OFF;
    shader.depthTest = SL_DEPTH_TEST_ON;
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
int scene_load_cube(SL_SceneGraph& graph)
{
    int retCode = 0;
    SL_Context& context = graph.mContext;
    constexpr unsigned numVerts = 36;
    constexpr size_t stride = sizeof(math::vec3);
    size_t numVboBytes = 0;

    size_t vboId = context.create_vbo();
    SL_VertexBuffer& vbo = context.vbo(vboId);
    retCode = vbo.init(numVerts*stride*3);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SL_VertexArray& vao = context.vao(vaoId);
    vao.set_vertex_buffer(vboId);
    retCode = vao.set_num_bindings(3);
    if (retCode != 3)
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

    // Ensure UVs are only between 0-1.
    for (size_t i = 0; i < numVerts; ++i)
    {
        verts[i] = 0.5f + verts[i] * 0.5f;
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(1, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    // Normalizing the vertex positions should allow for smooth shading.
    for (size_t i = 0; i < numVerts; ++i)
    {
        verts[i] = math::normalize(verts[i] - 0.5f);
    }
    vbo.assign(verts, numVboBytes, sizeof(verts));
    vao.set_binding(2, numVboBytes, stride, SL_Dimension::VERTEX_DIMENSION_3, SL_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    assert(numVboBytes == (numVerts*stride*3));

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
 * Create the Transfer Functions
-----------------------------------------------------------------------------*/
int create_opacity_map(SL_SceneGraph& graph, const size_t volumeTexIndex)
{
    SL_Context&            context    = graph.mContext;
    const size_t           texId      = context.create_texture();
    SL_Texture&            opacityTex = context.texture(texId);
    const SL_Texture&      volumeTex  = context.texture(volumeTexIndex);
    const SL_ColorDataType volumeType = volumeTex.type();

    const uint16_t w = (uint16_t)((1 << (sl_bytes_per_color(volumeType)*CHAR_BIT)) - 1);
    const uint16_t h = 1;
    const uint16_t d = 1;

    if (0 != opacityTex.init(SL_COLOR_R_FLOAT, w, h, d))
    {
        std::cerr << "Error: Unable to allocate memory for the opacity transfer functions." << std::endl;
        return 1;
    }

    const auto add_transfer_func = [&opacityTex](const uint16_t begin, const uint16_t end, const float opacity)->void
    {
        for (uint16_t i = begin; i < end; ++i)
        {
            opacityTex.raw_texel<float>(i, 0, 0) = opacity;
        }
    };

    #if 1
    add_transfer_func(0,  17,  0.f);
    add_transfer_func(17, 29,  0.05f);
    add_transfer_func(29, 40,  0.002f);
    add_transfer_func(40, 50,  0.05f);
    add_transfer_func(50, 60,  0.003f);
    add_transfer_func(60, 75,  0.05f);
    add_transfer_func(75, 255, 0.001f);
    #else
    add_transfer_func(0,  17,  0.f);
    add_transfer_func(17, 40,  0.025f);
    add_transfer_func(40, 50,  0.1f);
    add_transfer_func(50, 75,  0.15f);
    add_transfer_func(75, 255, 0.5f);
    #endif

    return 0;
}



int create_color_map(SL_SceneGraph& graph, const size_t volumeTexIndex)
{
    SL_Context&            context    = graph.mContext;
    const size_t           texId      = context.create_texture();
    SL_Texture&            colorTex   = context.texture(texId);
    const SL_Texture&      volumeTex  = context.texture(volumeTexIndex);
    const SL_ColorDataType volumeType = volumeTex.type();

    const uint16_t w = (uint16_t)((1 << (sl_bytes_per_color(volumeType)*CHAR_BIT)) - 1);
    const uint16_t h = 1;
    const uint16_t d = 1;

    if (0 != colorTex.init(SL_COLOR_RGB_FLOAT, w, h, d))
    {
        std::cerr << "Error: Unable to allocate memory for the color transfer functions." << std::endl;
        return 1;
    }

    const auto add_transfer_func = [&colorTex](const uint16_t begin, const uint16_t end, const SL_ColorRGBType<float> color)->void
    {
        for (uint16_t i = begin; i < end; ++i)
        {
            colorTex.raw_texel<SL_ColorRGBf>(i, 0, 0) = color;
        }
    };

    /*
    add_transfer_func(0,  17,  SL_ColorRGBType<float>{0.f,   0.f,  0.f});
    add_transfer_func(17, 40,  SL_ColorRGBType<float>{0.5f,  0.2f, 0.2f});
    add_transfer_func(40, 50,  SL_ColorRGBType<float>{0.4f,  0.3f, 0.1f});
    add_transfer_func(50, 75,  SL_ColorRGBType<float>{1.f,   1.f,  1.f});
    add_transfer_func(75, 255, SL_ColorRGBType<float>{0.6f,  0.6f, 0.6f});
    */
    add_transfer_func(0,  17,  SL_ColorRGBType<float>{0.f,   0.f,  0.f});
    add_transfer_func(17, 40,  SL_ColorRGBType<float>{0.2f,  0.2f, 0.5f});
    add_transfer_func(40, 50,  SL_ColorRGBType<float>{0.1f,  0.3f, 0.4f});
    add_transfer_func(50, 75,  SL_ColorRGBType<float>{1.f,   1.f,  1.f});
    add_transfer_func(75, 255, SL_ColorRGBType<float>{0.6f,  0.6f, 0.6f});

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
    retCode = tex.init(SL_ColorDataType::SL_COLOR_RGBA_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SL_Texture& depth = context.texture(depthId);
    retCode = depth.init(SL_ColorDataType::SL_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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

    retCode = read_volume_file(*pGraph); // creates volume at texture index 2
    assert(retCode == 0);

    retCode = create_opacity_map(*pGraph, 2); // creates volume at texture index 3
    assert(retCode == 0);

    retCode = create_color_map(*pGraph, 2); // creates volume at texture index 4
    assert(retCode == 0);

    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);

    const SL_VertexShader&&   volVertShader = volume_vert_shader();
    const SL_FragmentShader&& volFragShader = volume_frag_shader();

    size_t uboId = context.create_ubo();
    SL_UniformBuffer& ubo = context.ubo(uboId);
    VolumeUniforms* pUniforms = ubo.as<VolumeUniforms>();

    pUniforms->pCubeMap = &context.texture(2);
    pUniforms->pOpacityMap = &context.texture(3);
    pUniforms->pColorMap = &context.texture(4);

    size_t volShaderId = context.create_shader(volVertShader, volFragShader, uboId);
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
void render_volume(SL_SceneGraph* pGraph, const SL_Transform& viewMatrix, const math::mat4& vpMatrix)
{
    SL_Context&        context   = pGraph->mContext;
    VolumeUniforms*    pUniforms = context.ubo(0).as<VolumeUniforms>();
    const math::vec3&& camPos    = viewMatrix.absolute_position();
    const math::mat4   modelMat  = math::mat4{1.f};
    pUniforms->spacing           = {1.f, 2.f, 2.f, 1.f};
    pUniforms->camPos            = math::vec4{camPos[0], camPos[1], camPos[2], 0.f};
    pUniforms->viewMatrix        = viewMatrix.transform();
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
    ls::utils::Pointer<SL_WindowBuffer> pRenderBuf {SL_WindowBuffer::create()};
    ls::utils::Pointer<SL_SceneGraph>   pGraph     {std::move(init_volume_context())};
    SL_Context&                         context    = pGraph->mContext;
    VolumeUniforms*                     pUniforms = context.ubo(0).as<VolumeUniforms>();
    ls::utils::Pointer<bool[]>          pKeySyms   {new bool[256]};

    std::fill_n(pKeySyms.get(), 256, false);

    int shouldQuit = pWindow->init(IMAGE_WIDTH, IMAGE_HEIGHT);

    ls::utils::Clock<float> timer;
    unsigned currFrames = 0;
    float currSeconds = 0.f;
    float dx = 0.f;
    float dy = 0.f;
    unsigned numThreads = context.num_threads();

    math::mat4 vpMatrix;
    SL_Transform camTrans;
    camTrans.type(SL_TransformType::SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y);
    camTrans.extract_transforms(math::look_from(math::vec3{-2.f}, math::vec3{0.f}, math::vec3{0.f, -1.f, 0.f}));

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
        pUniforms->windowSize = math::vec2{(float)pWindow->width(), (float)pWindow->height()};
        pWindow->set_keys_repeat(false); // text mode
        timer.start();
    }

    while (!shouldQuit)
    {
        pWindow->update();
        SL_WindowEvent evt;

        if (pWindow->has_event())
        {
            pWindow->pop_event(&evt);

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

                    case SL_KeySymbol::KEY_SYM_UP:
                        numThreads = ls::math::min(numThreads + 1u, std::thread::hardware_concurrency());
                        context.num_threads(numThreads);
                        break;

                    case SL_KeySymbol::KEY_SYM_DOWN:
                        numThreads = (numThreads > 1) ? (numThreads-1) : 1;
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

                constexpr float    viewAngle  = math::radians(45.f);
                //const float        w          = 0.001f * (float)pWindow->width();
                //const float        h          = 0.001f * (float)pWindow->height();
                //const math::mat4&& projMatrix = math::ortho(-w, w, -h, h, 0.0001f, 0.1f);
                const math::mat4&& projMatrix = math::infinite_perspective(viewAngle, (float)pWindow->width() / (float)pWindow->height(), 0.001f);

                pUniforms->viewAngle = viewAngle;
                vpMatrix = projMatrix * camTrans.transform();
            }

            if (pWindow->width() != pRenderBuf->width() || pWindow->height() != pRenderBuf->height())
            {
                context.texture(0).init(SL_ColorDataType::SL_COLOR_RGBA_FLOAT, (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);
                context.texture(1).init(SL_ColorDataType::SL_COLOR_R_FLOAT,    (uint16_t)pWindow->width(), (uint16_t)pWindow->height(), 1);

                pRenderBuf->terminate();
                pRenderBuf->init(*pWindow, pWindow->width(), pWindow->height());
                pUniforms->windowSize = math::vec2{(float)pWindow->width(), (float)pWindow->height()};
            }

            pGraph->update();

            context.clear_framebuffer(0, 0, SL_ColorRGBAd{0.6, 0.6, 0.6, 1.0}, 0.0);

            render_volume(pGraph.get(), camTrans, vpMatrix);

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

