
#include "softlight/SL_ShaderProcessor.hpp"



/*--------------------------------------
 * Constructor
--------------------------------------*/
SL_ShaderProcessor::SL_ShaderProcessor() noexcept :
    mType{SL_VERTEX_SHADER},
    mVertProcessor()
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SL_ShaderProcessor::SL_ShaderProcessor(const SL_ShaderProcessor& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SL_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SL_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SL_BLIT_SHADER:
            mBlitter = sp.mBlitter;

        case SL_CLEAR_SHADER:
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
        case SL_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SL_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SL_BLIT_SHADER:
            mBlitter = sp.mBlitter;

        case SL_CLEAR_SHADER:
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
            case SL_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SL_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SL_BLIT_SHADER:
                mBlitter = sp.mBlitter;

            case SL_CLEAR_SHADER:
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
            case SL_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SL_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SL_BLIT_SHADER:
                mBlitter = sp.mBlitter;

            case SL_CLEAR_SHADER:
                mClear = sp.mClear;
        }
    }

    return *this;
}
