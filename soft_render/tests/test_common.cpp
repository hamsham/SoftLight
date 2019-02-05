
#include <iostream>

#include "lightsky/math/vec_utils.h"

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_ImgFilePPM.hpp"
#include "soft_render/SR_Material.hpp"
#include "soft_render/SR_Mesh.hpp"
#include "soft_render/SR_SceneFileLoader.hpp"
#include "soft_render/SR_SceneGraph.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_Transform.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"

#include "test_common.hpp"



#ifndef SR_TEST_MAX_THREADS
    #define SR_TEST_MAX_THREADS 4
#endif /* SR_TEST_MAX_THREADS */

#ifndef SR_TEST_DEBUG_AABBS
    #define SR_TEST_DEBUG_AABBS 0
#endif



/*
        pos.w = 0;
        value = tex3Dlod(VolumeS, pos).r;

        src = (float4)value;
        src.a *= .5f; //reduce the alpha to have a more transparent result

        //Front to back blending
        // dst.rgb = dst.rgb + (1 - dst.a) * src.a * src.rgb
        // dst.a   = dst.a   + (1 - dst.a) * src.a
        src.rgb *= src.a;
        dst = (1.0f - dst.a)*src + dst;

        //break from the loop when alpha gets high enough
        if(dst.a >= .95f)
            break;

        //advance the current position
        pos.xyz += Step;

        //break if the position is greater than <1, 1, 1>
        if(pos.x > 1.0f  pos.y > 1.0f  pos.z > 1.0f)
            break;
 */



/*
    vec3 rayDirection;
    rayDirection.xy = 2.0 * gl_FragCoord.xy / WindowSize - 1.0;
    rayDirection.z = -FocalLength;
    rayDirection = (vec4(rayDirection, 0) * Modelview).xyz;

    Ray eye = Ray( RayOrigin, normalize(rayDirection) );
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    float tnear, tfar;
    IntersectBox(eye, aabb, tnear, tfar);
    if (tnear < 0.0) tnear = 0.0;

    vec3 rayStart = eye.Origin + eye.Dir * tnear;
    vec3 rayStop = eye.Origin + eye.Dir * tfar;

    // Transform from object space to texture coordinate space:
    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    // Perform the ray marching:
    vec3 pos = rayStart;
    vec3 step = normalize(rayStop-rayStart) * stepSize;
    float travel = distance(rayStop, rayStart);
 */



