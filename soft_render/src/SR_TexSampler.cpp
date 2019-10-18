
#include "soft_render/SR_TexSampler.hpp"



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler::~SR_TexSampler() noexcept
{
}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler::SR_TexSampler() noexcept :
    mWrapping{SR_TEXTURE_WRAP_DEFAULT},
    mWidthi{0},
    mHeighti{0},
    mDepthi{0},
    mWidthf{0},
    mHeightf{0},
    mDepthf{0},
    mBytesPerTexel{0},
    mType{SR_ColorDataType::SR_COLOR_INVALID},
    mNumChannels{0},
    mTexData{nullptr}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler::SR_TexSampler(const SR_Texture& t) noexcept :
    mWrapping{t.wrap_mode()},
    mWidthi{t.width()},
    mHeighti{t.height()},
    mDepthi{t.depth()},
    mWidthf{ls::math::fixed_cast<SR_Texture::fixed_type>(t.width())},
    mHeightf{ls::math::fixed_cast<SR_Texture::fixed_type>(t.height())},
    mDepthf{ls::math::fixed_cast<SR_Texture::fixed_type>(t.depth())},
    mBytesPerTexel{t.bpp()},
    mType{t.type()},
    mNumChannels{t.channels()},
    mTexData{t.data()}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler::SR_TexSampler(const SR_TexSampler& s) noexcept :
    mWrapping{s.mWrapping},
    mWidthi{s.mWidthi},
    mHeighti{s.mHeighti},
    mDepthi{s.mDepthi},
    mWidthf{s.mWidthf},
    mHeightf{s.mHeightf},
    mDepthf{s.mDepthf},
    mBytesPerTexel{s.mBytesPerTexel},
    mType{s.mType},
    mNumChannels{s.mNumChannels},
    mTexData{s.mTexData}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler::SR_TexSampler(SR_TexSampler&& s) noexcept :
    mWrapping{s.mWrapping},
    mWidthi{s.mWidthi},
    mHeighti{s.mHeighti},
    mDepthi{s.mDepthi},
    mWidthf{s.mWidthf},
    mHeightf{s.mHeightf},
    mDepthf{s.mDepthf},
    mBytesPerTexel{s.mBytesPerTexel},
    mType{s.mType},
    mNumChannels{s.mNumChannels},
    mTexData{s.mTexData}
{}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler& SR_TexSampler::operator=(const SR_TexSampler& s) noexcept
{
    mWrapping = s.mWrapping;
    mWidthi = s.mWidthi;
    mHeighti = s.mHeighti;
    mDepthi = s.mDepthi;
    mWidthf = s.mWidthf;
    mHeightf = s.mHeightf;
    mDepthf = s.mDepthf;
    mBytesPerTexel = s.mBytesPerTexel;
    mType = s.mType;
    mNumChannels = s.mNumChannels;
    mTexData = s.mTexData;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
SR_TexSampler& SR_TexSampler::operator=(SR_TexSampler&& s) noexcept
{
    mWrapping = s.mWrapping;
    mWidthi = s.mWidthi;
    mHeighti = s.mHeighti;
    mDepthi = s.mDepthi;
    mWidthf = s.mWidthf;
    mHeightf = s.mHeightf;
    mDepthf = s.mDepthf;
    mBytesPerTexel = s.mBytesPerTexel;
    mType = s.mType;
    mNumChannels = s.mNumChannels;
    mTexData = s.mTexData;

    return *this;
}



/*-------------------------------------
 *
-------------------------------------*/
void SR_TexSampler::init(const SR_Texture& t) noexcept
{
    mWrapping = t.wrap_mode();
    mWidthi = t.width();
    mHeighti = t.height();
    mDepthi = t.depth();
    mWidthf = ls::math::fixed_cast<SR_Texture::fixed_type>(t.width());
    mHeightf = ls::math::fixed_cast<SR_Texture::fixed_type>(t.height());
    mDepthf = ls::math::fixed_cast<SR_Texture::fixed_type>(t.depth());
    mBytesPerTexel = t.bpp();
    mType = t.type();
    mNumChannels = t.channels();
    mTexData = t.data();
}
