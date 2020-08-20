
#ifndef SL_SHADER_PROCESSOR_HPP
#define SL_SHADER_PROCESSOR_HPP

#include <cstdint>

#include "softlight/SL_BlitProcesor.hpp"
#include "softlight/SL_ClearProcesor.hpp"
#include "softlight/SL_FragmentProcessor.hpp"
#include "softlight/SL_VertexProcessor.hpp"



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SL_ShaderType : uint8_t
{
    SL_VERTEX_SHADER,
    SL_FRAGMENT_SHADER,
    SL_BLIT_SHADER,
    SL_CLEAR_SHADER
};



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex & fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SL_ShaderProcessor
{
    SL_ShaderType mType; // 32 bits

    // 2128 bits
    union
    {
        SL_VertexProcessor mVertProcessor;
        SL_FragmentProcessor mFragProcessor;
        SL_BlitProcessor mBlitter;
        SL_ClearProcessor mClear;
    };

    // 2144 bits (268 bytes), padding not included

    ~SL_ShaderProcessor() noexcept = default;

    SL_ShaderProcessor() noexcept;

    SL_ShaderProcessor(const SL_ShaderProcessor&) noexcept;

    SL_ShaderProcessor(SL_ShaderProcessor&&) noexcept;

    SL_ShaderProcessor& operator=(const SL_ShaderProcessor&) noexcept;

    SL_ShaderProcessor& operator=(SL_ShaderProcessor&&) noexcept;

    void operator()() noexcept;
};



/*--------------------------------------
 * Task Execution
--------------------------------------*/
inline void SL_ShaderProcessor::operator()() noexcept
{
    switch (mType)
    {
        case SL_VERTEX_SHADER:
            mVertProcessor.execute();
            break;

        case SL_FRAGMENT_SHADER:
            mFragProcessor.execute();
            break;

        case SL_BLIT_SHADER:
            mBlitter.execute();
            break;

        case SL_CLEAR_SHADER:
            mClear.execute();
            break;
    }
}



#endif /* SL_SHADER_PROCESSOR_HPP */
