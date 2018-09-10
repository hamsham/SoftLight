
#include "lightsky/math/scalar_utils.h"

#include "lightsky/utils/WorkerThread.hpp"

#include "soft_render/SR_Geometry.hpp"
#include "soft_render/SR_Shader.hpp"

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"



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
SR_Shader::~SR_Shader() noexcept
{
}


/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_Shader::SR_Shader(
    const SR_VertexShader& vertShader,
    const SR_FragmentShader& fragShader,
    const std::shared_ptr<SR_UniformBuffer>& pUniforms) noexcept :
    mVertShader(vertShader),
    mFragShader(fragShader),
    mUniforms{pUniforms}
{}


/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_Shader::SR_Shader(const SR_Shader& s) noexcept :
    mVertShader(s.mVertShader),
    mFragShader(s.mFragShader),
    mUniforms{s.mUniforms}
{
}


/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SR_Shader::SR_Shader(SR_Shader&& s) noexcept :
    mVertShader(std::move(s.mVertShader)),
    mFragShader(std::move(s.mFragShader)),
    mUniforms{std::move(s.mUniforms)}
{}


/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SR_Shader& SR_Shader::operator=(const SR_Shader& s) noexcept
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
SR_Shader& SR_Shader::operator=(SR_Shader&& s) noexcept
{
    if (this != &s)
    {
        mVertShader = std::move(s.mVertShader);
        mFragShader = std::move(s.mFragShader);
        mUniforms = std::move(s.mUniforms);
    }

    return *this;
}
