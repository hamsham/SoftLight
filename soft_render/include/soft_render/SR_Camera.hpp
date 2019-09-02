
#ifndef SR_CAMERA_HPP
#define SR_CAMERA_HPP

#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"
#include "lightsky/math/mat4.h"



/*-----------------------------------------------------------------------------
 Forward Declarations
-----------------------------------------------------------------------------*/
class SR_Camera;

class SR_BoundingBox;

class SR_Transform;



/*-----------------------------------------------------------------------------
 Enums
-----------------------------------------------------------------------------*/
enum SR_FrustumPlane
{
    SR_FRUSTUM_PLANE_LEFT,
    SR_FRUSTUM_PLANE_RIGHT,
    SR_FRUSTUM_PLANE_BOTTOM,
    SR_FRUSTUM_PLANE_TOP,
    SR_FRUSTUM_PLANE_NEAR,
    SR_FRUSTUM_PLANE_FAR,
};



/*-----------------------------------------------------------------------------
 Utility Functions
-----------------------------------------------------------------------------*/
/**
 * @brief Extract the planes from a projection matrix and store them in an
 * array.
 *
 * @param projection
 *
 * @param planes
 */
void sr_extract_frustum_planes(const ls::math::mat4& projection, ls::math::vec4 planes[6]) noexcept;

bool sr_is_visible(const ls::math::vec4& worldSpacePoint, const ls::math::vec4 planes[6]) noexcept;

bool sr_is_visible(const SR_BoundingBox& bb, const ls::math::mat4& mvMatrix, const ls::math::vec4 planes[6]) noexcept;



/**
 * Determine if a point is contained within a frustum.
 * 
 * @param point
 * A 3D vector which will be tested for visibility in a view frustum.
 * 
 * @param mvpMatrix
 * A 4x4 homogenous matrix representing the input point's clip-space
 * coordinates.
 * 
 * @param fovDivisor
 * This value can be increased to reduce the calculated frustum's dimensions.
 * This is mostly used for debugging purposes.
 * 
 * @return TRUE if the input point is within the frustum extracted from
 * "mvpMatrix", FALSE if not.
 */