/*-----------------------------------------------------------------------------
 * Shader to display bounding boxes
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _box_vert_shader_impl(const size_t vertId, const SR_VertexArray&, const SR_VertexBuffer&, const SR_UniformBuffer* uniforms, math::vec4*)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec4&   trr       = pUniforms->aabb->get_top_rear_right();
    const math::vec4&   bfl       = pUniforms->aabb->get_bot_front_left();
    const math::vec4    points[]  = {
        {trr[0], bfl[1], bfl[2], 1.f},
        {trr[0], trr[1], bfl[2], 1.f},
        {trr[0], trr[1], trr[2], 1.f},
        {bfl[0], trr[1], trr[2], 1.f},
        {bfl[0], bfl[1], trr[2], 1.f},
        {bfl[0], bfl[1], bfl[2], 1.f},
        {trr[0], bfl[1], trr[2], 1.f},
        {bfl[0], trr[1], bfl[2], 1.f}
    };

    return pUniforms->mvpMatrix * points[vertId % LS_ARRAY_SIZE(points)];
}



SR_VertexShader box_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 0;
    shader.cullMode    = SR_CULL_OFF;
    shader.shader      = _box_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _box_frag_shader_impl(const math::vec4&, const SR_UniformBuffer*, const math::vec4*, math::vec4* outputs)
{
    outputs[0] = SR_ColorRGBAf{1.f, 0.f, 1.f, 1.f};
    return true;
}



SR_FragmentShader box_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 0;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_OFF;
    shader.depthMask   = SR_DEPTH_MASK_OFF;
    shader.shader      = _box_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with a position and normal
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _normal_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);

    const math::vec3& vert = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec3& norm = *vbo.element<const math::vec3>(vao.offset(1, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 0.f};
    varyings[1] = pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f};

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader normal_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 2;
    shader.cullMode    = SR_CULL_BACK_FACE;
    shader.shader      = _normal_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _normal_frag_shader_impl(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    const MeshUniforms* pUniforms     = static_cast<const MeshUniforms*>(uniforms);
    const Light         l             = pUniforms->light;
    const math::vec4    pos           = varyings[0];
    const math::vec4    norm          = math::normalize(varyings[1]);
    math::vec4&         output        = outputs[0];

    math::vec4          lightDir      = l.pos - pos;
    const float         lightDist     = math::length(lightDir);

    lightDir = math::normalize(lightDir);

    const float         lightAngle    = math::max(math::dot(lightDir, norm), 0.f);
    const float         constant      = pUniforms->point.constant;
    const float         linear        = pUniforms->point.linear;
    const float         quadratic     = pUniforms->point.quadratic;
    const float         attenuation   = math::rcp(constant + (linear*lightDist) + (quadratic*lightDist*lightDist));
    const math::vec4&&  diffuse       = l.diffuse * (lightAngle * attenuation);

    const SpotLight     s             = pUniforms->spot;
    const float         theta         = math::dot(lightDir, s.direction);
    const float         spotIntensity = math::clamp((theta - s.outerCutoff) * s.epsilon, 0.f, 1.f);
    const math::vec4&&  specular      = diffuse + (l.specular * (spotIntensity * attenuation));

    output = specular;

    return true;
}



SR_FragmentShader normal_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 2;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _normal_frag_shader_impl;

    return shader;
}



/*-----------------------------------------------------------------------------
 * Shader to display vertices with positions, UVs, normals, and a texture
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Vertex Shader
--------------------------------------*/
math::vec4 _texture_vert_shader_impl(const size_t vertId, const SR_VertexArray& vao, const SR_VertexBuffer& vbo, const SR_UniformBuffer* uniforms, math::vec4* varyings)
{
    const MeshUniforms* pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec3&   vert      = *vbo.element<const math::vec3>(vao.offset(0, vertId));
    const math::vec2&   uv        = *vbo.element<const math::vec2>(vao.offset(1, vertId));
    const math::vec3&   norm      = *vbo.element<const math::vec3>(vao.offset(2, vertId));

    varyings[0] = pUniforms->modelMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
    varyings[1] = math::vec4{uv.v[0], uv.v[1], 0.f, 0.f};
    varyings[2] = math::normalize(pUniforms->modelMatrix * math::vec4{norm[0], norm[1], norm[2], 0.f});

    return pUniforms->mvpMatrix * math::vec4{vert[0], vert[1], vert[2], 1.f};
}



SR_VertexShader texture_vert_shader()
{
    SR_VertexShader shader;
    shader.numVaryings = 3;
    shader.cullMode = SR_CULL_BACK_FACE;
    shader.shader = _texture_vert_shader_impl;

    return shader;
}



/*--------------------------------------
 * Fragment Shader
--------------------------------------*/
bool _texture_frag_shader_spot(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    const math::vec4     pos       = varyings[0];
    const math::vec4     uv        = varyings[1];
    const math::vec4     norm      = varyings[2];
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const SR_Texture*    albedo    = pUniforms->pTexture;
    float                attenuation;
    math::vec4           pixel;
    math::vec4           diffuse;
    math::vec4           specular;

    // normalize the texture colors to within (0.f, 1.f)
    {
        math::vec3_t<uint8_t>&& pixel8 = albedo->nearest<math::vec3_t<uint8_t>>(uv[0], uv[1]);
        math::vec4_t<uint8_t> pixelF{pixel8[0], pixel8[1], pixel8[2], 255};
        pixel = color_cast<float, uint8_t>(pixelF);
    }

    // Light direction calculation
    const Light l         = pUniforms->light;
    math::vec4  lightDir  = l.pos - pos;
    const float lightDist = math::length(lightDir);

    lightDir = math::normalize(lightDir);

    // Diffuse light calculation
    {
        const PointLight p     = pUniforms->point;
        const float lightAngle = math::max(math::dot(lightDir, norm), 0.f);
        const float constant   = p.constant;
        const float linear     = p.linear;
        const float quadratic  = p.quadratic;

        attenuation = math::rcp(constant + (linear * lightDist) + (quadratic * lightDist * lightDist));
        diffuse     = l.diffuse * (lightAngle * attenuation);
    }

    // Specular light calculation
    {
        const SpotLight s         = pUniforms->spot;
        const float theta         = math::dot(lightDir, s.direction);
        const float spotIntensity = math::clamp((theta - s.outerCutoff) * s.epsilon, 0.f, 1.f);

        specular = l.specular * (spotIntensity * attenuation);
    }

    // output composition
    {
        pixel = pixel * (diffuse + specular);
        outputs[0] = math::min(pixel, math::vec4{1.f});
    }

    return true;
}



