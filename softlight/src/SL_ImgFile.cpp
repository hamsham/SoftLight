/*
 * File:   ImageResource.cpp
 * Author: Miles Lacey
 *
 * Created on February 2, 2014, 1:42 PM
 */

#include <utility> // std::move

#include "lightsky/setup/Compiler.h"

#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "softlight/SL_Geometry.hpp" // SL_DataType
#include "softlight/SL_ImgFile.hpp"

// Ignore errors regarding different code page mismatches
#ifdef LS_COMPILER_MSC
    #pragma warning(push)
    #pragma warning(disable:4828)
#endif

#include <FreeImage.h>

#ifdef LS_COMPILER_MSC
    #pragma warning(pop)
#endif



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
SL_DataType get_bitmap_size(FIBITMAP* pImg)
{
    // Get the data mType of the image. Convert to an internal format
    const int storageType = FreeImage_GetImageType(pImg);
    SL_DataType dataType = VERTEX_DATA_BYTE;

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



SL_ColorDataType get_pixel_format(FIBITMAP* pImg, unsigned bpp, bool* success)
{
    LS_LOG_MSG("\tImage Bits Per Pixel: ", bpp);
    *success = true;

    // Get the data mType of the image. Convert to an internal format
    const FREE_IMAGE_TYPE dataType = FreeImage_GetImageType(pImg);

    if (dataType == FIT_BITMAP)
    {
        LS_LOG_MSG("\t8-bit Image");
        switch (bpp)
        {
            case 8: return SL_COLOR_R_8U;
            case 16: return SL_COLOR_RG_8U;
            case 24: return SL_COLOR_RGB_8U;
            case 32: return SL_COLOR_RGBA_8U;
        }
    }
    else if (dataType == FIT_INT16 || dataType == FIT_UINT16)
    {
        LS_LOG_MSG("\t16-bit Image");
        switch (bpp)
        {
            case 16: return SL_COLOR_R_16U;
            case 32: return SL_COLOR_RG_16U;
            case 48: return SL_COLOR_RGB_16U;
            case 64: return SL_COLOR_RGBA_16U;
        }
    }
    else if (dataType == FIT_INT32 || dataType == FIT_UINT32)
    {
        LS_LOG_MSG("\t32-bit Image");
        switch (bpp)
        {
            case 32: return SL_COLOR_R_32U;
            case 64: return SL_COLOR_RG_32U;
            case 96: return SL_COLOR_RGB_32U;
            case 128: return SL_COLOR_RGBA_32U;
        }
    }
    else if (dataType == FIT_FLOAT)
    {
        LS_LOG_MSG("\tFloat Image");
        switch (bpp)
        {
            case 32: return SL_COLOR_R_FLOAT;
            case 64: return SL_COLOR_RG_FLOAT;
            case 96: return SL_COLOR_RGB_FLOAT;
            case 128: return SL_COLOR_RGBA_FLOAT;
        }
    }
    else if (dataType == FIT_RGB16)
    {
        LS_LOG_MSG("\tRGB16 Image");
        return SL_COLOR_RGB_16U;
    }
    else if (dataType == FIT_RGBA16)
    {
        LS_LOG_MSG("\tRGBA16 Image");
        return SL_COLOR_RGBA_16U;
    }
    else if (dataType == FIT_RGBF)
    {
        LS_LOG_MSG("\tRGB_F Image");
        return SL_COLOR_RGB_FLOAT;
    }
    else if (dataType == FIT_RGBAF)
    {
        LS_LOG_MSG("\tRGBA_F Image");
        return SL_COLOR_RGBA_FLOAT;
    }

    *success = false;
    return SL_COLOR_RGBA_8U;
}



FREE_IMAGE_TYPE sl_color_to_freeimage(SL_ColorDataType type)
{
    LS_LOG_MSG("\tImage Bits Per Pixel: ", sl_bytes_per_color(type)*CHAR_BIT);

    switch (type)
    {
        case SL_COLOR_R_8U:
        case SL_COLOR_RG_8U:
        case SL_COLOR_RGB_8U:
        case SL_COLOR_RGBA_8U:
            return FIT_BITMAP;

        case SL_COLOR_R_16U:
        case SL_COLOR_RG_16U:
            return FIT_UINT16;

        case SL_COLOR_RGB_16U:
            return FIT_RGB16;

        case SL_COLOR_RGBA_16U:
            return FIT_RGBA16;

        case SL_COLOR_R_32U:
        case SL_COLOR_RG_32U:
        case SL_COLOR_RGB_32U:
        case SL_COLOR_RGBA_32U:
            return FIT_UINT32;

        case SL_COLOR_R_64U:
        case SL_COLOR_RG_64U:
        case SL_COLOR_RGB_64U:
        case SL_COLOR_RGBA_64U:
            break;

        case SL_COLOR_R_FLOAT:
        case SL_COLOR_RG_FLOAT:
            return FIT_FLOAT;

        case SL_COLOR_RGB_FLOAT:
            return FIT_RGBF;

        case SL_COLOR_RGBA_FLOAT:
            return FIT_RGBAF;

        case SL_COLOR_R_DOUBLE:
        case SL_COLOR_RG_DOUBLE:
        case SL_COLOR_RGB_DOUBLE:
        case SL_COLOR_RGBA_DOUBLE:
            return FIT_DOUBLE;

        case SL_COLOR_RGB_565:
        case SL_COLOR_RGBA_5551:
        case SL_COLOR_RGBA_4444:
            break;
    }

    return FIT_UNKNOWN;
}



unsigned sl_r_mask_to_freeimage(SL_ColorDataType type)
{
    switch (type)
    {
        case SL_COLOR_R_8U:
        case SL_COLOR_RG_8U:
        case SL_COLOR_RGB_8U:
        case SL_COLOR_RGBA_8U:
            return 0x000000FF;

        case SL_COLOR_R_16U:
        case SL_COLOR_RG_16U:
        case SL_COLOR_RGB_16U:
        case SL_COLOR_RGBA_16U:
            return 0x0000FFFF;

        case SL_COLOR_R_32U:
        case SL_COLOR_RG_32U:
        case SL_COLOR_RGB_32U:
        case SL_COLOR_RGBA_32U:

        case SL_COLOR_R_FLOAT:
        case SL_COLOR_RG_FLOAT:
        case SL_COLOR_RGB_FLOAT:
        case SL_COLOR_RGBA_FLOAT:
            return 0xFFFFFFFF;

        default:
            break;
    }

    return 0;
}



unsigned sl_g_mask_to_freeimage(SL_ColorDataType type)
{
    switch (type)
    {
        case SL_COLOR_RG_8U:
        case SL_COLOR_RGB_8U:
        case SL_COLOR_RGBA_8U:
            return 0x000000FF;

        case SL_COLOR_RG_16U:
        case SL_COLOR_RGB_16U:
        case SL_COLOR_RGBA_16U:
            return 0x0000FFFF;

        case SL_COLOR_RG_32U:
        case SL_COLOR_RGB_32U:
        case SL_COLOR_RGBA_32U:

        case SL_COLOR_RG_FLOAT:
        case SL_COLOR_RGB_FLOAT:
        case SL_COLOR_RGBA_FLOAT:
            return 0xFFFFFFFF;

        default:
            break;
    }

    return 0;
}



unsigned sl_b_mask_to_freeimage(SL_ColorDataType type)
{
    switch (type)
    {
        case SL_COLOR_RGB_8U:
        case SL_COLOR_RGBA_8U:
            return 0x000000FF;

        case SL_COLOR_RGB_16U:
        case SL_COLOR_RGBA_16U:
            return 0x0000FFFF;

        case SL_COLOR_RGB_32U:
        case SL_COLOR_RGBA_32U:

        case SL_COLOR_RGB_FLOAT:
        case SL_COLOR_RGBA_FLOAT:
            return 0xFFFFFFFF;

        default:
            break;
    }

    return 0;
}



/*-----------------------------------------------------------------------------
    Image Resource Method Definitions
-----------------------------------------------------------------------------*/
/*-------------------------------------
    Constructor
-------------------------------------*/
SL_ImgFile::SL_ImgFile() :
    mImgData{nullptr},
    mDimens{0},
    mBpp{0},
    mFormat{SL_COLOR_RGB_DEFAULT}
{}


/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_ImgFile::SL_ImgFile(const SL_ImgFile& img)
{
    this->operator=(img);
}


/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_ImgFile::SL_ImgFile(SL_ImgFile&& img)
{
    this->operator=(std::move(img));
}


/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_ImgFile::~SL_ImgFile()
{
    unload();
}


/*-------------------------------------
 * Copy operator
-------------------------------------*/
SL_ImgFile& SL_ImgFile::operator=(const SL_ImgFile& img)
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
SL_ImgFile& SL_ImgFile::operator=(SL_ImgFile&& img)
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
    img.mFormat = SL_COLOR_RGB_DEFAULT;

    return *this;
}


