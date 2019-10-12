
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
    mProcessors.run_shader_processors(this, &m, &mShaders[shaderId], &mFbos[fboId]);
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



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
unsigned SR_Context::num_threads() const noexcept
{
    return mProcessors.num_threads();
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SR_Context::num_threads(unsigned inNumThreads) noexcept
{
    return mProcessors.num_threads(inNumThreads);
}
