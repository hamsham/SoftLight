
#include <algorithm> // std::move (overload)
#include <cassert>
#include <iostream>
#include <iterator> // std::back_inserter
#include <utility> // std::move

#include "soft_render/SR_Context.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_Framebuffer.hpp"
#include "soft_render/SR_IndexBuffer.hpp"
#include "soft_render/SR_Shader.hpp"
#include "soft_render/SR_Texture.hpp"
#include "soft_render/SR_UniformBuffer.hpp"
#include "soft_render/SR_VertexArray.hpp"
#include "soft_render/SR_VertexBuffer.hpp"
#include "soft_render/SR_WindowBuffer.hpp"



/*-------------------------------------
 *
-------------------------------------*/
SR_Context::~SR_Context() noexcept
{
    terminate();
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Context::SR_Context() noexcept :
    mVaos{},
    mTextures{},
    mFbos{},
    mVbos{},
    mIbos{},
    mUniforms{},
    mShaders{},
    mProcessors{}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Context::SR_Context(const SR_Context& c) noexcept :
    mVaos{c.mVaos},
    mTextures{},
    mFbos{c.mFbos},
    mVbos{c.mVbos},
    mIbos{c.mIbos},
    mUniforms{c.mUniforms},
    mShaders{c.mShaders},
    mProcessors{c.mProcessors}
{
    mTextures.reserve(c.mTextures.size());

    for (SR_Texture* pTex : c.mTextures)
    {
        mTextures.push_back(new SR_Texture{*pTex});
    }
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Context::SR_Context(SR_Context&& c) noexcept :
    mVaos{std::move(c.mVaos)},
    mTextures{std::move(c.mTextures)},
    mFbos{std::move(c.mFbos)},
    mVbos{std::move(c.mVbos)},
    mIbos{std::move(c.mIbos)},
    mUniforms{std::move(c.mUniforms)},
    mShaders{std::move(c.mShaders)},
    mProcessors{std::move(c.mProcessors)}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_Context& SR_Context::operator=(const SR_Context& c) noexcept
{
    if (this != &c)
    {
        mVaos       = c.mVaos;
        mFbos       = c.mFbos;
        mVbos       = c.mVbos;
        mIbos       = c.mIbos;
        mUniforms   = c.mUniforms;
        mShaders    = c.mShaders;

        for (SR_Texture* pTex : mTextures)
        {
            delete pTex;
        }

        mTextures.clear();
        mTextures.reserve(c.mTextures.size());

        for (SR_Texture* pTex : c.mTextures)
        {
            mTextures.push_back(new SR_Texture{*pTex});
        }

        mProcessors = c.mProcessors;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Context& SR_Context::operator=(SR_Context&& c) noexcept
{
    if (this != &c)
    {
        mVaos       = std::move(c.mVaos);
        mTextures   = std::move(c.mTextures);
        mFbos       = std::move(c.mFbos);
        mVbos       = std::move(c.mVbos);
        mIbos       = std::move(c.mIbos);
        mUniforms   = std::move(c.mUniforms);
        mShaders    = std::move(c.mShaders);
        mProcessors = std::move(c.mProcessors);
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_VertexArray>& SR_Context::vaos() const
{
    return mVaos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_VertexArray& SR_Context::vao(std::size_t index) const
{
    return mVaos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_VertexArray& SR_Context::vao(std::size_t index)
{
    return mVaos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_vao()
{
    mVaos.emplace_back(SR_VertexArray{});
    return mVaos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_vao(std::size_t index)
{
    mVaos.erase(mVaos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_Texture*>& SR_Context::textures() const
{
    return mTextures;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_Texture& SR_Context::texture(std::size_t index) const
{
    return *mTextures[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Texture& SR_Context::texture(std::size_t index)
{
    return *mTextures[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_texture()
{
    mTextures.emplace_back(new SR_Texture{});
    return mTextures.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_texture(std::size_t index)
{
    delete mTextures[index];
    mTextures.erase(mTextures.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_Framebuffer>& SR_Context::framebuffers() const
{
    return mFbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_Framebuffer& SR_Context::framebuffer(std::size_t index) const
{
    return mFbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Framebuffer& SR_Context::framebuffer(std::size_t index)
{
    return mFbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_framebuffer()
{
    mFbos.emplace_back(SR_Framebuffer{});
    return mFbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_framebuffer(std::size_t index)
{
    mFbos.erase(mFbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_VertexBuffer>& SR_Context::vbos() const
{
    return mVbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_VertexBuffer& SR_Context::vbo(std::size_t index) const
{
    return mVbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_VertexBuffer& SR_Context::vbo(std::size_t index)
{
    return mVbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_vbo()
{
    mVbos.emplace_back(SR_VertexBuffer{});
    return mVbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_vbo(std::size_t index)
{
    mVbos.erase(mVbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_IndexBuffer>& SR_Context::ibos() const
{
    return mIbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_IndexBuffer& SR_Context::ibo(std::size_t index) const
{
    return mIbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_IndexBuffer& SR_Context::ibo(std::size_t index)
{
    return mIbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_ibo()
{
    mIbos.emplace_back(SR_IndexBuffer{});
    return mIbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_ibo(std::size_t index)
{
    mIbos.erase(mIbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_UniformBuffer>& SR_Context::ubos() const
{
    return mUniforms;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_UniformBuffer& SR_Context::ubo(std::size_t index) const
{
    return mUniforms[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_UniformBuffer& SR_Context::ubo(std::size_t index)
{
    return mUniforms[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_ubo()
{
    mUniforms.emplace_back(SR_UniformBuffer{});
    return mUniforms.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_ubo(std::size_t index)
{
    mUniforms.erase(mUniforms.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const std::vector<SR_Shader>& SR_Context::shaders() const
{
    return mShaders;
}



/*-------------------------------------
 *
-------------------------------------*/
const SR_Shader& SR_Context::shader(std::size_t index) const
{
    return mShaders[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SR_Shader& SR_Context::shader(std::size_t index)
{
    return mShaders[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_shader(
    const SR_VertexShader& vertShader,
    const SR_FragmentShader& fragShader)
{
    if (vertShader.numVaryings < fragShader.numVaryings)
    {
        return (std::size_t)-1;
    }

    if (vertShader.numVaryings > SR_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (fragShader.numVaryings > SR_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (!fragShader.numOutputs)
    {
        return (std::size_t)-1;
    }

    mShaders.emplace_back(SR_Shader{vertShader, fragShader});
    return mShaders.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SR_Context::create_shader(
    const SR_VertexShader& vertShader,
    const SR_FragmentShader& fragShader,
    std::size_t uniformIndex)
{
    if (vertShader.numVaryings < fragShader.numVaryings)
    {
        return (std::size_t)-1;
    }

    if (vertShader.numVaryings > SR_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (fragShader.numVaryings > SR_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (!fragShader.numOutputs)
    {
        return (std::size_t)-1;
    }

    mShaders.emplace_back(SR_Shader{vertShader, fragShader, mUniforms[uniformIndex]});
    return mShaders.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::destroy_shader(std::size_t index)
{
    mShaders.erase(mShaders.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::terminate()
{
    mVaos.clear();
    mVaos.shrink_to_fit();

    for (SR_Texture* pTex : mTextures)
    {
        delete pTex;
    }
    mTextures.clear();
    mTextures.shrink_to_fit();

    mFbos.clear();
    mFbos.shrink_to_fit();

    mVbos.clear();
    mVbos.shrink_to_fit();

    mIbos.clear();
    mIbos.shrink_to_fit();

    mShaders.clear();
    mShaders.shrink_to_fit();

    mProcessors.clear_fragment_bins();
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_Context::import(SR_Context&& inContext) noexcept
{
    std::vector<SR_VertexArray>&  inVaos     = inContext.mVaos;
    std::vector<SR_Texture*>&     inTextures = inContext.mTextures;
    std::vector<SR_Framebuffer>&  inFbos     = inContext.mFbos;
    std::vector<SR_VertexBuffer>& inVbos     = inContext.mVbos;
    std::vector<SR_IndexBuffer>&  inIbos     = inContext.mIbos;
    std::vector<SR_Shader>&       inShaders  = inContext.mShaders;

    const std::size_t baseVboId  = mVbos.size();
    const std::size_t baseIboId  = mIbos.size();

    for (SR_VertexArray& inVao : inVaos)
    {
        inVao.set_vertex_buffer(inVao.get_vertex_buffer() + baseVboId);
        inVao.set_index_buffer(inVao.get_index_buffer() + baseIboId);
    }

    std::move(inVaos.begin(), inVaos.end(), std::back_inserter(mVaos));
    inVaos.clear();

    std::move(inTextures.begin(), inTextures.end(), std::back_inserter(mTextures));
    inTextures.clear();

    std::move(inFbos.begin(), inFbos.end(), std::back_inserter(mFbos));
    inFbos.clear();

    std::move(inVbos.begin(), inVbos.end(), std::back_inserter(mVbos));
    inVbos.clear();

    std::move(inIbos.begin(), inIbos.end(), std::back_inserter(mIbos));
    inIbos.clear();

    std::move(inShaders.begin(), inShaders.end(), std::back_inserter(mShaders));
    inShaders.clear();
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SR_Context::draw(const SR_Mesh& m, size_t shaderId, size_t fboId) noexcept
{
    mProcessors.run_shader_processors(*this, m, 1, mShaders[shaderId], mFbos[fboId]);
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SR_Context::draw_multiple(const SR_Mesh* meshes, size_t numMeshes, size_t shaderId, size_t fboId) noexcept
{
    if (meshes != nullptr && numMeshes > 0)
    {
        mProcessors.run_shader_processors(*this, meshes, numMeshes, mShaders[shaderId], mFbos[fboId]);
    }
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SR_Context::draw_instanced(const SR_Mesh& m, size_t numInstances, size_t shaderId, size_t fboId) noexcept
{
    mProcessors.run_shader_processors(*this, m, numInstances, mShaders[shaderId], mFbos[fboId]);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SR_Context::blit(size_t outTextureId, size_t inTextureId) noexcept
{
    SR_Texture*    pOut  = mTextures[outTextureId];
    SR_Texture*    pIn   = mTextures[inTextureId];
    const uint16_t srcX0 = 0;
    const uint16_t srcY0 = 0;
    const uint16_t srcX1 = pIn->width();
    const uint16_t srcY1 = pIn->height();
    const uint16_t dstX0 = 0;
    const uint16_t dstY0 = 0;
    const uint16_t dstX1 = pOut->width();
    const uint16_t dstY1 = pOut->height();

    mProcessors.run_blit_processors(
        mTextures[inTextureId],
        mTextures[outTextureId],
        srcX0,
        srcY0,
        srcX1,
        srcY1,
        dstX0,
        dstY0,
        dstX1,
        dstY1);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SR_Context::blit(
    size_t outTextureId,
    size_t inTextureId,
    uint16_t srcX0,
    uint16_t srcY0,
    uint16_t srcX1,
    uint16_t srcY1,
    uint16_t dstX0,
    uint16_t dstY0,
    uint16_t dstX1,
    uint16_t dstY1) noexcept
{
    mProcessors.run_blit_processors(
        mTextures[inTextureId],
        mTextures[outTextureId],
        srcX0,
        srcY0,
        srcX1,
        srcY1,
        dstX0,
        dstY0,
        dstX1,
        dstY1);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SR_Context::blit(SR_WindowBuffer& buffer, size_t textureId) noexcept
{
    SR_Texture*    pTex  = mTextures[textureId];
    const uint16_t srcX0 = 0;
    const uint16_t srcY0 = 0;
    const uint16_t srcX1 = pTex->width();
    const uint16_t srcY1 = pTex->height();
    const uint16_t dstX0 = 0;
    const uint16_t dstY0 = 0;
    const uint16_t dstX1 = buffer.width();
    const uint16_t dstY1 = buffer.height();

    mProcessors.run_blit_processors(
        mTextures[textureId],
        &(buffer.mTexture),
        srcX0,
        srcY0,
        srcX1,
        srcY1,
        dstX0,
        dstY0,
        dstX1,
        dstY1);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SR_Context::blit(
    SR_WindowBuffer& buffer,
    size_t textureId,
    uint16_t srcX0,
    uint16_t srcY0,
    uint16_t srcX1,
    uint16_t srcY1,
    uint16_t dstX0,
    uint16_t dstY0,
    uint16_t dstX1,
    uint16_t dstY1) noexcept
{
    mProcessors.run_blit_processors(
        mTextures[textureId],
        &(buffer.mTexture),
        srcX0,
        srcY0,
        srcX1,
        srcY1,
        dstX0,
        dstY0,
        dstX1,
        dstY1);
}



/*-------------------------------------
 * Clear a Texture
-------------------------------------*/
template <typename color_type>
void SR_Context::clear(size_t outTextureId, const color_type& inColor) noexcept
{
    typedef typename color_type::value_type ConvertedType;

    // First, convert the input color to a float so we can work in a common
    // format
    const auto&& temp = color_cast<float, ConvertedType>(inColor);
    ls::math::vec4 temp4{0.f};

    // Import all relevant color channels
    switch (color_type::num_components())
    {
        case 4: temp4[3] = temp[3];
        case 3: temp4[2] = temp[2];
        case 2: temp4[1] = temp[1];
        case 1: temp4[0] = temp[0];
    }

    union
    {
        SR_ColorRType<uint8_t> r8;
        SR_ColorRType<uint16_t> r16;
        SR_ColorRType<uint32_t> r32;
        SR_ColorRType<uint64_t> r64;
        SR_ColorRType<float> rf;
        SR_ColorRType<double> rd;

        ls::math::vec2_t<uint8_t> rg8;
        ls::math::vec2_t<uint16_t> rg16;
        ls::math::vec2_t<uint32_t> rg32;
        ls::math::vec2_t<uint64_t> rg64;
        ls::math::vec2_t<float> rgf;
        ls::math::vec2_t<double> rgd;

        ls::math::vec3_t<uint8_t> rgb8;
        ls::math::vec3_t<uint16_t> rgb16;
        ls::math::vec3_t<uint32_t> rgb32;
        ls::math::vec3_t<uint64_t> rgb64;
        ls::math::vec3_t<float> rgbf;
        ls::math::vec3_t<double> rgbd;

        ls::math::vec4_t<uint8_t> rgba8;
        ls::math::vec4_t<uint16_t> rgba16;
        ls::math::vec4_t<uint32_t> rgba32;
        ls::math::vec4_t<uint64_t> rgba64;
        ls::math::vec4_t<float> rgbaf;
        ls::math::vec4_t<double> rgbad;
    } outColor;

    // Convert to the correct output type
    switch (mTextures[outTextureId]->type())
    {
        case SR_COLOR_R_8U:        outColor.r8 = color_cast<uint8_t, float>(*reinterpret_cast<const SR_ColorRf*>(temp4.v)); break;
        case SR_COLOR_RG_8U:       outColor.rg8 = color_cast<uint8_t, float>(*reinterpret_cast<const SR_ColorRGf*>(temp4.v)); break;
        case SR_COLOR_RGB_8U:      outColor.rgb8 = color_cast<uint8_t, float>(*reinterpret_cast<const SR_ColorRGBf*>(temp4.v)); break;
        case SR_COLOR_RGBA_8U:     outColor.rgba8 = color_cast<uint8_t, float>(*reinterpret_cast<const SR_ColorRGBAf*>(temp4.v)); break;

        case SR_COLOR_R_16U:        outColor.r16 = color_cast<uint16_t, float>(*reinterpret_cast<const SR_ColorRf*>(temp4.v)); break;
        case SR_COLOR_RG_16U:       outColor.rg16 = color_cast<uint16_t, float>(*reinterpret_cast<const SR_ColorRGf*>(temp4.v)); break;
        case SR_COLOR_RGB_16U:      outColor.rgb16 = color_cast<uint16_t, float>(*reinterpret_cast<const SR_ColorRGBf*>(temp4.v)); break;
        case SR_COLOR_RGBA_16U:     outColor.rgba16 = color_cast<uint16_t, float>(*reinterpret_cast<const SR_ColorRGBAf*>(temp4.v)); break;

        case SR_COLOR_R_32U:        outColor.r32 = color_cast<uint32_t, float>(*reinterpret_cast<const SR_ColorRf*>(temp4.v)); break;
        case SR_COLOR_RG_32U:       outColor.rg32 = color_cast<uint32_t, float>(*reinterpret_cast<const SR_ColorRGf*>(temp4.v)); break;
        case SR_COLOR_RGB_32U:      outColor.rgb32 = color_cast<uint32_t, float>(*reinterpret_cast<const SR_ColorRGBf*>(temp4.v)); break;
        case SR_COLOR_RGBA_32U:     outColor.rgba32 = color_cast<uint32_t, float>(*reinterpret_cast<const SR_ColorRGBAf*>(temp4.v)); break;

        case SR_COLOR_R_64U:        outColor.r64 = color_cast<uint64_t, float>(*reinterpret_cast<const SR_ColorRf*>(temp4.v)); break;
        case SR_COLOR_RG_64U:       outColor.rg64 = color_cast<uint64_t, float>(*reinterpret_cast<const SR_ColorRGf*>(temp4.v)); break;
        case SR_COLOR_RGB_64U:      outColor.rgb64 = color_cast<uint64_t, float>(*reinterpret_cast<const SR_ColorRGBf*>(temp4.v)); break;
        case SR_COLOR_RGBA_64U:     outColor.rgba64 = color_cast<uint64_t, float>(*reinterpret_cast<const SR_ColorRGBAf*>(temp4.v)); break;

        case SR_COLOR_R_FLOAT:        outColor.rf = *reinterpret_cast<const SR_ColorRf*>(temp4.v); break;
        case SR_COLOR_RG_FLOAT:       outColor.rgf = *reinterpret_cast<const SR_ColorRGf*>(temp4.v); break;
        case SR_COLOR_RGB_FLOAT:      outColor.rgbf = *reinterpret_cast<const SR_ColorRGBf*>(temp4.v); break;
        case SR_COLOR_RGBA_FLOAT:     outColor.rgbaf = *reinterpret_cast<const SR_ColorRGBAf*>(temp4.v); break;

        case SR_COLOR_R_DOUBLE:        outColor.rd = color_cast<double, float>(*reinterpret_cast<const SR_ColorRf*>(temp4.v)); break;
        case SR_COLOR_RG_DOUBLE:       outColor.rgd = color_cast<double, float>(*reinterpret_cast<const SR_ColorRGf*>(temp4.v)); break;
        case SR_COLOR_RGB_DOUBLE:      outColor.rgbd = color_cast<double, float>(*reinterpret_cast<const SR_ColorRGBf*>(temp4.v)); break;
        case SR_COLOR_RGBA_DOUBLE:     outColor.rgbad = color_cast<double, float>(*reinterpret_cast<const SR_ColorRGBAf*>(temp4.v)); break;

        default:
            LS_UNREACHABLE();
    }

    // Clear the output texture
    mProcessors.run_clear_processors(&outColor, mTextures[outTextureId]);
}

template void SR_Context::clear<SR_ColorRType<uint8_t>>( size_t, const SR_ColorRType<uint8_t>&) noexcept;
template void SR_Context::clear<SR_ColorRType<uint16_t>>(size_t, const SR_ColorRType<uint16_t>&) noexcept;
template void SR_Context::clear<SR_ColorRType<uint32_t>>(size_t, const SR_ColorRType<uint32_t>&) noexcept;
template void SR_Context::clear<SR_ColorRType<uint64_t>>(size_t, const SR_ColorRType<uint64_t>&) noexcept;
template void SR_Context::clear<SR_ColorRType<float>>(   size_t, const SR_ColorRType<float>&) noexcept;
template void SR_Context::clear<SR_ColorRType<double>>(  size_t, const SR_ColorRType<double>&) noexcept;

template void SR_Context::clear<ls::math::vec2_t<uint8_t>>( size_t, const ls::math::vec2_t<uint8_t>&) noexcept;
template void SR_Context::clear<ls::math::vec2_t<uint16_t>>(size_t, const ls::math::vec2_t<uint16_t>&) noexcept;
template void SR_Context::clear<ls::math::vec2_t<uint32_t>>(size_t, const ls::math::vec2_t<uint32_t>&) noexcept;
template void SR_Context::clear<ls::math::vec2_t<uint64_t>>(size_t, const ls::math::vec2_t<uint64_t>&) noexcept;
template void SR_Context::clear<ls::math::vec2_t<float>>(   size_t, const ls::math::vec2_t<float>&) noexcept;
template void SR_Context::clear<ls::math::vec2_t<double>>(  size_t, const ls::math::vec2_t<double>&) noexcept;

template void SR_Context::clear<ls::math::vec3_t<uint8_t>>( size_t, const ls::math::vec3_t<uint8_t>&) noexcept;
template void SR_Context::clear<ls::math::vec3_t<uint16_t>>(size_t, const ls::math::vec3_t<uint16_t>&) noexcept;
template void SR_Context::clear<ls::math::vec3_t<uint32_t>>(size_t, const ls::math::vec3_t<uint32_t>&) noexcept;
template void SR_Context::clear<ls::math::vec3_t<uint64_t>>(size_t, const ls::math::vec3_t<uint64_t>&) noexcept;
template void SR_Context::clear<ls::math::vec3_t<float>>(   size_t, const ls::math::vec3_t<float>&) noexcept;
template void SR_Context::clear<ls::math::vec3_t<double>>(  size_t, const ls::math::vec3_t<double>&) noexcept;

template void SR_Context::clear<ls::math::vec4_t<uint8_t>>( size_t, const ls::math::vec4_t<uint8_t>&) noexcept;
template void SR_Context::clear<ls::math::vec4_t<uint16_t>>(size_t, const ls::math::vec4_t<uint16_t>&) noexcept;
template void SR_Context::clear<ls::math::vec4_t<uint32_t>>(size_t, const ls::math::vec4_t<uint32_t>&) noexcept;
template void SR_Context::clear<ls::math::vec4_t<uint64_t>>(size_t, const ls::math::vec4_t<uint64_t>&) noexcept;
template void SR_Context::clear<ls::math::vec4_t<float>>(   size_t, const ls::math::vec4_t<float>&) noexcept;
template void SR_Context::clear<ls::math::vec4_t<double>>(  size_t, const ls::math::vec4_t<double>&) noexcept;



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
unsigned SR_Context::num_threads() const noexcept
{
    return mProcessors.concurrency();
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SR_Context::num_threads(unsigned inNumThreads) noexcept
{
    return mProcessors.concurrency(inNumThreads);
}