/*-------------------------------------
 * Loading
-------------------------------------*/
SL_ImgFile::ImgStatus SL_ImgFile::load(const char* filename)
{
    LS_LOG_MSG("Attempting to load the image ", filename);
    unload();

    if (!filename || !filename[0])
    {
        LS_LOG_ERR("\tFailed to load an image as no filename was provided.\n");
        return ImgStatus::INVALID_FILE_NAME;
    }

    // Set FreeImage's error function
    FreeImage_SetOutputMessage(&print_img_load_error);

    // Determine the file mType that should be loaded
    FREE_IMAGE_FORMAT fileFormat = deduce_img_format(filename);

    if (fileFormat == FIF_UNKNOWN)
    {
        LS_LOG_ERR("\tUnable to determine the file mType for ", filename, ".\n");
        return ImgStatus::INVALID_FILE_TYPE;
    }

    if (FreeImage_FIFSupportsReading(fileFormat) == false)
    {
        LS_LOG_ERR(
            "\tSupport for the mType of file used by ", filename,
            " is not currently implemented.\n");
        return ImgStatus::UNSUPPORTED_FILE_TYPE;
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
        return ImgStatus::INTERNAL_ERROR;
    }

    const SL_DataType dataType = get_bitmap_size(fileData);
    if (dataType == VERTEX_DATA_INVALID)
    {
        LS_LOG_ERR('\t', filename, " contains an unsupported pixel format.\n");
        FreeImage_Unload(fileData);
        return ImgStatus::UNSUPPORTED_FORMAT;
    }

    bool success = true;
    this->mImgData = fileData;
    this->mDimens[0] = (int)FreeImage_GetWidth(fileData);
    this->mDimens[1] = (int)FreeImage_GetHeight(fileData);
    this->mDimens[2] = 1; // TODO
    this->mBpp = (unsigned)FreeImage_GetBPP(fileData);
    this->mFormat = get_pixel_format(fileData, this->mBpp, &success);

    if (success)
    {
        LS_LOG_MSG("\tSuccessfully loaded ", filename, ".\n");
        return ImgStatus::FILE_LOAD_SUCCESS;
    }

    LS_LOG_ERR('\t', filename, " contains an unsupported pixel format.\n");
    FreeImage_Unload(fileData);
    return ImgStatus::UNSUPPORTED_FORMAT;
}



