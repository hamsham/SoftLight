/*
 * File:   ImageResource.cpp
 * Author: Miles Lacey
 *
 * Created on February 2, 2014, 1:42 PM
 */

#include <utility> // std::move

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "soft_render/SR_ImgFile.hpp"

#include <FreeImage.h>


/*-----------------------------------------------------------------------------
    Utility/Helper Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * FreeImage error handler callback
-------------------------------------*/
#ifndef NDEBUG

void print_img_load_error(FREE_IMAGE_FORMAT fif, const char* msg)
{
    LS_LOG_ERR(
        "\tAn image file error has occurred:",
        "\n\tFormat: ", FreeImage_GetFormatFromFIF(fif),
        "\n\t", msg, "."
    );
}

#else
void print_img_load_error(FREE_IMAGE_FORMAT, const char*) {
}
#endif


/*-------------------------------------
 * Deduce an image's file format
-------------------------------------*/
FREE_IMAGE_FORMAT deduce_img_format(const char* filename)
{
    FREE_IMAGE_FORMAT outFormat = FreeImage_GetFileType(filename, 0);
    if (outFormat == FIF_UNKNOWN)
    {
        outFormat = FreeImage_GetFIFFromFilename(filename);
    }
    return outFormat;
}


/*-------------------------------------
 * Predefined image flags
-------------------------------------*/
int get_img_flags(FREE_IMAGE_FORMAT inFormat)
{
    switch (inFormat)
    {
        case FIF_JPEG: return JPEG_ACCURATE;
        case FIF_TARGA: return TARGA_LOAD_RGB888;
        case FIF_ICO: return ICO_MAKEALPHA;
        default: return 0;
    }
}


/*-------------------------------------
 * Get an image's pixel format, combined with its bits per pixel
-------------------------------------*/
SR_DataType get_bitmap_size(FIBITMAP* pImg)
{
    // Get the data mType of the image. Convert to an internal format
    const int storageType = FreeImage_GetImageType(pImg);
    SR_DataType dataType = VERTEX_DATA_BYTE;

    if (storageType == FIT_UNKNOWN
        || storageType == FIT_DOUBLE
        || storageType == FIT_COMPLEX)
    {
        return VERTEX_DATA_INVALID;
    }

    switch (storageType)
    {
        // n-bit char
        case FIT_BITMAP:
            LS_LOG_MSG("\tImage pixel mType: BYTE");
            dataType = VERTEX_DATA_BYTE;
            break;

            // 16-bit short
        case FIT_INT16:
            dataType = VERTEX_DATA_SHORT;
            LS_LOG_MSG("\tImage pixel mType: SHORT");
            break;

        case FIT_UINT16:
            dataType = VERTEX_DATA_SHORT;
            LS_LOG_MSG("\tImage pixel mType: UNSIGNED SHORT");
            break;

            // 32-bit int
        case FIT_INT32:
            dataType = VERTEX_DATA_INT;
            LS_LOG_MSG("\tImage pixel mType: INT");
            break;

        case FIT_UINT32:
            dataType = VERTEX_DATA_INT;
            LS_LOG_MSG("\tImage pixel mType: UNSIGNED INT");
            break;

            // 96-bit float
        case FIT_RGBF:
            // 128-bit float
        case FIT_RGBAF:
            dataType = VERTEX_DATA_FLOAT;
            LS_LOG_MSG("\tImage pixel mType: FLOAT");
            break;

            // unknown
        default:
            dataType = VERTEX_DATA_INVALID;
            LS_LOG_MSG("\tImage pixel mType: INVALID");
            break;
    }

    return dataType;
}



