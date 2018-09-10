
#ifndef SR_ANIMATION_PROPERTY_HPP
#define SR_ANIMATION_PROPERTY_HPP



/*-----------------------------------------------------------------------------
 * Forward Declarations
------------------------------------------------------------------------------*/
namespace ls
{
   namespace math
   {
       template<typename num_t>
       struct vec3_t;

       template<typename num_t>
       struct quat_t;
   } // end math namespace
} // end ls namespace



/*-----------------------------------------------------------------------------
 * Data Types used for animations
-----------------------------------------------------------------------------*/
/**-------------------------------------
 * @brief Data Type to be used for animation interpolation.
 * 
 * Animations themselves use single-point precision for vectors and quaternions
 * but interpolation between frames may need more precision.
-------------------------------------*/
typedef float SR_AnimPrecision;



#endif /* SR_ANIMATION_PROPERTY_HPP */