// Calculate the metallic component of a surface
template <class vec_type = math::vec4>
inline vec_type fresnel_schlick(float cosTheta, const vec_type& surfaceReflection)
{
    return surfaceReflection + (vec_type{1.f} - surfaceReflection) * math::pow(1.f - cosTheta, 5.f);
}



// normal distribution function within a hemisphere
template <class vec_type = math::vec4>
inline float distribution_ggx(const vec_type& norm, const vec_type& hemisphere, float roughness)
{
    float roughSquared = roughness * roughness;
    float roughQuad = roughSquared * roughSquared;
    float nDotH = math::max(math::dot(norm, hemisphere), 0.f);
    float nDotH2 = nDotH * nDotH;

    float distribution = nDotH2 * (roughQuad - 1.f) + 1.f;

    return nDotH2 / (LS_PI * distribution  * distribution );
}



// Determine how a surface's roughness affects how it reflects light
inline float geometry_schlick_ggx(float normDotView, float roughness)
{
    roughness += 1.f;
    roughness = (roughness*roughness) * 0.125f; // 1/8

    float geometry = normDotView * (1.f - roughness) + roughness;
    return normDotView / geometry;
}



// PBR Geometry function for determining how light bounces off a surface
template <class vec_type = math::vec4>
inline float geometry_smith(const vec_type& norm, const vec_type& viewDir, const vec_type& radiance, float roughness)
{
    float normDotView = math::max(math::dot(norm, viewDir), 0.f);
    float normDotRad = math::max(math::dot(norm, radiance), 0.f);

    return geometry_schlick_ggx(normDotView, roughness) * geometry_schlick_ggx(normDotRad, roughness);
}



