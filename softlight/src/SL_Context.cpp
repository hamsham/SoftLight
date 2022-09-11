
#include <algorithm> // std::move (overload)
#include <iterator> // std::back_inserter
#include <utility> // std::move

#include "softlight/SL_Context.hpp"
#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_Framebuffer.hpp"
#include "softlight/SL_IndexBuffer.hpp"
#include "softlight/SL_Shader.hpp"
#include "softlight/SL_Texture.hpp"
#include "softlight/SL_UniformBuffer.hpp"
#include "softlight/SL_VertexArray.hpp"
#include "softlight/SL_VertexBuffer.hpp"



/*-------------------------------------
 *
-------------------------------------*/
SL_Context::~SL_Context() noexcept
{
    for (SL_Texture* pTex : mTextures)
    {
        delete pTex;
    }
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Context::SL_Context() noexcept :
    mVaos{},
    mTextures{},
    mFbos{},
    mVbos{},
    mIbos{},
    mUniforms{},
    mShaders{},
    mViewState{},
    mProcessors{}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_Context::SL_Context(const SL_Context& c) noexcept :
    mVaos{c.mVaos},
    mTextures{},
    mFbos{c.mFbos},
    mVbos{c.mVbos},
    mIbos{c.mIbos},
    mUniforms{c.mUniforms},
    mShaders{c.mShaders},
    mViewState{c.mViewState},
    mProcessors{c.mProcessors}
{
    mTextures.reserve(c.mTextures.size());

    for (SL_Texture* pTex : c.mTextures)
    {
        mTextures.push_back(new SL_Texture{*pTex});
    }
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Context::SL_Context(SL_Context&& c) noexcept :
    mVaos{std::move(c.mVaos)},
    mTextures{std::move(c.mTextures)},
    mFbos{std::move(c.mFbos)},
    mVbos{std::move(c.mVbos)},
    mIbos{std::move(c.mIbos)},
    mUniforms{std::move(c.mUniforms)},
    mShaders{std::move(c.mShaders)},
    mViewState{std::move(c.mViewState)},
    mProcessors{std::move(c.mProcessors)}
{}



/*-------------------------------------
 *
-------------------------------------*/
SL_Context& SL_Context::operator=(const SL_Context& c) noexcept
{
    if (this != &c)
    {
        mVaos       = c.mVaos;
        mFbos       = c.mFbos;
        mVbos       = c.mVbos;
        mIbos       = c.mIbos;
        mUniforms   = c.mUniforms;
        mShaders    = c.mShaders;

        for (SL_Texture* pTex : mTextures)
        {
            delete pTex;
        }

        mTextures.clear();
        mTextures.reserve(c.mTextures.size());

        for (SL_Texture* pTex : c.mTextures)
        {
            mTextures.push_back(new SL_Texture{*pTex});
        }

        mViewState = c.mViewState;
        mProcessors = c.mProcessors;
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Context& SL_Context::operator=(SL_Context&& c) noexcept
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
        mViewState      = std::move(c.mViewState);
        mProcessors = std::move(c.mProcessors);
    }

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_VertexArray>& SL_Context::vaos() const
{
    return mVaos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_VertexArray& SL_Context::vao(std::size_t index) const
{
    return mVaos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_VertexArray& SL_Context::vao(std::size_t index)
{
    return mVaos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_vao()
{
    mVaos.push_back(SL_VertexArray{});
    return mVaos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_vao(std::size_t index)
{
    mVaos.erase(mVaos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_Texture*>& SL_Context::textures() const
{
    return mTextures;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_Texture& SL_Context::texture(std::size_t index) const
{
    return *mTextures[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Texture& SL_Context::texture(std::size_t index)
{
    return *mTextures[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_texture()
{
    mTextures.push_back(new SL_Texture{});
    return mTextures.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_texture(std::size_t index)
{
    delete mTextures[index];
    mTextures.erase(mTextures.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_Framebuffer>& SL_Context::framebuffers() const
{
    return mFbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_Framebuffer& SL_Context::framebuffer(std::size_t index) const
{
    return mFbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Framebuffer& SL_Context::framebuffer(std::size_t index)
{
    return mFbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_framebuffer()
{
    mFbos.push_back(SL_Framebuffer{});
    return mFbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_framebuffer(std::size_t index)
{
    mFbos.erase(mFbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_VertexBuffer>& SL_Context::vbos() const
{
    return mVbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_VertexBuffer& SL_Context::vbo(std::size_t index) const
{
    return mVbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_VertexBuffer& SL_Context::vbo(std::size_t index)
{
    return mVbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_vbo()
{
    mVbos.push_back(SL_VertexBuffer{});
    return mVbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_vbo(std::size_t index)
{
    mVbos.erase(mVbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_IndexBuffer>& SL_Context::ibos() const
{
    return mIbos;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_IndexBuffer& SL_Context::ibo(std::size_t index) const
{
    return mIbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_IndexBuffer& SL_Context::ibo(std::size_t index)
{
    return mIbos[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_ibo()
{
    mIbos.push_back(SL_IndexBuffer{});
    return mIbos.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_ibo(std::size_t index)
{
    mIbos.erase(mIbos.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_UniformBuffer>& SL_Context::ubos() const
{
    return mUniforms;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_UniformBuffer& SL_Context::ubo(std::size_t index) const
{
    return mUniforms[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_UniformBuffer& SL_Context::ubo(std::size_t index)
{
    return mUniforms[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_ubo()
{
    mUniforms.push_back(SL_UniformBuffer{});
    return mUniforms.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_ubo(std::size_t index)
{
    mUniforms.erase(mUniforms.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_AlignedVector<SL_Shader>& SL_Context::shaders() const
{
    return mShaders;
}



/*-------------------------------------
 *
-------------------------------------*/
const SL_Shader& SL_Context::shader(std::size_t index) const
{
    return mShaders[index];
}



/*-------------------------------------
 *
-------------------------------------*/
SL_Shader& SL_Context::shader(std::size_t index)
{
    return mShaders[index];
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_shader(
    const SL_VertexShader& vertShader,
    const SL_FragmentShader& fragShader)
{
    if (vertShader.numVaryings < fragShader.numVaryings)
    {
        return (std::size_t)-1;
    }

    if (vertShader.numVaryings > SL_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (fragShader.numVaryings > SL_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    SL_PipelineState pipeline;
    pipeline.reset();
    pipeline.cull_mode(vertShader.cullMode);
    pipeline.depth_test(fragShader.depthTest);
    pipeline.depth_mask(fragShader.depthMask);
    pipeline.blend_mode(fragShader.blend);

    switch (fragShader.numVaryings)
    {
        case 0: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_0); break;
        case 1: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_1); break;
        case 2: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_2); break;
        case 3: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_3); break;
        case 4: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_4); break;
    }

    switch (fragShader.numOutputs)
    {
        case 0: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_0); break;
        case 1: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_1); break;
        case 2: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_2); break;
        case 3: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_3); break;
        case 4: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_4); break;
    }

    SL_Shader shader;
    shader.pipelineState = pipeline;
    shader.pVertShader = vertShader.shader;
    shader.pFragShader = fragShader.shader;
    shader.pUniforms = nullptr;

    mShaders.push_back(shader);
    return mShaders.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
std::size_t SL_Context::create_shader(
    const SL_VertexShader& vertShader,
    const SL_FragmentShader& fragShader,
    std::size_t uniformIndex)
{
    if (uniformIndex >= mUniforms.size())
    {
        return (std::size_t)-1;
    }

    if (vertShader.numVaryings < fragShader.numVaryings)
    {
        return (std::size_t)-1;
    }

    if (vertShader.numVaryings > SL_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    if (fragShader.numVaryings > SL_SHADER_MAX_VARYING_VECTORS)
    {
        return (std::size_t)-1;
    }

    SL_PipelineState pipeline;
    pipeline.reset();
    pipeline.cull_mode(vertShader.cullMode);
    pipeline.depth_test(fragShader.depthTest);
    pipeline.depth_mask(fragShader.depthMask);
    pipeline.blend_mode(fragShader.blend);

    switch (fragShader.numVaryings)
    {
        case 0: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_0); break;
        case 1: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_1); break;
        case 2: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_2); break;
        case 3: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_3); break;
        case 4: pipeline.num_varyings(SL_VaryingCount::SL_VARYING_COUNT_4); break;
    }

    switch (fragShader.numOutputs)
    {
        case 0: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_0); break;
        case 1: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_1); break;
        case 2: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_2); break;
        case 3: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_3); break;
        case 4: pipeline.num_render_targets(SL_RenderTargetCount::SL_RENDER_TARGET_COUNT_4); break;
    }

    SL_Shader shader;
    shader.pipelineState = pipeline;
    shader.pVertShader = vertShader.shader;
    shader.pFragShader = fragShader.shader;
    shader.pUniforms = &mUniforms[uniformIndex];

    mShaders.push_back(shader);
    return mShaders.size() - 1;
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::destroy_shader(std::size_t index)
{
    mShaders.erase(mShaders.begin() + index);
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::terminate()
{
    mVaos.clear();
    mVaos.shrink_to_fit();

    for (SL_Texture* pTex : mTextures)
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

    mUniforms.clear();
    mUniforms.shrink_to_fit();

    mShaders.clear();
    mShaders.shrink_to_fit();

    mViewState.reset();

    mViewState.reset();

    mProcessors.concurrency(1);
}



/*-------------------------------------
 *
-------------------------------------*/
void SL_Context::import(SL_Context&& inContext) noexcept
{
    SL_AlignedVector<SL_VertexArray>&   inVaos     = inContext.mVaos;
    SL_AlignedVector<SL_Texture*>&      inTextures = inContext.mTextures;
    SL_AlignedVector<SL_Framebuffer>&   inFbos     = inContext.mFbos;
    SL_AlignedVector<SL_VertexBuffer>&  inVbos     = inContext.mVbos;
    SL_AlignedVector<SL_IndexBuffer>&   inIbos     = inContext.mIbos;
    SL_AlignedVector<SL_UniformBuffer>& inUniforms = inContext.mUniforms;
    SL_AlignedVector<SL_Shader>&        inShaders  = inContext.mShaders;

    const std::size_t baseVboId  = mVbos.size();
    const std::size_t baseIboId  = mIbos.size();

    for (SL_VertexArray& inVao : inVaos)
    {
        if (inVao.has_vertex_buffer())
        {
            inVao.set_vertex_buffer(inVao.get_vertex_buffer() + baseVboId);
        }

        if (inVao.has_index_buffer())
        {
            inVao.set_index_buffer(inVao.get_index_buffer() + baseIboId);
        }
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

    std::move(inUniforms.begin(), inUniforms.end(), std::back_inserter(mUniforms));
    inUniforms.clear();

    std::move(inShaders.begin(), inShaders.end(), std::back_inserter(mShaders));
    inShaders.clear();
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SL_Context::draw(const SL_Mesh& m, size_t shaderId, size_t fboId) noexcept
{
    mProcessors.run_shader_processors(*this, m, 1, mShaders[shaderId], mFbos[fboId]);
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SL_Context::draw_multiple(const SL_Mesh* meshes, size_t numMeshes, size_t shaderId, size_t fboId) noexcept
{
    if (meshes != nullptr && numMeshes > 0)
    {
        mProcessors.run_shader_processors(*this, meshes, numMeshes, mShaders[shaderId], mFbos[fboId]);
    }
}



/*-------------------------------------
 * Draw a mesh
-------------------------------------*/
void SL_Context::draw_instanced(const SL_Mesh& m, size_t numInstances, size_t shaderId, size_t fboId) noexcept
{
    mProcessors.run_shader_processors(*this, m, numInstances, mShaders[shaderId], mFbos[fboId]);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SL_Context::blit(size_t outTextureId, size_t inTextureId) noexcept
{
    SL_Texture*    pOut  = mTextures[outTextureId];
    SL_Texture*    pIn   = mTextures[inTextureId];
    const uint16_t srcX0 = 0;
    const uint16_t srcY0 = 0;
    const uint16_t srcX1 = pIn->width();
    const uint16_t srcY1 = pIn->height();
    const uint16_t dstX0 = 0;
    const uint16_t dstY0 = 0;
    const uint16_t dstX1 = pOut->width();
    const uint16_t dstY1 = pOut->height();

    this->blit(
        outTextureId,
        inTextureId,
        srcX0, srcY0,
        srcX1, srcY1,
        dstX0, dstY0,
        dstX1, dstY1);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SL_Context::blit(
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
    SL_TextureView& i = mTextures[inTextureId]->view();
    SL_TextureView& o = mTextures[outTextureId]->view();

    if (sl_is_compressed_color(mTextures[outTextureId]->type()) || sl_is_compressed_color(mTextures[inTextureId]->type()))
    {
        mProcessors.run_blit_compressed_processors(
            &i,
            &o,
            srcX0, srcY0,
            srcX1, srcY1,
            dstX0, dstY0,
            dstX1, dstY1);
    }
    else
    {
        mProcessors.run_blit_processors(
            &i,
            &o,
            srcX0, srcY0,
            srcX1, srcY1,
            dstX0, dstY0,
            dstX1, dstY1);
    }
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SL_Context::blit(SL_TextureView& buffer, size_t textureId) noexcept
{
    SL_Texture*    pTex  = mTextures[textureId];
    const uint16_t srcX0 = 0;
    const uint16_t srcY0 = 0;
    const uint16_t srcX1 = pTex->width();
    const uint16_t srcY1 = pTex->height();
    const uint16_t dstX0 = 0;
    const uint16_t dstY0 = 0;
    const uint16_t dstX1 = (uint16_t)buffer.width;
    const uint16_t dstY1 = (uint16_t)buffer.height;

    this->blit(
        buffer,
        textureId,
        srcX0, srcY0,
        srcX1, srcY1,
        dstX0, dstY0,
        dstX1, dstY1);
}



/*-------------------------------------
 * Blit to a window
-------------------------------------*/
void SL_Context::blit(
    SL_TextureView& buffer,
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
    SL_TextureView& t = mTextures[textureId]->view();

    if (sl_is_compressed_color(mTextures[textureId]->type()) || sl_is_compressed_color(buffer.type))
    {
        mProcessors.run_blit_compressed_processors(
            &t,    &buffer,
            srcX0, srcY0,
            srcX1, srcY1,
            dstX0, dstY0,
            dstX1, dstY1);
    }
    else
    {
        mProcessors.run_blit_processors(
            &t,    &buffer,
            srcX0, srcY0,
            srcX1, srcY1,
            dstX0, dstY0,
            dstX1, dstY1);
    }
}



/*--------------------------------------
 * Clear a framebuffer's color attachment
--------------------------------------*/
void SL_Context::clear_color_buffer(size_t fboId, unsigned attachmentId, const ls::math::vec4_t<double>& color) noexcept
{
    SL_TextureView& pTex = mFbos[fboId].get_color_buffer(attachmentId);
    SL_GeneralColor outColor = sl_match_color_for_type(pTex.type, color);

    mProcessors.run_clear_processors(&outColor.color, &pTex);
}



/*--------------------------------------
 * Clear a framebuffer's depth attachment
--------------------------------------*/
void SL_Context::clear_depth_buffer(size_t fboId, double depth) noexcept
{
    SL_TextureView& pTex = mFbos[fboId].get_depth_buffer();
    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pTex.bytesPerTexel)
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

    mProcessors.run_clear_processors(&depthVal, &pTex);

}



/*--------------------------------------
 * Clear a framebuffer
--------------------------------------*/
void SL_Context::clear_framebuffer(size_t fboId, unsigned attachmentId, const ls::math::vec4_t<double>& color, double depth) noexcept
{
    SL_TextureView& pColorBuf = mFbos[fboId].get_color_buffer(attachmentId);
    SL_TextureView& pDepth = mFbos[fboId].get_depth_buffer();
    SL_GeneralColor outColor = sl_match_color_for_type(pColorBuf.type, color);

    union
    {
        double d;
        float f;
        ls::math::half h;
    } depthVal;

    switch (pDepth.bytesPerTexel)
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

    mProcessors.run_clear_processors(&outColor.color, &depthVal, &pColorBuf, &pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (2 attachments)
--------------------------------------*/
void SL_Context::clear_framebuffer(size_t fboId, const std::array<unsigned, 2>& bufferIndices, const std::array<ls::math::vec4_t<double>, 2>& colors, double depth) noexcept
{
    SL_TextureView& pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SL_TextureView*, 2> buffers{
        &mFbos[fboId].get_color_buffer(bufferIndices[0]),
        &mFbos[fboId].get_color_buffer(bufferIndices[1])
    };

    SL_GeneralColor tempColors[2] = {
        sl_match_color_for_type(buffers[0]->type, colors[0]),
        sl_match_color_for_type(buffers[1]->type, colors[1])
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

    switch (pDepth.bytesPerTexel)
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

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, &pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (&3 attachments)
--------------------------------------*/
void SL_Context::clear_framebuffer(size_t fboId, const std::array<unsigned, 3>& bufferIndices, const std::array<ls::math::vec4_t<double>, 3>& colors, double depth) noexcept
{
    SL_TextureView& pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SL_TextureView*, 3> buffers{
        &mFbos[fboId].get_color_buffer(bufferIndices[0]),
        &mFbos[fboId].get_color_buffer(bufferIndices[1]),
        &mFbos[fboId].get_color_buffer(bufferIndices[2])
    };

    SL_GeneralColor tempColors[3] = {
        sl_match_color_for_type(buffers[0]->type, colors[0]),
        sl_match_color_for_type(buffers[1]->type, colors[1]),
        sl_match_color_for_type(buffers[2]->type, colors[2])
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

    switch (pDepth.bytesPerTexel)
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

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, &pDepth);
}



/*--------------------------------------
 * Clear a framebuffer (4 attachments)
--------------------------------------*/
void SL_Context::clear_framebuffer(size_t fboId, const std::array<unsigned, 4>& bufferIndices, const std::array<ls::math::vec4_t<double>, 4>& colors, double depth) noexcept
{
    SL_TextureView& pDepth = mFbos[fboId].get_depth_buffer();

    std::array<SL_TextureView*, 4> buffers{
        &mFbos[fboId].get_color_buffer(bufferIndices[0]),
        &mFbos[fboId].get_color_buffer(bufferIndices[1]),
        &mFbos[fboId].get_color_buffer(bufferIndices[2]),
        &mFbos[fboId].get_color_buffer(bufferIndices[3])
    };

    SL_GeneralColor tempColors[4] = {
        sl_match_color_for_type(buffers[0]->type, colors[0]),
        sl_match_color_for_type(buffers[1]->type, colors[1]),
        sl_match_color_for_type(buffers[2]->type, colors[2]),
        sl_match_color_for_type(buffers[3]->type, colors[3])
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

    switch (pDepth.bytesPerTexel)
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

    mProcessors.run_clear_processors(outColors, &depthVal, buffers, &pDepth);
}



/*--------------------------------------
 * Retrieve the number of threads
--------------------------------------*/
unsigned SL_Context::num_threads() const noexcept
{
    return mProcessors.concurrency();
}



/*--------------------------------------
 * Set the number of threads
--------------------------------------*/
unsigned SL_Context::num_threads(unsigned inNumThreads) noexcept
{
    return mProcessors.concurrency(inNumThreads);
}