bool sr_is_visible(const ls::math::vec4& point, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept;

inline bool sr_is_visible(const ls::math::vec3& point, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept
{
    return sr_is_visible(ls::math::vec4{point[0], point[1], point[2], 1.f}, mvpMatrix, fovDivisor);
}



/**
 * Determine if a bounding box is contained within a frustum..
 * 
 * @param bb
 * A bounding box object which will be tested for visibility in a view frustum.
 * 
 * @param mvpMatrix
 * A 4x4 homogenous matrix representing the clip-space coordinates of the input
 * bounding box
 * 
 * @param fovDivisor
 * This value can be increased to reduce the calculated frustum's dimensions.
 * This is mostly used for debugging purposes.
 * 
 * @return TRUE if the input box is within the frustum extracted from
 * "mvpMatrix", FALSE if not.
 */
bool sr_is_visible(const SR_BoundingBox& bb, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept;



/*-------------------------------------
 * Radar-based frustum culling method as described by Hernandez-Rudomin in
 * their paper "A Rendering Pipeline for Real-time Crowds."
 *
 * https://pdfs.semanticscholar.org/4fae/54e3f9e79ba09ead5702648664b9932a1d3f.pdf
-------------------------------------*/
bool sr_is_visible(
    const SR_BoundingBox& bounds,
    const SR_Transform& camTrans,
    const ls::math::mat4& modelMat,
    float aspect,
    float fov) noexcept;



/**----------------------------------------------------------------------------
 * @brief View modes for SR_Camera objects
-----------------------------------------------------------------------------*/
enum SR_ProjectionType : uint32_t
{
    SR_PROJECTION_ORTHOGONAL,
    SR_PROJECTION_PERSPECTIVE,
    SR_PROJECTION_LOGARITHMIC_PERSPECTIVE,

    SR_PROJECTION_DEFAULT = SR_PROJECTION_PERSPECTIVE,
};



/**----------------------------------------------------------------------------
 * @brief Camera transformation class
-----------------------------------------------------------------------------*/
class SR_Camera
{
    friend class SceneFileLoader;

  public:
    /**
     * @brief Default SR_Camera aspect width.
     */
    static constexpr float DEFAULT_ASPECT_WIDTH = 4.f;

    /**
     * @brief Default SR_Camera aspect height.
     */
    static constexpr float DEFAULT_ASPECT_HEIGHT = 3.f;

    /**
     * @brief Default near-plane distance
     */
    static constexpr float DEFAULT_Z_NEAR = 0.1f;

    /**
     * @brief Default far plane distance
     */
    static constexpr float DEFAULT_Z_FAR = 100.f;

    /**
     * @brief Default angle for the field-of-view
     */
    static constexpr float DEFAULT_VIEW_ANGLE = LS_DEG2RAD(60.f);

    /**
     * @brief Default Perspective-projection matrix.
     */
    static const ls::math::mat4 DEFAULT_PERSPECTIVE;

  private:
    /**
     * Flag to determine if *this camera needs updating.
     */
    bool mIsDirty;

    /**
     * Projection type for the camera. This can help determine if the current
     * projection matrix is orthogonal, perspective, or represents a
     * logarithmic (pseudo-infinite) perspective matrix.
     */
    SR_ProjectionType mProjType;

    /**
     * @brief fov Determines the angle of vision for the camera.
     */
    float mFov;

    /**
     * @brief Helps to determine the aspect ratio for perspective and
     * orthographic camera modes.
     */
    float mAspectW;

    /**
     * @brief mAspectH Helps to determine the aspect ratio for perspective
     * and orthographic camera modes.
     */
    float mAspectH;

    /**
     * @brief Distance to the nearby occlusion plane.
     */
    float mZNear;

    /**
     * @brief Distance to the far occlusion plane.
     */
    float mZFar;

    /**
     * @brief mProjection contains only the projection parameters of the
     * camera which make up a viewing frustum.
     */
    ls::math::mat4 mProjection;

  public:
    /**
     * @brief Destructor
     */
    ~SR_Camera();

    /**
     * @brief Constructor
     */
    SR_Camera();

    /**
     * @brief Copy Constructor
     */
    SR_Camera(const SR_Camera&);

    /**
     * @brief Move Constructor
     */
    SR_Camera(SR_Camera&&);

    /**
     * @brief Copy Operator
     */
    SR_Camera& operator=(const SR_Camera&);

    /**
     * @brief Move Operator
     */
    SR_Camera& operator=(SR_Camera&&);

    /**
     * @brief Assign a projection type for the current camera.
     * 
     * @param p
     * A value from the SR_ProjectionType enumeration which determines what
     * type of frustum to use for the projection matrix.
     */
    void set_projection_type(const SR_ProjectionType p);

    /**
     * @brief Retrieve the current frustum type for the projection matrix.
     * 
     * @return A value from the SR_ProjectionType enumeration which determines
     * what type of frustum to use for the projection matrix.
     */
    SR_ProjectionType get_projection_type() const;

    /**
     * @brief Retrieve the camera's projection matrix for external use.
     *
     * @return A 4x4 projection matrix which is used by *this for
     * projecting geometry data.
     */
    const ls::math::mat4& get_proj_matrix() const;

    /**
     * @brief Set the field of view for the camera;
     *
     * Remember to call either "makeOrtho()" or "makePerspective()" after
     * calling this method.
     *
     * @param viewAngle
     * The desired horizontal angle, in radians, which the field of view
     * should be set to.
     */
    void set_fov(unsigned viewAngle);

    /**
     * @brief Retrieve the horizontal field of view of the camera.
     *
     * @return A floating point number, representing the number of radians
     * of the field of view of the camera.
     */
    float get_fov() const;

    /**
     * @brief Set the aspect ration of the internal camera.
     *
     * @param w
     * The width of the projection matrix frustum.
     *
     * @param h
     * The height of the projection matrix frustum.
     */
    void set_aspect_ratio(float w, float h);

    /**
     * @brief Set the aspect ration of the internal camera.
     *
     * @param aspect
     * A 2D vector containing both the width and height of the projection
     * matrix frustum.
     */
    void set_aspect_ratio(const ls::math::vec2& aspect);

    /**
     * @brief Get the aspect ratio of the camera's projection matrix.
     *
     * @return A floating-point value, representing the width of the
     * projection matrix frustum, divided by its height.
     */
    float get_aspect_ratio() const;

    /**
     * @brief Get the width of the projection matrix frustum.
     *
     * @return A floating-point value, representing the width of the
     * projection matrix frustum.
     */
    float get_aspect_width() const;

    /**
     * @brief Get the height of the projection matrix frustum.
     *
     * @return A floating-point value, representing the height of the
     * projection matrix frustum.
     */
    float get_aspect_height() const;

    /**
     * @brief Set the distance to the camera's near-clipping plane.
     *
     * @param inZnear
     * A floating point value, representing the distance to the camera's
     * near-clipping plane.
     */
    void set_near_plane(float inZNear);

    /**
     * @brief Get the distance to the camera's near-clipping plane.
     *
     * @return A floating point value, representing the distance to the
     * camera's near-clipping plane.
     */
    float get_near_plane() const;

    /**
     * @brief Set the distance to the camera's far-clipping plane.
     *
     * @param inZfar
     * A floating point value, representing the distance to the camera's
     * far-clipping plane.
     */
    void set_far_plane(float inZFar);

    /**
     * @brief Get the distance to the camera's far-clipping plane.
     *
     * @return A floating point value, representing the distance to the
     * camera's far-clipping plane.
     */
    float get_far_plane() const;

    /**
     * Determine if *this SR_Camera object needs an update.
     * 
     * @return TRUE if a call to "update()" is needed to apply any pending
     * projection updates, FALSE if not.
     */
    bool is_dirty() const noexcept;

    /**
     * @brief Apply all pending updates to the camera's view+projection
     * matrix.
     */
    void update();
};



/*-------------------------------------
 * Get the projection matrix
-------------------------------------*/
inline const ls::math::mat4& SR_Camera::get_proj_matrix() const
{
    return mProjection;
}



/*-------------------------------------
 * Set the FOV
-------------------------------------*/
inline void SR_Camera::set_fov(unsigned viewAngle)
{
    mIsDirty = true;
    mFov = viewAngle;
}



/*-------------------------------------
 * Get the FOV
-------------------------------------*/
inline float SR_Camera::get_fov() const
{
    return mFov;
}



/*-------------------------------------
 * Set the aspect ratio
-------------------------------------*/
inline void SR_Camera::set_aspect_ratio(float w, float h)
{
    mIsDirty = true;
    mAspectW = w;
    mAspectH = h;
}



/*-------------------------------------
 * Set the aspect ratio
-------------------------------------*/
inline void SR_Camera::set_aspect_ratio(const ls::math::vec2& aspect)
{
    mIsDirty = true;
    mAspectW = aspect[0];
    mAspectH = aspect[1];
}



/*-------------------------------------
 * Get the current aspect ratio.
-------------------------------------*/
inline float SR_Camera::get_aspect_ratio() const
{
    return mAspectW / mAspectH;
}



/*-------------------------------------
 * Get the current aspect width.
-------------------------------------*/
inline float SR_Camera::get_aspect_width() const
{
    return mAspectW;
}



/*-------------------------------------
 * Get the current aspect height.
-------------------------------------*/
inline float SR_Camera::get_aspect_height() const
{
    return mAspectH;
}



/*-------------------------------------
 * Set the near plane distance
-------------------------------------*/
inline void SR_Camera::set_near_plane(float inZNear)
{
    mIsDirty = true;
    mZNear = inZNear;
}



/*-------------------------------------
 * Get the near plane distance
-------------------------------------*/
inline float SR_Camera::get_near_plane() const
{
    return mZNear;
}



/*-------------------------------------
 * Set the far plane distance
-------------------------------------*/
inline void SR_Camera::set_far_plane(float inZFar)
{
    mIsDirty = true;
    mZFar = inZFar;
}



/*-------------------------------------
 * Get the distance to the far plane.
-------------------------------------*/
inline float SR_Camera::get_far_plane() const
{
    return mZFar;
}



/*-------------------------------------
 * SR_Camera Update Inquiry
-------------------------------------*/
inline bool SR_Camera::is_dirty() const noexcept
{
    return mIsDirty;
}


#endif // SR_CAMERA_HPP
