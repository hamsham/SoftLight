
#ifndef SR_SHADER_HPP
#define SR_SHADER_HPP

#include "soft_render/SR_ShaderUtil.hpp" // SR_SHADER_MAX_FRAG_OUTPUTS, SR_SHADER_MAX_VARYING_VECTORS



/*-----------------------------------------------------------------------------
 * Forward Declarations
-----------------------------------------------------------------------------*/
namespace ls
{
namespace math
{
    template <typename num_type>
    struct vec3_t;

    template <typename num_type>
    union vec4_t;
} // end math namespace
} // end ls namespace


class SR_Context;
class SR_Shader;
class SR_UniformBuffer;
class SR_VertexArray;
class SR_VertexBuffer;

enum SR_RenderMode : uint32_t; // SR_Geometry.hpp



/*-----------------------------------------------------------------------------
 * Vertex Shaders
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Triangle Cull Mode
-------------------------------------*/
enum SR_CullMode : uint8_t
{
    SR_CULL_BACK_FACE,
    SR_CULL_FRONT_FACE,
    SR_CULL_OFF
};



/*-------------------------------------
 * Parameters which go into a vert shader.
-------------------------------------*/
struct SR_VertexParam
{
    const SR_UniformBuffer* pUniforms;

    size_t vertId;
    size_t instanceId;
    const SR_VertexArray* pVao;
    const SR_VertexBuffer* pVbo;

    ls::math::vec4* pVaryings;
};



/*-------------------------------------
 * Vertex Shader Configuration.
-------------------------------------*/
struct SR_VertexShader
{
    uint8_t numVaryings;
    SR_CullMode cullMode;

    ls::math::vec4_t<float> (*shader)(SR_VertexParam& vertParams);
};



/*-----------------------------------------------------------------------------
 * Fragment Shaders
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Fragment Blending
-------------------------------------*/
enum SR_BlendMode : uint8_t
{
    SR_BLEND_OFF,
    SR_BLEND_ALPHA,
    SR_BLEND_PREMULTIPLED_ALPHA,
    SR_BLEND_ADDITIVE,
    SR_BLEND_SCREEN,
};



/*-------------------------------------
 * Depth-Write Configuration
-------------------------------------*/
enum SR_DepthMask : uint8_t
{
    SR_DEPTH_MASK_OFF,
    SR_DEPTH_MASK_ON
};



/*-------------------------------------
 * Depth Test Configuration
-------------------------------------*/
enum SR_DepthTest : uint8_t
{
    SR_DEPTH_TEST_OFF,
    SR_DEPTH_TEST_ON
};



/*-------------------------------------
 * Parameters which go into a frag shader.
-------------------------------------*/
struct SR_FragmentParam
{
    SR_FragCoordXYZ         coord;
    const SR_UniformBuffer* pUniforms;
    ls::math::vec4*         pVaryings;

    alignas(sizeof(ls::math::vec4)) ls::math::vec4 pOutputs[SR_SHADER_MAX_FRAG_OUTPUTS];
};



/*-------------------------------------
 * Fragment Shader Configuration.
-------------------------------------*/
struct SR_FragmentShader
{
    uint8_t      numVaryings;
    uint8_t      numOutputs;
    SR_BlendMode blend;
    SR_DepthTest depthTest;
    SR_DepthMask depthMask;

    bool (*shader)(SR_FragmentParam& perFragParams);
};



/*-----------------------------------------------------------------------------
 *
-----------------------------------------------------------------------------*/
class SR_Shader
{
    friend class SR_Context;
    friend struct SR_VertexProcessor;
    friend struct SR_FragmentProcessor;

  private:
    SR_VertexShader mVertShader;

    SR_FragmentShader mFragShader;

    // Shared pointers are only changed in the move and copy operators
    SR_UniformBuffer* mUniforms;


    SR_Shader(
        const SR_VertexShader& vertShader,
        const SR_FragmentShader& fragShader
    ) noexcept;

    SR_Shader(
        const SR_VertexShader& vertShader,
        const SR_FragmentShader& fragShader,
        SR_UniformBuffer& uniforms
    ) noexcept;

  public:
    ~SR_Shader() noexcept;

    SR_Shader(const SR_Shader& s) noexcept;

    SR_Shader(SR_Shader&& s) noexcept;

    SR_Shader& operator=(const SR_Shader& s) noexcept;

    SR_Shader& operator=(SR_Shader&& s) noexcept;

    uint8_t get_num_varyings() const noexcept;

    uint8_t get_num_fragment_outputs() const noexcept;

    const SR_UniformBuffer* uniforms() const noexcept;

    SR_UniformBuffer* uniforms() noexcept;

    void uniforms(SR_UniformBuffer* pUniforms) noexcept;

    const SR_VertexShader& vertex_shader() const noexcept;

    const SR_FragmentShader& fragment_shader() const noexcept;
};



/*--------------------------------------
 *
--------------------------------------*/
inline uint8_t SR_Shader::get_num_varyings() const noexcept
{
    return mVertShader.numVaryings;
}



/*--------------------------------------
 *
--------------------------------------*/
inline uint8_t SR_Shader::get_num_fragment_outputs() const noexcept
{
    return mFragShader.numOutputs;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SR_UniformBuffer* SR_Shader::uniforms() const noexcept
{
    return mUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline SR_UniformBuffer* SR_Shader::uniforms() noexcept
{
    return mUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline void SR_Shader::uniforms(SR_UniformBuffer* pUniforms) noexcept
{
    mUniforms = pUniforms;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SR_VertexShader& SR_Shader::vertex_shader() const noexcept
{
    return mVertShader;
}



/*--------------------------------------
 *
--------------------------------------*/
inline const SR_FragmentShader& SR_Shader::fragment_shader() const noexcept
{
    return mFragShader;
}



#endif /* SR_SHADER_HPP */