SR_ColorDataType get_pixel_format(FIBITMAP* pImg, unsigned bpp)
{
    LS_LOG_MSG("\tImage Bits Per Pixel: ", bpp);

    // Get the data mType of the image. Convert to an internal format
    const FREE_IMAGE_TYPE dataType = FreeImage_GetImageType(pImg);

    if (dataType == FIT_BITMAP)
    {
        LS_LOG_MSG("\t8-bit Image");
        switch (bpp)
        {
            case 8: return SR_COLOR_R_8U;
            case 16: return SR_COLOR_RG_8U;
            case 24: return SR_COLOR_RGB_8U;
            case 32: return SR_COLOR_RGBA_8U;
        }
    }
    else if (dataType == FIT_INT16 || dataType == FIT_UINT16)
    {
        LS_LOG_MSG("\t16-bit Image");
        switch (bpp)
        {
            case 16: return SR_COLOR_R_16U;
            case 32: return SR_COLOR_RG_16U;
            case 48: return SR_COLOR_RGB_16U;
            case 64: return SR_COLOR_RGBA_16U;
        }
    }
    else if (dataType == FIT_INT32 || dataType == FIT_UINT32)
    {
        LS_LOG_MSG("\t32-bit Image");
        switch (bpp)
        {
            case 32: return SR_COLOR_R_32U;
            case 64: return SR_COLOR_RG_32U;
            case 96: return SR_COLOR_RGB_32U;
            case 128: return SR_COLOR_RGBA_32U;
        }
    }
    else if (dataType == FIT_FLOAT)
    {
        LS_LOG_MSG("\tFloat Image");
        switch (bpp)
        {
            case 32: return SR_COLOR_R_FLOAT;
            case 64: return SR_COLOR_RG_FLOAT;
            case 96: return SR_COLOR_RGB_FLOAT;
            case 128: return SR_COLOR_RGBA_FLOAT;
        }
    }
    else if (dataType == FIT_RGB16)
    {
        LS_LOG_MSG("\tRGB16 Image");
        return SR_COLOR_RGB_16U;
    }
    else if (dataType == FIT_RGBA16)
    {
        LS_LOG_MSG("\tRGBA16 Image");
        return SR_COLOR_RGBA_16U;
    }
    else if (dataType == FIT_RGBF)
    {
        LS_LOG_MSG("\tRGB_F Image");
        return SR_COLOR_RGB_FLOAT;
    }
    else if (dataType == FIT_RGBAF)
    {
        LS_LOG_MSG("\tRGBA_F Image");
        return SR_COLOR_RGBA_FLOAT;
    }

    return SR_COLOR_INVALID;
}



/*-----------------------------------------------------------------------------
    Image Resource Method Definitions
-----------------------------------------------------------------------------*/
/*-------------------------------------
    Constructor
-------------------------------------*/
SR_ImgFile::SR_ImgFile() :
    mImgData{nullptr},
    mDimens{0},
    mBpp{0},
    mFormat{SR_COLOR_RGB_DEFAULT}
{}