/*-------------------------------------
 * Loading
-------------------------------------*/
SL_ImgFile::ImgStatus SL_ImgFile::load_memory_stream(const void* pImgBits, SL_ColorDataType type, unsigned w, unsigned h)
{
    LS_LOG_MSG("Importing image from memory.");
    unload();

    if (!pImgBits)
    {
        LS_LOG_ERR("\tFailed to load an image as no valid image data was provided.\n");
        return ImgStatus::INVALID_FILE_TYPE;
    }

    // Set FreeImage's error function
    FreeImage_SetOutputMessage(&print_img_load_error);

    // Determine the file mType that should be loaded
    FREE_IMAGE_TYPE fiType = sl_color_to_freeimage(type);

    if (fiType == FIT_UNKNOWN)
    {
        LS_LOG_ERR("\tUnable to convert the in-memory image from ", (int)type, " to a suitable FreeImage type.\n");
        return ImgStatus::INVALID_FILE_TYPE;
    }

    // Preliminary setup passed. Attempt to load the file data

    // Use some predefined image flags
    unsigned byteDepth = (unsigned)sl_bytes_per_color(type);
    unsigned bitDepth = byteDepth*CHAR_BIT;
    unsigned rMask = sl_r_mask_to_freeimage(type);
    unsigned gMask = sl_g_mask_to_freeimage(type);
    unsigned bMask = sl_b_mask_to_freeimage(type);
    FIBITMAP* const fileData = FreeImage_ConvertFromRawBitsEx(TRUE, (BYTE*)pImgBits, fiType, (int)w, (int)h, byteDepth*w, bitDepth, rMask, gMask, bMask, FALSE);

    if (!fileData)
    {
        LS_LOG_ERR("\tUnable to load an image from memory due to an internal library error.\n");
        return ImgStatus::INTERNAL_ERROR;
    }

    bool success = true;
    this->mImgData = fileData;
    this->mDimens[0] = (int)FreeImage_GetWidth(fileData);
    this->mDimens[1] = (int)FreeImage_GetHeight(fileData);
    this->mDimens[2] = 1; // TODO
    this->mBpp = (unsigned)FreeImage_GetBPP(fileData);
    this->mFormat = get_pixel_format(fileData, this->mBpp, &success);

    if (success)
    {
        LS_LOG_MSG("\tSuccessfully loaded a memory stream.\n");
        return ImgStatus::FILE_LOAD_SUCCESS;
    }

    LS_LOG_ERR("\tImage memory stream contains an unsupported pixel format.\n");
    FreeImage_Unload(fileData);
    return ImgStatus::UNSUPPORTED_FORMAT;
}


