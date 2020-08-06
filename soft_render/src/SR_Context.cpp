
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
const SR_AlignedVector<SR_VertexArray>& SR_Context::vaos() const
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
const SR_AlignedVector<SR_Texture*>& SR_Context::textures() const
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
const SR_AlignedVector<SR_Framebuffer>& SR_Context::framebuffers() const
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
const SR_AlignedVector<SR_VertexBuffer>& SR_Context::vbos() const
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
const SR_AlignedVector<SR_IndexBuffer>& SR_Context::ibos() const
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
const SR_AlignedVector<SR_UniformBuffer>& SR_Context::ubos() const
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
const SR_AlignedVector<SR_Shader>& SR_Context::shaders() const
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
    SR_AlignedVector<SR_VertexArray>&  inVaos     = inContext.mVaos;
    SR_AlignedVector<SR_Texture*>&     inTextures = inContext.mTextures;
    SR_AlignedVector<SR_Framebuffer>&  inFbos     = inContext.mFbos;
    SR_AlignedVector<SR_VertexBuffer>& inVbos     = inContext.mVbos;
    SR_AlignedVector<SR_IndexBuffer>&  inIbos     = inContext.mIbos;
    SR_AlignedVector<SR_Shader>&       inShaders  = inContext.mShaders;

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



/*--------------------------------------
 * Clear a framebuffer's color attachment
--------------------------------------*/
void SR_Context::clear_color_buffer(size_t fboId, size_t attachmentId, const ls::math::vec4_t<double>& color) noexcept
{
    SR_Texture* pTex = mFbos[fboId].get_color_buffer(attachmentId);
    SR_GeneralColor outColor = sr_match_color_for_type(pTex->type(), color);

    mProcessors.run_clear_processors(&outColor.color, pTex);
}



/*--------------------------------------
 * Clear a framebuffer's depth attachment
--------------------------------------*/
void SR_Context::clear_depth_buffer(size_t fboId, double depth) noexcept
{
    SR_Texture* pTex = mFbos[fboId].get_depth_buffer();
    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pTex->bpp())
    {
        case sizeof(ls::math::half):
            depthVal.h = (ls::math::half)(float)depth;
            break;

        case sizeof(float):
            depthVal.f = (float)depth;
            break;

        case sizeof(double):
            depthVal.d = depth;
            break;

        default:
            return;
    }

    mProcessors.run_clear_processors(&depthVal, pTex);

}



/*--------------------------------------
 * Clear a framebuffer
--------------------------------------*/
void SR_Context::clear_framebuffer(size_t fboId, size_t attachmentId, const ls::math::vec4_t<double>& color, double depth) noexcept
{
    SR_Texture* pColorBuf = mFbos[fboId].get_color_buffer(attachmentId);
    SR_Texture* pDepth = mFbos[fboId].get_depth_buffer();
    SR_GeneralColor outColor = sr_match_color_for_type(pColorBuf->type(), color);

    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pDepth->bpp())
    {
        case sizeof(ls::math::half):
            depthVal.h = (ls::math::half)(float)depth;
            break;

        case sizeof(float):
            depthVal.f = (float)depth;
            break;

        case sizeof(double):
            depthVal.d = depth;
            break;

        default:
            return;
    }

    mProcessors.run_clear_processors(&outColor.color, &depthVal, pColorBuf, pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (2 attachments)
--------------------------------------*/
void SR_Context::clear_framebuffer(size_t fboId, const std::array<size_t, 2>& bufferIndices, const std::array<const ls::math::vec4_t<double>, 2>& colors, double depth) noexcept
{
    SR_Texture* pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SR_Texture*, 2> buffers{
        mFbos[fboId].get_color_buffer(bufferIndices[0]),
        mFbos[fboId].get_color_buffer(bufferIndices[1])
    };

    SR_GeneralColor tempColors[2] = {
        sr_match_color_for_type(buffers[0]->type(), colors[0]),
        sr_match_color_for_type(buffers[1]->type(), colors[1])
    };

    std::array<const void*, 2> outColors{
        &tempColors[0].color,
        &tempColors[1].color
    };

    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pDepth->bpp())
    {
        case sizeof(ls::math::half):
            depthVal.h = (ls::math::half)(float)depth;
            break;

        case sizeof(float):
            depthVal.f = (float)depth;
            break;

        case sizeof(double):
            depthVal.d = depth;
            break;

        default:
            return;
    }

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (3 attachments)
--------------------------------------*/
void SR_Context::clear_framebuffer(size_t fboId, const std::array<size_t, 3>& bufferIndices, const std::array<const ls::math::vec4_t<double>, 3>& colors, double depth) noexcept
{
    SR_Texture* pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SR_Texture*, 3> buffers{
        mFbos[fboId].get_color_buffer(bufferIndices[0]),
        mFbos[fboId].get_color_buffer(bufferIndices[1]),
        mFbos[fboId].get_color_buffer(bufferIndices[2])
    };

    SR_GeneralColor tempColors[3] = {
        sr_match_color_for_type(buffers[0]->type(), colors[0]),
        sr_match_color_for_type(buffers[1]->type(), colors[1]),
        sr_match_color_for_type(buffers[2]->type(), colors[2])
    };

    std::array<const void*, 3> outColors{
        &tempColors[0].color,
        &tempColors[1].color,
        &tempColors[2].color
    };

    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pDepth->bpp())
    {
        case sizeof(ls::math::half):
            depthVal.h = (ls::math::half)(float)depth;
            break;

        case sizeof(float):
            depthVal.f = (float)depth;
            break;

        case sizeof(double):
            depthVal.d = depth;
            break;

        default:
            return;
    }

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (4 attachments)
--------------------------------------*/
void SR_Context::clear_framebuffer(size_t fboId, const std::array<size_t, 4>& bufferIndices, const std::array<const ls::math::vec4_t<double>, 4>& colors, double depth) noexcept
{
    SR_Texture* pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SR_Texture*, 4> buffers{
        mFbos[fboId].get_color_buffer(bufferIndices[0]),
        mFbos[fboId].get_color_buffer(bufferIndices[1]),
        mFbos[fboId].get_color_buffer(bufferIndices[2]),
        mFbos[fboId].get_color_buffer(bufferIndices[3])
    };

    SR_GeneralColor tempColors[4] = {
        sr_match_color_for_type(buffers[0]->type(), colors[0]),
        sr_match_color_for_type(buffers[1]->type(), colors[1]),
        sr_match_color_for_type(buffers[2]->type(), colors[2]),
        sr_match_color_for_type(buffers[3]->type(), colors[3])
    };

    std::array<const void*, 4> outColors{
        &tempColors[0].color,
        &tempColors[1].color,
        &tempColors[2].color,
        &tempColors[3].color
    };

    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pDepth->bpp())
    {
        case sizeof(ls::math::half):
            depthVal.h = (ls::math::half)(float)depth;
            break;

        case sizeof(float):
            depthVal.f = (float)depth;
            break;

        case sizeof(double):
            depthVal.d = depth;
            break;

        default:
            return;
    }

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, pDepth);
}



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
