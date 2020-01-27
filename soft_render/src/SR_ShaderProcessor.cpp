
#include "soft_render/SR_ShaderProcessor.hpp"



/*--------------------------------------
 * Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor() noexcept :
    mType{SR_VERTEX_SHADER},
    mVertProcessor()
{}



/*--------------------------------------
 * Copy Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor(const SR_ShaderProcessor& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SR_BLIT_SHADER:
            mBlitter = sp.mBlitter;
    }
}



/*--------------------------------------
 * Move Constructor
--------------------------------------*/
SR_ShaderProcessor::SR_ShaderProcessor(SR_ShaderProcessor&& sp) noexcept :
    mType{sp.mType}
{
    switch (sp.mType)
    {
        case SR_VERTEX_SHADER:
            mVertProcessor = sp.mVertProcessor;
            break;

        case SR_FRAGMENT_SHADER:
            mFragProcessor = sp.mFragProcessor;
            break;

        case SR_BLIT_SHADER:
            mBlitter = sp.mBlitter;
    }
}



/*--------------------------------------
 * Copy Operator
--------------------------------------*/
SR_ShaderProcessor& SR_ShaderProcessor::operator=(const SR_ShaderProcessor& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SR_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SR_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SR_BLIT_SHADER:
                mBlitter = sp.mBlitter;
        }
    }

    return *this;
}



/*--------------------------------------
 * Move Operator
--------------------------------------*/
SR_ShaderProcessor& SR_ShaderProcessor::operator=(SR_ShaderProcessor&& sp) noexcept
{
    if (this != &sp)
    {
        mType = sp.mType;

        switch (sp.mType)
        {
            case SR_VERTEX_SHADER:
                mVertProcessor = sp.mVertProcessor;
                break;

            case SR_FRAGMENT_SHADER:
                mFragProcessor = sp.mFragProcessor;
                break;

            case SR_BLIT_SHADER:
                mBlitter = sp.mBlitter;
        }
    }

    return *this;
}