bool _texture_frag_shader_pbr(const math::vec4&, const SR_UniformBuffer* uniforms, const math::vec4* varyings, math::vec4* outputs)
{
    const MeshUniforms*  pUniforms = static_cast<const MeshUniforms*>(uniforms);
    const math::vec4     pos       = varyings[0];
    const math::vec4     uv        = varyings[1];
    const math::vec4     norm      = math::normalize(varyings[2]);
    math::vec4&       output    = outputs[0];
    const SR_Texture*    pTexture  = pUniforms->pTexture;
    math::vec4           pixel;

    {
        // vectors are faster than colors
        math::vec3_t<uint8_t>&& pixel8 = pTexture->nearest<math::vec3_t<uint8_t>>(uv[0], uv[1]);
        //const math::vec4&& pixelF = color_cast<float, uint8_t>(pixel8);
        const math::vec4_t<uint8_t> pixelF{pixel8[0], pixel8[1], pixel8[2], 255};
        //pixel = math::vec4{pixelF.r, pixelF.g, pixelF.b, 1.f};

        pixel = (math::vec4)pixelF * math::vec4{0.00392156862745f};
        pixel[0] = math::pow(pixel[0], 2.2f);
        pixel[1] = math::pow(pixel[1], 2.2f);
        pixel[2] = math::pow(pixel[2], 2.2f);
    }

    // surface model
    const math::vec4     camPos           = pUniforms->light.pos;
    const math::vec4&&   viewDir          = math::normalize(camPos - pos);
    const math::vec4     lightPos         = {30.f, 45.f, 45.f, 0.f};
    const math::vec4     albedo           = {1.f};
    constexpr float      metallic         = 0.4f;
    constexpr float      roughness        = 0.15f;
    constexpr float      ambientIntensity = 0.25f;

    // Metallic reflectance at a normal incidence
    // 0.04f should be close to plastic.
    const math::vec4   surfaceConstant   = {0.04f, 0.04f, 0.04f, 1.f};
    const math::vec4&& surfaceReflection = math::mix(surfaceConstant, albedo, metallic);

    math::vec4         lightDir0         = {0.f};
    math::vec4&&       lightDirN         = math::normalize(lightPos - pos);
    math::vec4&&       hemisphere        = math::normalize(viewDir + lightDirN);

    const float        distance          = math::length(lightPos - pos);
    const float        attenuation       = math::rcp(distance * distance);
    math::vec4&&       radianceObj       = pUniforms->light.diffuse * attenuation;

    const float        ndf               = distribution_ggx(norm, hemisphere, roughness);
    const float        geom              = geometry_smith(norm, viewDir, lightPos, roughness);
    const math::vec4&& fresnel           = fresnel_schlick(math::clamp(math::dot(hemisphere, viewDir), 0.f, 1.f), surfaceReflection);

    const math::vec4&& brdf              = fresnel * ndf * geom;
    const float        cookTorrance      = math::rcp(4.f * math::max(math::dot(norm, viewDir), 0.f) * math::max(math::dot(norm, lightDirN), 0.f));
    const math::vec4&& specular          = brdf * math::max(cookTorrance, LS_EPSILON); // avoid divide-by-0

    const math::vec4&  specContrib       = surfaceReflection;
    const math::vec4&& refractRatio      = (math::vec4{1.f} - specContrib) * (math::vec4{1.f} - metallic);

    const float normDotLight             = math::max(math::dot(norm, lightDirN), 0.f);
    lightDir0                            += (refractRatio * albedo / LS_PI + specular) * radianceObj * normDotLight;

    const math::vec4&& ambient           = pUniforms->light.ambient * albedo * ambientIntensity * pixel;

    // Color normalization and light contribution
    const math::vec4 outRGB = ambient + lightDir0;

    // Tone mapping
    output /= outRGB + 1.f;

    // HDR Tone mapping
    //const float exposure = 5.f;
    //outR = 1.f - math::exp(-outR * exposure);
    //outG = 1.f - math::exp(-outG * exposure);
    //outB = 1.f - math::exp(-outB * exposure);

    // Gamma correction
    constexpr float gamma = 1.f / 2.2f;
    output[0] = math::clamp(math::pow(outRGB[0], gamma), 0.f, 1.f);
    output[1] = math::clamp(math::pow(outRGB[1], gamma), 0.f, 1.f);
    output[2] = math::clamp(math::pow(outRGB[2], gamma), 0.f, 1.f);
    output[3] = 1.f;

    return true;
}



