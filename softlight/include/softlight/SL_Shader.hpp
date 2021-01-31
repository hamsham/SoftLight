
/**
 * @file Publicly-exposed shader functions & types
 */
#ifndef SL_SHADER_HPP
#define SL_SHADER_HPP

#include "lightsky/math/vec4.h"

#include "softlight/SL_ShaderUtil.hpp" // SL_SHADER_MAX_FRAG_OUTPUTS, SL_SHADER_MAX_VARYING_VECTORS



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
    template <typename num_type>
    struct vec3_t;
} // end math namespace
} // end ls namespace


class SL_Context;
class SL_Shader;
class SL_UniformBuffer;
class SL_VertexArray;
class SL_VertexBuffer;

enum SL_RenderMode : uint32_t; // SL_Geometry.hpp



/*-----------------------------------------------------------------------------
 * Vertex Shaders
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Triangle Cull Mode
-------------------------------------*/
enum SL_CullMode : uint8_t
{
    SL_CULL_BACK_FACE,
    SL_CULL_FRONT_FACE,
    SL_CULL_OFF
};



/*-------------------------------------
 * Parameters which go into a vert shader.
-------------------------------------*/
struct SL_VertexParam
{
    const SL_UniformBuffer* pUniforms;

    size_t vertId;
    size_t instanceId;
    const SL_VertexArray* pVao;
    const SL_VertexBuffer* pVbo;

    ls::math::vec4* pVaryings;
};



/*-------------------------------------
 * Vertex Shader Configuration.
-------------------------------------*/
struct SL_VertexShader
{
    uint8_t numVaryings;
    SL_CullMode cullMode;

    ls::math::vec4_t<float> (*shader)(SL_VertexParam& vertParams);
};



/*-----------------------------------------------------------------------------
 * Fragment Shaders
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Fragment Blending
-------------------------------------*/
enum SL_BlendMode : uint8_t
{
    SL_BLEND_OFF,
    SL_BLEND_ALPHA,
    SL_BLEND_PREMULTIPLED_ALPHA,
    SL_BLEND_ADDITIVE,
    SL_BLEND_SCREEN,
};



/*-------------------------------------
 * Depth-Write Configuration
-------------------------------------*/
enum SL_DepthMask : uint8_t
{
    SL_DEPTH_MASK_OFF,
    SL_DEPTH_MASK_ON
};



/*-------------------------------------
 * Depth Test Configuration
-------------------------------------*/
enum SL_DepthTest : uint8_t
{
    SL_DEPTH_TEST_OFF,
    SL_DEPTH_TEST_ON
};



/*-------------------------------------
 * Parameters which go into a frag shader.
-------------------------------------*/
struct SL_FragmentParam
{
    SL_FragCoordXYZ         coord;
    const SL_UniformBuffer* pUniforms;

    alignas(sizeof(ls::math::vec4)*2) ls::math::vec4 pVaryings[SL_SHADER_MAX_VARYING_VECTORS];

    alignas(sizeof(ls::math::vec4)) ls::math::vec4 pOutputs[SL_SHADER_MAX_FRAG_OUTPUTS];
};



/*-------------------------------------
 * Fragment Shader Configuration.
-------------------------------------*/
struct SL_FragmentShader
{
    uint8_t      numVaryings;
    uint8_t      numOutputs;
    SL_BlendMode blend;
    SL_DepthTest depthTest;
    SL_DepthMask depthMask;

    bool (*shader)(SL_FragmentParam& perFragParams);
};



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SL_Shader
{
    friend class SL_Context;
    friend struct SL_VertexProcessor;
    friend struct SL_FragmentProcessor;

  private:
    SL_VertexShader mVertShader;

    SL_FragmentShader mFragShader;

    // Shared pointers are only changed in the move and copy operators
    SL_UniformBuffer* mUniforms;


    SL_Shader(
        const SL_VertexShader& vertShader,
        const SL_FragmentShader& fragShader
    ) noexcept;

    SL_Shader(
        const SL_VertexShader& vertShader,
        const SL_FragmentShader& fragShader,
        SL_UniformBuffer& uniforms
    ) noexcept;

  public:
    ~SL_Shader() noexcept;

    SL_Shader(const SL_Shader& s) noexcept;

    SL_Shader(SL_Shader&& s) noexcept;

    SL_Shader& operator=(const SL_Shader& s) noexcept;

    SL_Shader& operator=(SL_Shader&& s) noexcept;

    uint8_t get_num_varyings() const noexcept;

    uint8_t get_num_fragment_outputs() const noexcept;

    const SL_UniformBuffer* uniforms() const noexcept;

    SL_UniformBuffer* uniforms() noexcept;

    void uniforms(SL_UniformBuffer* pUniforms) noexcept;

    const SL_VertexShader& vertex_shader() const noexcept;

    const SL_FragmentShader& fragment_shader() const noexcept;
};



/*--------------------------------------
 *
--------------------------------------*/
inline uint8_t SL_Shader::get_num_varyings() const noexcept
{
    return mVertShader.numVaryings;
}



/*--------------------------------------
 *
--------------------------------------*/
inline uint8_t SL_Shader::get_num_fragment_outputs() const noexcept
{
    return mFragShader.numOutputs;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SL_UniformBuffer* SL_Shader::uniforms() const noexcept
{
    return mUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline SL_UniformBuffer* SL_Shader::uniforms() noexcept
{
    return mUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline void SL_Shader::uniforms(SL_UniformBuffer* pUniforms) noexcept
{
    mUniforms = pUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SL_VertexShader& SL_Shader::vertex_shader() const noexcept
{
    return mVertShader;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SL_FragmentShader& SL_Shader::fragment_shader() const noexcept
{
    return mFragShader;
}



#endif /* SL_SHADER_HPP */
