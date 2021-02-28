
#include "lightsky/utils/Assertions.h"

#include "softlight/SL_Mesh.hpp" // SL_RenderMode
#include "softlight/SL_ShaderProcessor.hpp"



/*--------------------------------------
 * Determine the correct processor for a render mode
--------------------------------------*/
SL_ShaderType sl_processor_type_for_draw_mode(SL_RenderMode drawMode) noexcept
{
    switch (drawMode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
            return SL_POINT_PROCESSOR;

        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
            return SL_LINE_PROCESSOR;

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
            return SL_TRI_PROCESSOR;

        default:
            LS_DEBUG_ASSERT(false);
            LS_UNREACHABLE();
    }

    return SL_TRI_PROCESSOR;
}



/*--------------------------------------
 * Destructor
--------------------------------------*/
SL_ShaderProcessor::~SL_ShaderProcessor() noexcept
{
}



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_ShaderProcessor::SL_ShaderProcessor() noexcept :
    mType{SL_TRI_PROCESSOR},
    mTriProcessor()
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_ShaderProcessor::SL_ShaderProcessor(const SL_ShaderProcessor& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SL_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SL_TRI_PROCESSOR:
            mTriProcessor = sp.mTriProcessor;
            break;

        case SL_LINE_PROCESSOR:
            mLineProcessor = sp.mLineProcessor;
            break;

        case SL_POINT_PROCESSOR:
            mPointProcessor = sp.mPointProcessor;
            break;

        case SL_BLIT_PROCESSOR:
            mBlitter = sp.mBlitter;

        case SL_CLEAR_PROCESSOR:
            mClear = sp.mClear;
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SL_ShaderProcessor::SL_ShaderProcessor(SL_ShaderProcessor&& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SL_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SL_TRI_PROCESSOR:
            mTriProcessor = sp.mTriProcessor;
            break;

        case SL_LINE_PROCESSOR:
            mLineProcessor = sp.mLineProcessor;
            break;

        case SL_POINT_PROCESSOR:
            mPointProcessor = sp.mPointProcessor;
            break;

        case SL_BLIT_PROCESSOR:
            mBlitter = sp.mBlitter;

        case SL_CLEAR_PROCESSOR:
            mClear = sp.mClear;
    }
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SL_ShaderProcessor& SL_ShaderProcessor::operator=(const SL_ShaderProcessor& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SL_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SL_TRI_PROCESSOR:
                mTriProcessor = sp.mTriProcessor;
                break;

            case SL_LINE_PROCESSOR:
                mLineProcessor = sp.mLineProcessor;
                break;

            case SL_POINT_PROCESSOR:
                mPointProcessor = sp.mPointProcessor;
                break;

            case SL_BLIT_PROCESSOR:
                mBlitter = sp.mBlitter;

            case SL_CLEAR_PROCESSOR:
                mClear = sp.mClear;
        }
    }

    return *this;
}



/*--------------------------------------
 * Move Operator
--------------------------------------*/
SL_ShaderProcessor& SL_ShaderProcessor::operator=(SL_ShaderProcessor&& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SL_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SL_TRI_PROCESSOR:
                mTriProcessor = sp.mTriProcessor;
                break;

            case SL_LINE_PROCESSOR:
                mLineProcessor = sp.mLineProcessor;
                break;

            case SL_POINT_PROCESSOR:
                mPointProcessor = sp.mPointProcessor;
                break;

            case SL_BLIT_PROCESSOR:
                mBlitter = sp.mBlitter;

            case SL_CLEAR_PROCESSOR:
                mClear = sp.mClear;
        }
    }

    return *this;
}



/*--------------------------------------
 * Deduce the correct rasterizer for a particular draw mode
--------------------------------------*/
SL_RasterProcessor* SL_ShaderProcessor::processor_for_draw_mode(SL_RenderMode drawMode) noexcept
{
    switch (drawMode)
    {
        case RENDER_MODE_POINTS:
        case RENDER_MODE_INDEXED_POINTS:
            return &mPointProcessor;

        case RENDER_MODE_LINES:
        case RENDER_MODE_INDEXED_LINES:
            return &mLineProcessor;

        case RENDER_MODE_TRIANGLES:
        case RENDER_MODE_INDEXED_TRIANGLES:
        case RENDER_MODE_TRI_WIRE:
        case RENDER_MODE_INDEXED_TRI_WIRE:
            return &mTriProcessor;

        default:
            LS_DEBUG_ASSERT(false);
            LS_UNREACHABLE();
    }

    return nullptr;
}