SR_FragmentShader texture_frag_shader()
{
    SR_FragmentShader shader;
    shader.numVaryings = 3;
    shader.numOutputs  = 1;
    shader.blend       = SR_BLEND_OFF;
    shader.depthTest   = SR_DEPTH_TEST_ON;
    shader.depthMask   = SR_DEPTH_MASK_ON;
    shader.shader      = _texture_frag_shader_spot;
    //shader.shader      = _texture_frag_shader_pbr;

    return shader;
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
    retCode = vbo.init(numVerts*stride);
    if (retCode != 0)
    {
        std::cerr << "Error while creating a VBO: " << retCode << std::endl;
        abort();
    }

    size_t vaoId = context.create_vao();
    SR_VertexArray& vao = context.vao(vaoId);
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
    vao.set_binding(0, numVboBytes, stride, SR_Dimension::VERTEX_DIMENSION_3, SR_DataType::VERTEX_DATA_FLOAT);
    numVboBytes += sizeof(verts);

    assert(numVboBytes == (numVerts*stride));

    ls::utils::Pointer<size_t[]> meshId{new size_t[1]};
    meshId[0] = 0;

    graph.mNodes.push_back({SR_SceneNodeType::NODE_TYPE_EMPTY, 0, 0, SCENE_NODE_ROOT_ID});
    graph.mBaseTransforms.push_back(math::mat4{1.f});
    graph.mCurrentTransforms.push_back(SR_Transform{math::mat4{1.f}, SR_TRANSFORM_TYPE_MODEL});
    graph.mNodeNames.push_back(std::string{"AABB"});
    graph.mModelMatrices.push_back(math::mat4{1.f});
    graph.mNodeMeshes.emplace_back(std::move(meshId));
    graph.mNumNodeMeshes.push_back(1);
    graph.mMeshes.emplace_back(SR_Mesh());
    SR_Mesh& mesh = graph.mMeshes.back();
    mesh.vaoId = vaoId;
    mesh.elementBegin = 0;
    mesh.elementEnd = numVerts;
    mesh.mode = SR_RenderMode::RENDER_MODE_TRI_WIRE;
    mesh.materialId = (uint32_t)-1;

    return 0;
}



