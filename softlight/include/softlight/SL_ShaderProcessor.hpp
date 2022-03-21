
#ifndef SL_SHADER_PROCESSOR_HPP
#define SL_SHADER_PROCESSOR_HPP

#include <cstdint>

#include "softlight/SL_BlitProcesor.hpp"
#include "softlight/SL_BlitCompressedProcesor.hpp"
#include "softlight/SL_ClearProcesor.hpp"
#include "softlight/SL_LineProcessor.hpp"
#include "softlight/SL_PointProcessor.hpp"
#include "softlight/SL_TriProcessor.hpp"



enum SL_RenderMode : uint32_t;



/*-----------------------------------------------------------------------------
 * Constants needed for shader operation
-----------------------------------------------------------------------------*/
enum SL_ShaderType : uint8_t
{
    SL_TRI_PROCESSOR,
    SL_LINE_PROCESSOR,
    SL_POINT_PROCESSOR,
    SL_BLIT_PROCESSOR,
    SL_BLIT_COMPRESSED_PROCESSOR,
    SL_CLEAR_PROCESSOR
};

SL_ShaderType sl_processor_type_for_draw_mode(SL_RenderMode drawMode) noexcept;



/*-----------------------------------------------------------------------------
 * Encapsulation of vertex & fragment processing on another thread.
-----------------------------------------------------------------------------*/
struct SL_ShaderProcessor
{
    SL_ShaderType mType; // 32 bits

    // 2128 bits
    union
    {
        SL_TriProcessor mTriProcessor;
        SL_LineProcessor mLineProcessor;
        SL_PointProcessor mPointProcessor;
        SL_BlitProcessor mBlitter;
        SL_BlitCompressedProcessor mBlitterCompressed;
        SL_ClearProcessor mClear;
    };

    // 2144 bits (268 bytes), padding not included

    ~SL_ShaderProcessor() noexcept;

    SL_ShaderProcessor() noexcept;

    SL_ShaderProcessor(const SL_ShaderProcessor&) noexcept;

    SL_ShaderProcessor(SL_ShaderProcessor&&) noexcept;

    SL_ShaderProcessor& operator=(const SL_ShaderProcessor&) noexcept;

    SL_ShaderProcessor& operator=(SL_ShaderProcessor&&) noexcept;

    SL_VertexProcessor* processor_for_draw_mode(SL_RenderMode drawMode) noexcept;

    void operator()() noexcept;
};



/*--------------------------------------
 * Task Execution
--------------------------------------*/
inline void SL_ShaderProcessor::operator()() noexcept
{
    switch (mType)
    {
        case SL_TRI_PROCESSOR:
            mTriProcessor.execute();
            break;

        case SL_LINE_PROCESSOR:
            mLineProcessor.execute();
            break;

        case SL_POINT_PROCESSOR:
            mPointProcessor.execute();
            break;

        case SL_BLIT_PROCESSOR:
            mBlitter.execute();
            break;

        case SL_BLIT_COMPRESSED_PROCESSOR:
            mBlitterCompressed.execute();
            break;

        case SL_CLEAR_PROCESSOR:
            mClear.execute();
            break;
    }
}



#endif /* SL_SHADER_PROCESSOR_HPP */
