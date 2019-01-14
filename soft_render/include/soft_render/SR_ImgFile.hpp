
#ifndef SR_IMG_FILE_HPP
#define SR_IMG_FILE_HPP

#include "lightsky/utils/Pointer.h"

#include "soft_render/SR_Color.hpp" // SR_ColorDataType
#include "soft_render/SR_Geometry.hpp" // SR_DataType



/*-----------------------------------------------------------------------------
 * Forward declarations
-----------------------------------------------------------------------------*/
struct FIBITMAP;



/**----------------------------------------------------------------------------
 * Enumerations for saving image files.
-----------------------------------------------------------------------------*/
enum class img_file_t {
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
class SR_ImgFile final {

    // Public static interfaces
    public:
        enum img_status_t {
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
        SR_ColorDataType mFormat;

    public:
        /**
         * @brief Constructor
         */
        SR_ImgFile();

        /**
         * @brief Copy Constructor
         * 
         * Copies all image data from the input parameter into *this.
         * 
         * @param ib
         * A constant reference to another SR_ImgFile object which will have all
         * of its data copied into *this.
         */
        SR_ImgFile(const SR_ImgFile& ib);

        /**
         * @brief Move Operator
         *
         * Moves data from the source operand into *this. No copies are
         * performed.
         *
         * @param ib
         * An r-value reference to a temporary image resource object.
         */
        SR_ImgFile(SR_ImgFile&& ib);

        /**
         * @brief Destructor
         *
         * Calls "unload()" and releases all memory from *this.
         */
        ~SR_ImgFile();

        /**
         * @brief Copy Operator
         * 
         * Copies all image data from the input parameter into *this.
         * 
         * @param ib
         * A constant reference to another SR_ImgFile object which will have all
         * of its data copied into *this.
         * 
         * @return
         * A reference to *this.
         */
        SR_ImgFile& operator=(const SR_ImgFile& ib);

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
        SR_ImgFile& operator=(SR_ImgFile&& ib);
        
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
         * @return true if the file was successfully loaded. False if not.
         */
        img_status_t load(const char* filename);

        /**
         * @brief Save an image file in a specific format
         *
         * @param filename
         * A C-style string containing the relative path name to a file that
         * should be saved to the computer.
         *
         * @param filetype
         * An img_file_t, representing the file format that should be used when
         * saving image data.
         *
         * @return true if the file was successfully saved. False if not.
         */
        bool save(const char* filename, img_file_t filetype = img_file_t::IMG_FILE_PNG) const;

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
        SR_ColorDataType format() const;
};



/*-------------------------------------
 * Get the pixel size of the currently loaded image
-------------------------------------*/
inline size_t SR_ImgFile::num_bytes() const {
    const size_t bytesPerPixel = sr_bytes_per_color(mFormat);
    return mDimens[0] * mDimens[1] * mDimens[2] * bytesPerPixel;
}



/*-------------------------------------
 * Get the pixel size of the currently loaded image
-------------------------------------*/
inline const size_t* SR_ImgFile::size() const {
    return mDimens;
}



/*-------------------------------------
 * Image Width
-------------------------------------*/
inline size_t SR_ImgFile::width() const {
    return mDimens[0];
}



/*-------------------------------------
 * Image Height
-------------------------------------*/
inline size_t SR_ImgFile::height() const {
    return mDimens[1];
}



/*-------------------------------------
 * Image Depth
-------------------------------------*/
inline size_t SR_ImgFile::depth() const {
    return mDimens[2];
}



/*-------------------------------------
 * Get the number of bits per pixel in the image.
-------------------------------------*/
inline unsigned SR_ImgFile::bpp() const {
    return mBpp;
}



/*-------------------------------------
 * Get the current image format.
-------------------------------------*/
inline SR_ColorDataType SR_ImgFile::format() const {
    return mFormat;
}



#endif  /* SR_IMG_FILE_HPP */