/*-----------------------------------------------------------------------------
 * Create the context for a demo scene
-----------------------------------------------------------------------------*/
utils::Pointer<SR_SceneGraph> create_context()
{
    int retCode = 0;

    #ifdef LS_ARCH_X86
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _mm_setcsr(_mm_getcsr() | 0x8040); // denormals-are-zero
    #endif


    SR_SceneFileLoader meshLoader;
    utils::Pointer<SR_SceneGraph> pGraph{new SR_SceneGraph{}};
    SR_Context& context = pGraph->mContext;
    uint32_t fboId   = (uint32_t)context.create_framebuffer();
    uint32_t texId   = (uint32_t)context.create_texture();
    uint32_t depthId = (uint32_t)context.create_texture();

    retCode = context.num_threads(SR_TEST_MAX_THREADS);
    assert(retCode == SR_TEST_MAX_THREADS);

    SR_Texture& tex = context.texture(texId);
    retCode = tex.init(SR_ColorDataType::SR_COLOR_RGBA_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
    assert(retCode == 0);

    SR_Texture& depth = context.texture(depthId);
    retCode = depth.init(SR_ColorDataType::SR_COLOR_R_FLOAT, IMAGE_WIDTH, IMAGE_HEIGHT, 1);
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

    // cube exists at index 0
#if SR_TEST_DEBUG_AABBS
    retCode = scene_load_cube(*pGraph);
    assert(retCode == 0);
#endif

    //retCode = meshLoader.load("testdata/african_head/african_head.obj");
    //retCode = meshLoader.load("testdata/bob/Bob.md5mesh");
    //retCode = meshLoader.load("testdata/rover/testmesh.dae");
    //retCode = meshLoader.load("testdata/base_model.dae");
    retCode = meshLoader.load("testdata/sibenik/sibenik.obj");
    assert(retCode != 0);

    retCode = pGraph->import(meshLoader.data());
    assert(retCode == 0);

    pGraph->mCurrentTransforms[0].scale(math::vec3{20.f});
    pGraph->update();

    const SR_VertexShader&&   normVertShader = normal_vert_shader();
    const SR_FragmentShader&& normFragShader = normal_frag_shader();
    const SR_VertexShader&&   texVertShader  = texture_vert_shader();
    const SR_FragmentShader&& texFragShader  = texture_frag_shader();
    const SR_VertexShader&&   boxVertShader  = box_vert_shader();
    const SR_FragmentShader&& boxFragShader  = box_frag_shader();

    // I keep getting this weird error about alignment so I'm using malloc
    std::shared_ptr<MeshUniforms>  pUniforms{(MeshUniforms*)ls::utils::aligned_malloc(sizeof(MeshUniforms)), [](MeshUniforms* p)->void {ls::utils::aligned_free(p);}};

    pUniforms->light.pos        = math::vec4{30.f, 45.f, 45.f, 1.f};
    pUniforms->light.ambient    = math::vec4{1.f};
    pUniforms->light.diffuse    = math::vec4{1.f};
    pUniforms->light.specular   = math::vec4{1.f};
    pUniforms->point.constant   = 1.f;
    pUniforms->point.linear     = 0.009f;
    pUniforms->point.quadratic  = 0.000018f;
    pUniforms->spot.innerCutoff = std::cos(LS_DEG2RAD(6.5f));
    pUniforms->spot.outerCutoff = std::cos(LS_DEG2RAD(13.f));
    pUniforms->spot.epsilon     = math::rcp(pUniforms->spot.innerCutoff - pUniforms->spot.outerCutoff);

    uint32_t texShaderId  = (uint32_t)context.create_shader(texVertShader,  texFragShader,  pUniforms);
    uint32_t normShaderId = (uint32_t)context.create_shader(normVertShader, normFragShader, pUniforms);
    uint32_t boxShaderId  = (uint32_t)context.create_shader(boxVertShader,  boxFragShader,  pUniforms);

    assert(texShaderId == 0);
    assert(normShaderId == 1);
    assert(boxShaderId == 2);
    (void)texShaderId;
    (void)normShaderId;
    (void)boxShaderId;

    //const math::mat4&& viewMatrix = math::look_at(math::vec3{75.f}, math::vec3{0.f, 10.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& viewMatrix = math::look_at(math::vec3{0.f}, math::vec3{3.f, -5.f, 0.f}, math::vec3{0.f, 1.f, 0.f});
    const math::mat4&& projMatrix = math::infinite_perspective(LS_DEG2RAD(60.f), (float)IMAGE_WIDTH/(float)IMAGE_HEIGHT, 0.01f);

    render_scene(pGraph.get(), projMatrix*viewMatrix);

    //sr_img_save_ppm(IMAGE_WIDTH, IMAGE_HEIGHT, reinterpret_cast<const math::vec3_t<uint8_t>*>(tex.data()), "window_buffer_test.ppm");

    //SR_Texture& baseTex = context.texture(2);
    //sr_img_save_ppm(baseTex.width(), baseTex.height(), reinterpret_cast<const math::vec3_t<uint8_t>*>(baseTex.data()), "window_buffer_texture.ppm");

    if (retCode != 0)
    {
        abort();
    }

    std::cout << "First frame rendered." << std::endl;

    return pGraph;
}



/*-----------------------------------------------------------------------------
 * Render a scene
-----------------------------------------------------------------------------*/
void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix)
{
    SR_Context& context = pGraph->mContext;
    MeshUniforms* pUniforms = static_cast<MeshUniforms*>(context.shader(0).uniforms().get());

    for (SR_SceneNode& n : pGraph->mNodes)
    {
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
            const SR_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];
            pUniforms->pTexture = material.pTextures[0];

            // Use the textureless shader if needed
            /*
            if (!material.pTextures[0])
            {
                continue;
            }
            */
            const size_t shaderId = (size_t)(material.pTextures[0] == nullptr);
            //const uint32_t shaderId = 0;

            if (!sr_is_visible(box, pUniforms->mvpMatrix))
            {
                continue;
            }

            context.draw(m, shaderId, 0);
        }
    }
}



