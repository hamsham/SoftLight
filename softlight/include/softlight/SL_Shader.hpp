
/**
 * @file Publicly-exposed shader functions & types
 */
#ifndef SL_SHADER_HPP
#define SL_SHADER_HPP

#include "lightsky/math/vec4.h"

#include "softlight/SL_PipelineState.hpp"
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



class SL_UniformBuffer;
class SL_VertexArray;
class SL_VertexBuffer;



/*-----------------------------------------------------------------------------
 * Vertex Shaders
-----------------------------------------------------------------------------*/
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
struct SL_Shader
{
    SL_PipelineState pipelineState;

    ls::math::vec4_t<float> (*pVertShader)(SL_VertexParam& vertParams);

    bool (*pFragShader)(SL_FragmentParam& perFragParams);

    // Shared pointers are only changed in the move and copy operators
    SL_UniformBuffer* pUniforms;
};



#endif /* SL_SHADER_HPP */
