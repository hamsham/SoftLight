
#include "lightsky/math/scalar_utils.h"

#include "lightsky/utils/WorkerThread.hpp"

#include "softlight/SL_Geometry.hpp"
#include "softlight/SL_Shader.hpp"

#include "softlight/SL_Context.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"



/*-----------------------------------------------------------------------------
 * Anonymous Helper Functions
-----------------------------------------------------------------------------*/
namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Shader Class
-----------------------------------------------------------------------------*/
/*--------------------------------------
 * Destructor
--------------------------------------*/
SL_Shader::~SL_Shader() noexcept
{
}


/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_Shader::SL_Shader(
    const SL_VertexShader& vertShader,
    const SL_FragmentShader& fragShader) noexcept :
    mVertShader(vertShader),
    mFragShader(fragShader),
    mUniforms{nullptr}
{}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_Shader::SL_Shader(
    const SL_VertexShader& vertShader,
    const SL_FragmentShader& fragShader,
    SL_UniformBuffer& uniforms) noexcept :
    mVertShader(vertShader),
    mFragShader(fragShader),
    mUniforms{&uniforms}
{}


/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_Shader::SL_Shader(const SL_Shader& s) noexcept :
    mVertShader(s.mVertShader),
    mFragShader(s.mFragShader),
    mUniforms{s.mUniforms}
{
}


/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_Shader::SL_Shader(SL_Shader&& s) noexcept :
    mVertShader(std::move(s.mVertShader)),
    mFragShader(std::move(s.mFragShader)),
    mUniforms{s.mUniforms}
{}


/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SL_Shader& SL_Shader::operator=(const SL_Shader& s) noexcept
{
    if (this != &s)
    {
        mVertShader = s.mVertShader;
        mFragShader = s.mFragShader;
        mUniforms = s.mUniforms;
    }

    return *this;
}


/*--------------------------------------
 * Move Operator
--------------------------------------*/
SL_Shader& SL_Shader::operator=(SL_Shader&& s) noexcept
{
    if (this != &s)
    {
        mVertShader = std::move(s.mVertShader);
        mFragShader = std::move(s.mFragShader);
        mUniforms = s.mUniforms;
    }

    return *this;
}