/*-------------------------------------
 * Radar-based frustum culling method as described by Hernandez-Rudomin in
 * their paper "A Rendering Pipeline for Real-time Crowds."
 *
 * https://pdfs.semanticscholar.org/4fae/54e3f9e79ba09ead5702648664b9932a1d3f.pdf
-------------------------------------*/
bool is_visible(
    float aspect,
    float fov,
    const SR_Transform& camTrans,
    const math::mat4& modelMat,
    const SR_BoundingBox& bounds) noexcept
{
    const float        viewAngle = math::const_tan(fov*0.5f);
    const math::vec3&& c         = camTrans.get_abs_position();
    const math::mat3&& t         = math::mat3{math::transpose(camTrans.get_transform())};
    const math::vec3&  cx        = t[0];
    const math::vec3&  cy        = t[1];
    const math::vec3&& cz        = -t[2];
    const math::vec4&  trr0      = bounds.get_top_rear_right();
    const math::vec4&  bfl0      = bounds.get_bot_front_left();
    const math::vec4&& trr       = modelMat * trr0;
    const math::vec4&& bfl       = modelMat * bfl0;
    constexpr float    delta     = 0.f;

    const math::vec3   points[]  = {
        {trr[0], bfl[1], bfl[2]},
        {trr[0], trr[1], bfl[2]},
        {trr[0], trr[1], trr[2]},
        {bfl[0], trr[1], trr[2]},
        {bfl[0], bfl[1], trr[2]},
        {bfl[0], bfl[1], bfl[2]},
        {trr[0], bfl[1], trr[2]},
        {bfl[0], trr[1], bfl[2]}
    };

    float objX, objY, objZ, xAspect, yAspect;

    for (unsigned i = 0; i < LS_ARRAY_SIZE(points); ++i)
    {
        const math::vec3& p = points[i];

        // compute vector from camera position to p
        const math::vec3&& v = p - c;

        // compute and test the Z coordinate
        objZ = math::dot(v, cz);
        if (objZ < 0.f)
        {
            continue;
        }

        // compute and test the Y coordinate
        objY = math::dot(v, cy);
        yAspect = objZ * viewAngle;
        yAspect += delta;
        if (objY > yAspect || objY < -yAspect)
        {
            continue;
        }

        // compute and test the X coordinate
        objX = math::dot(v, cx);
        xAspect = yAspect * aspect;
        xAspect += delta;
        if (objX > xAspect || objX < -xAspect)
        {
            continue;
        }

        return true;
    }

    const math::vec3&  cWorld  = camTrans.get_position();
    const math::vec3   bboxMin = {bfl[0], bfl[1], bfl[2]};
    const math::vec3   bboxMax = {trr[0], trr[1], trr[2]};

    return cWorld > bboxMin && cWorld < bboxMax;
}




void render_scene(SR_SceneGraph* pGraph, const math::mat4& vpMatrix, float aspect, float fov, const SR_Transform& camTrans)
{
    SR_Context&    context   = pGraph->mContext;
    MeshUniforms*  pUniforms = static_cast<MeshUniforms*>(context.shader(0).uniforms().get());
    unsigned       numHidden = 0;
    unsigned       numTotal  = 0;

    for (SR_SceneNode& n : pGraph->mNodes)
    {
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
            const SR_BoundingBox& box        = pGraph->mMeshBounds[nodeMeshId];
            const SR_Material&    material   = pGraph->mMaterials[m.materialId];

            pUniforms->pTexture = material.pTextures[0];

            // Use the textureless shader if needed
            /*
            if (!material.pTextures[0])
            {
                continue;
            }
            */
            const size_t shaderId = (size_t)(material.pTextures[0] == nullptr);
            //const uint32_t shaderId = 0;

            ++numTotal;

            if (!is_visible(aspect, fov, camTrans, modelMat, box))// && !sr_is_visible(box, pUniforms->mvpMatrix))
            {
                ++numHidden;
                continue;
            }

            context.draw(m, shaderId, 0);
        }
    }

    // debugging
#if SR_TEST_DEBUG_AABBS
    const SR_Mesh& boxMesh = pGraph->mMeshes[0];

    for (SR_SceneNode& n : pGraph->mNodes)
    {
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
            const size_t nodeMeshId = meshIds[meshId];
            const SR_BoundingBox& box = pGraph->mMeshBounds[nodeMeshId];
            pUniforms->aabb = &box;

            if (!is_visible(aspect, fov, camTrans, modelMat, box))
            {
                continue;
            }

            context.draw(boxMesh, 2,  0);
        }
    }
#endif

    /*
    std::cout
        << "Meshes Hidden: " << numHidden << '/' << numTotal << " (" << 100.f*((float)numHidden/(float)numTotal) << "%)."
        << std::endl;
    */
}