/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SR_ImgFile::SR_ImgFile(const SR_ImgFile& img)
{
    this->operator=(img);
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_ImgFile::SR_ImgFile(SR_ImgFile&& img)
{
    this->operator=(std::move(img));
}


/*-------------------------------------
 * Destructor
-------------------------------------*/
SR_ImgFile::~SR_ImgFile()
{
    unload();
}


/*-------------------------------------
 * Copy operator
-------------------------------------*/
SR_ImgFile& SR_ImgFile::operator=(const SR_ImgFile& img)
{
    unload();

    // nothing loaded in the other image buffer.
    if (!img.mImgData)
    {
        return *this;
    }

    mImgData = FreeImage_Clone(img.mImgData);

    // Fail brilliantly if we're out of memory
    LS_ASSERT(this->mImgData != nullptr);

    mDimens[0] = img.mDimens[0];
    mDimens[1] = img.mDimens[1];
    mDimens[2] = img.mDimens[2];
    mBpp = img.mBpp;
    mFormat = img.mFormat;

    return *this;
}


/*-------------------------------------
 * Move operator
-------------------------------------*/
SR_ImgFile& SR_ImgFile::operator=(SR_ImgFile&& img)
{
    // Resolve movement for the base class members first
    unload();

    mImgData = img.mImgData;
    img.mImgData = nullptr;

    mDimens[0] = img.mDimens[0];
    mDimens[1] = img.mDimens[1];
    mDimens[2] = img.mDimens[2];
    img.mDimens[0] = 0;
    img.mDimens[1] = 0;
    img.mDimens[2] = 0;

    mBpp = img.mBpp;
    img.mBpp = 0;

    mFormat = img.mFormat;
    img.mFormat = SR_COLOR_RGB_DEFAULT;

    return *this;
}


/*-------------------------------------
 * Loading
-------------------------------------*/
SR_ImgFile::img_status_t SR_ImgFile::load(const char* filename)
{
    LS_LOG_MSG("Attempting to load the image ", filename);
    unload();

    if (!filename || !filename[0])
    {
        LS_LOG_ERR("\tFailed to load an image as no filename was provided.\n");
        return img_status_t::INVALID_FILE_NAME;
    }

    // Set FreeImage's error function
    FreeImage_SetOutputMessage(&print_img_load_error);

    // Determine the file mType that should be loaded
    FREE_IMAGE_FORMAT fileFormat = deduce_img_format(filename);

    if (fileFormat == FIF_UNKNOWN)
    {
        LS_LOG_ERR("\tUnable to determine the file mType for ", filename, ".\n");
        return img_status_t::INVALID_FILE_TYPE;
    }

    if (FreeImage_FIFSupportsReading(fileFormat) == false)
    {
        LS_LOG_ERR(
            "\tSupport for the mType of file used by ", filename,
            " is not currently implemented.\n");
        return img_status_t::UNSUPPORTED_FILE_TYPE;
    }

    // Preliminary setup passed. Attempt to load the file data

    // Use some predefined image flags
    const int fileFlags = get_img_flags(fileFormat);
    FIBITMAP* const fileData = FreeImage_Load(fileFormat, filename, fileFlags);

    if (!fileData)
    {
        LS_LOG_ERR(
            "\tUnable to load the image ", filename,
            " due to an internal library error.\n");
        return img_status_t::INTERNAL_ERROR;
    }

    const SR_DataType dataType = get_bitmap_size(fileData);
    if (dataType == VERTEX_DATA_INVALID)
    {
        LS_LOG_ERR('\t', filename, " contains an unsupported pixel format.\n");
        FreeImage_Unload(fileData);
        return img_status_t::UNSUPPORTED_FORMAT;
    }

    this->mImgData = fileData;
    this->mDimens[0] = (int)FreeImage_GetWidth(fileData);
    this->mDimens[1] = (int)FreeImage_GetHeight(fileData);
    this->mDimens[2] = 1; // TODO
    this->mBpp = (unsigned)FreeImage_GetBPP(fileData);
    this->mFormat = get_pixel_format(fileData, this->mBpp);

    LS_LOG_MSG("\tSuccessfully loaded ", filename, ".\n");

    return img_status_t::FILE_LOAD_SUCCESS;
}


/*-------------------------------------
 * Unloading
-------------------------------------*/
void SR_ImgFile::unload()
{
    if (!mImgData)
    {
        return;
    }

    FreeImage_Unload(mImgData);

    mImgData = nullptr;
    mDimens[0] = 0;
    mDimens[1] = 0;
    mDimens[2] = 0;
    mBpp = 0;
    mFormat = SR_COLOR_RGB_DEFAULT;
}


/*-------------------------------------
 * saving
-------------------------------------*/
bool SR_ImgFile::save(const char* filename, img_file_t format) const
{
    if (!mImgData)
    {
        return false;
    }

    FREE_IMAGE_FORMAT fiFormat = FIF_PNG;

    switch (format)
    {
        case img_file_t::IMG_FILE_BMP: fiFormat = FIF_BMP;
            break;
        case img_file_t::IMG_FILE_EXR: fiFormat = FIF_EXR;
            break;
        case img_file_t::IMG_FILE_GIF: fiFormat = FIF_GIF;
            break;
        case img_file_t::IMG_FILE_HDR: fiFormat = FIF_HDR;
            break;
        case img_file_t::IMG_FILE_ICO: fiFormat = FIF_ICO;
            break;
        case img_file_t::IMG_FILE_JPG: fiFormat = FIF_JPEG;
            break;
        case img_file_t::IMG_FILE_J2K: fiFormat = FIF_J2K;
            break;
        case img_file_t::IMG_FILE_PNG: fiFormat = FIF_PNG;
            break;
        case img_file_t::IMG_FILE_PPM: fiFormat = FIF_PPM;
            break;
        case img_file_t::IMG_FILE_TGA: fiFormat = FIF_TARGA;
            break;
        case img_file_t::IMG_FILE_TIF: fiFormat = FIF_TIFF;
            break;
        //case img_file_t::IMG_FILE_WBP: fiFormat = FIF_WEBP;
        //    break;
        case img_file_t::IMG_FILE_XPM: fiFormat = FIF_XPM;
            break;
        default: fiFormat = FIF_PNG;
            break;
    }

    return 0 != FreeImage_Save(fiFormat, mImgData, filename);
}


/*-------------------------------------
 * Get the data stored in pData
-------------------------------------*/
const void* SR_ImgFile::data() const
{
    return (const void*)FreeImage_GetBits(mImgData);
}
