
#ifndef SR_SHADER_PROCESSOR_HPP
#define SR_SHADER_PROCESSOR_HPP

#include <cstdint>

#include "soft_render/SR_BlitProcesor.hpp"
#include "soft_render/SR_FragmentProcessor.hpp"
#include "soft_render/SR_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SR_ShaderType : uint8_t
{
    SR_VERTEX_SHADER,
    SR_FRAGMENT_SHADER,
    SR_BLIT_SHADER
};



struct SR_BinCounter
{
    uint32_t count;
    uint8_t padding[128];
};



struct SR_BinCounterAtomic
{
    std::atomic_int32_t count;
    uint8_t padding[128];
};



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex & fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SR_ShaderProcessor
{
    SR_ShaderType mType; // 32 bits

    // 2128 bits
    union
    {
        SR_VertexProcessor mVertProcessor;
        SR_FragmentProcessor mFragProcessor;
        SR_BlitProcessor mBlitter;
    };

    // 2144 bits (268 bytes), padding not included

    ~SR_ShaderProcessor() noexcept = default;

    SR_ShaderProcessor() noexcept;

    SR_ShaderProcessor(const SR_ShaderProcessor&) noexcept;

    SR_ShaderProcessor(SR_ShaderProcessor&&) noexcept;

    SR_ShaderProcessor& operator=(const SR_ShaderProcessor&) noexcept;

    SR_ShaderProcessor& operator=(SR_ShaderProcessor&&) noexcept;

    void operator()() noexcept;
};



/*--------------------------------------
 * Task Execution
--------------------------------------*/
inline void SR_ShaderProcessor::operator()() noexcept
{
    switch (mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor.execute();
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor.execute();
            break;

        case SR_BLIT_SHADER:
            mBlitter.execute();
            break;
    }
}



#endif /* SR_SHADER_PROCESSOR_HPP */