/*-------------------------------------
 * Unloading
-------------------------------------*/
void SL_ImgFile::unload()
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
    mFormat = SL_COLOR_RGB_DEFAULT;
}


/*-------------------------------------
 * saving
-------------------------------------*/
bool SL_ImgFile::save(const char* filename, SL_ImgFileType format) const
{
    if (!mImgData)
    {
        return false;
    }

    FREE_IMAGE_FORMAT fiFormat = FIF_PNG;
    int flags = 0;

    switch (format)
    {
        case SL_ImgFileType::IMG_FILE_BMP: fiFormat = FIF_BMP;
            flags = BMP_SAVE_RLE;
            break;

        case SL_ImgFileType::IMG_FILE_EXR: fiFormat = FIF_EXR;
            break;

        case SL_ImgFileType::IMG_FILE_GIF: fiFormat = FIF_GIF;
            break;

        case SL_ImgFileType::IMG_FILE_HDR: fiFormat = FIF_HDR;
            break;

        case SL_ImgFileType::IMG_FILE_ICO: fiFormat = FIF_ICO;
            break;

        case SL_ImgFileType::IMG_FILE_JPG: fiFormat = FIF_JPEG;
            flags = JPEG_QUALITYSUPERB | JPEG_OPTIMIZE;
            break;

        case SL_ImgFileType::IMG_FILE_J2K: fiFormat = FIF_J2K;
            break;

        case SL_ImgFileType::IMG_FILE_PNG: fiFormat = FIF_PNG;
            flags = PNG_Z_DEFAULT_COMPRESSION;
            break;

        case SL_ImgFileType::IMG_FILE_PPM: fiFormat = FIF_PPM;
            break;

        case SL_ImgFileType::IMG_FILE_TGA: fiFormat = FIF_TARGA;
            flags = TARGA_SAVE_RLE;
            break;

        case SL_ImgFileType::IMG_FILE_TIF: fiFormat = FIF_TIFF;
            flags = TIFF_DEFLATE;
            break;

        //case SL_ImgFileType::IMG_FILE_WBP: fiFormat = FIF_WEBP;
        //    break;

        case SL_ImgFileType::IMG_FILE_XPM: fiFormat = FIF_XPM;
            break;

        default: fiFormat = FIF_PNG;
            flags = PNG_Z_BEST_COMPRESSION;
            break;
    }

    return 0 != FreeImage_Save(fiFormat, mImgData, filename, flags);
}


/*-------------------------------------
 * Get the data stored in pData
-------------------------------------*/
const void* SL_ImgFile::data() const
{
    return (const void*)FreeImage_GetBits(mImgData);
}
