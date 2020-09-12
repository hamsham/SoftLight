
#ifndef SL_IMG_FILE_HPP
#define SL_IMG_FILE_HPP

#include "lightsky/utils/Pointer.h"

#include "softlight/SL_Color.hpp" // SL_ColorDataType



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
struct FIBITMAP;



/**----------------------------------------------------------------------------
 * Enumerations for saving image files.
-----------------------------------------------------------------------------*/
enum class SL_ImgFileType {
    IMG_FILE_BMP,
    IMG_FILE_EXR,
    IMG_FILE_GIF,
    IMG_FILE_HDR,
    IMG_FILE_ICO,
    IMG_FILE_JPG,
    IMG_FILE_J2K,
    IMG_FILE_PNG, // default
    IMG_FILE_PPM,
    IMG_FILE_TGA,
    IMG_FILE_TIF,
    IMG_FILE_WBP,
    IMG_FILE_XPM
};



/**----------------------------------------------------------------------------
 * @brief A resource object to load image files.
 *
 * This class can load images using the FreeImage library, The pData member
 * contains raw texture data. It has been marked final until another otherwise
 * required.
 *
 * The inherited member "pData" will be reinterpreted as a pointer to a
 * FIBITMAP structure so as to make loading and saving easier to maintain.
-----------------------------------------------------------------------------*/
class SL_ImgFile final {

    // Public static interfaces
    public:
        enum ImgStatus {
            FILE_LOAD_SUCCESS       = 0,
            FILE_NOT_FOUND          = -1,
            INVALID_FILE_NAME       = -2,
            INVALID_FILE_TYPE       = -3,
            UNSUPPORTED_FILE_TYPE   = -4,
            UNSUPPORTED_FORMAT      = -5,
            INTERNAL_ERROR          = -6
        };


    private:
        FIBITMAP* mImgData;
        
        /**
         * imgSize represents the pixel width & height of a loaded image
         */
        size_t mDimens[3];

        /**
         * Number of bits per pixel in the image
         */
        unsigned mBpp;

        /**
         * @brief Data format of an image
         */
        SL_ColorDataType mFormat;

    public:
        /**
         * @brief Constructor
         */
        SL_ImgFile();

        /**
         * @brief Copy Constructor
         * 
         * Copies all image data from the input parameter into *this.
         * 
         * @param ib
         * A constant reference to another SL_ImgFile object which will have all
         * of its data copied into *this.
         */
        SL_ImgFile(const SL_ImgFile& ib);

        /**
         * @brief Move Operator
         *
         * Moves data from the source operand into *this. No copies are
         * performed.
         *
         * @param ib
         * An r-value reference to a temporary image resource object.
         */
        SL_ImgFile(SL_ImgFile&& ib);

        /**
         * @brief Destructor
         *
         * Calls "unload()" and releases all memory from *this.
         */
        ~SL_ImgFile();

        /**
         * @brief Copy Operator
         * 
         * Copies all image data from the input parameter into *this.
         * 
         * @param ib
         * A constant reference to another SL_ImgFile object which will have all
         * of its data copied into *this.
         * 
         * @return
         * A reference to *this.
         */
        SL_ImgFile& operator=(const SL_ImgFile& ib);

        /**
         * @brief Move Operator
         *
         * Moves all data from the source operand into *this. No copies are
         * performed.
         *
         * @param ib
         * An r-value reference to a temporary image resource object.
         *
         * @return a reference to *this.
         */
        SL_ImgFile& operator=(SL_ImgFile&& ib);
        
        /**
         * Retrieve the total number of bytes contained within *this object's
         * internal buffer.
         * 
         * @return An unsigned integral type, containing the number of bytes
         * used by *this to hold an image in memory. A value of 0u is returned
         * if no data is being managed by *this.
         */
        size_t num_bytes() const;

        /**
         * @brief Load an image file
         *
         * @param filename
         * A C-style string containing the relative path name to a file that
         * should be loadable into memory.
         *
         * @return FILE_LOAD_SUCCESS if the file was successfully loaded or an
         * error code from ImgStatus if not.
         */
        ImgStatus load(const char* filename);

        /**
         * @brief Load an image from memory
         *
         * @param pImgBits
         * A pointer to raw image data located in memory.
         *
         * @type
         * The color type of the image in memory.
         *
         * @return FILE_LOAD_SUCCESS if the file was successfully loaded or an
         * error code from ImgStatus if not.
         */
        ImgStatus load_memory_stream(const void* pImgBits, SL_ColorDataType type, unsigned w, unsigned h);

        /**
         * @brief Save an image file in a specific format
         *
         * @param filename
         * A C-style string containing the relative path name to a file that
         * should be saved to the computer.
         *
         * @param filetype
         * An SL_ImgFileType, representing the file format that should be used
         * when saving image data.
         *
         * @return true if the file was successfully saved. False if not.
         */
        bool save(const char* filename, SL_ImgFileType filetype = SL_ImgFileType::IMG_FILE_PNG) const;

        /**
         * @brief Unload
         *
         * Free all memory used by *this.
         */
        void unload();

        /**
         * Get the raw, loaded, image data contained within *this.
         * The underlying type has been cast from a FreeImage FIBITMAP*.
         *
         * @return a void pointer to the raw image bits.
         */
        const void* data() const;

        /**
         * Get the pixel size of the currently loaded image
         *
         * @return a 2D integer vector containing the width and height of the
         * loaded image, in pixels.
         */
        const size_t* size() const;

        /**
         * Get the width of the currently loaded image
         *
         * @return The width, in pixels, of the current image.
         */
        size_t width() const;

        /**
         * Get the height of the currently loaded image
         *
         * @return The height, in pixels, of the current image.
         */
        size_t height() const;

        /**
         * Get the depth of the currently loaded image
         *
         * @return The depth, in pixels, of the current image.
         */
        size_t depth() const;

        /**
         * Get the number of bits per pixel in the image.
         *
         * @return 0, 1, 2, 4, 8, 16, 24, 32, 48, 64, 96, or 128
         */
        unsigned bpp() const;

        /**
         * Get the image format.
         *
         * @return An enumeration containing image format information that can
         * be used when setting up textures.
         */
        SL_ColorDataType format() const;
};



/*-------------------------------------
 * Get the pixel size of the currently loaded image
-------------------------------------*/
inline size_t SL_ImgFile::num_bytes() const {
    const size_t bytesPerPixel = sl_bytes_per_color(mFormat);
    return mDimens[0] * mDimens[1] * mDimens[2] * bytesPerPixel;
}



/*-------------------------------------
 * Get the pixel size of the currently loaded image
-------------------------------------*/
inline const size_t* SL_ImgFile::size() const {
    return mDimens;
}



/*-------------------------------------
 * Image Width
-------------------------------------*/
inline size_t SL_ImgFile::width() const {
    return mDimens[0];
}



/*-------------------------------------
 * Image Height
-------------------------------------*/
inline size_t SL_ImgFile::height() const {
    return mDimens[1];
}



/*-------------------------------------
 * Image Depth
-------------------------------------*/
inline size_t SL_ImgFile::depth() const {
    return mDimens[2];
}



/*-------------------------------------
 * Get the number of bits per pixel in the image.
-------------------------------------*/
inline unsigned SL_ImgFile::bpp() const {
    return mBpp;
}



/*-------------------------------------
 * Get the current image format.
-------------------------------------*/
inline SL_ColorDataType SL_ImgFile::format() const {
    return mFormat;
}



#endif  /* SL_IMG_FILE_HPP */
