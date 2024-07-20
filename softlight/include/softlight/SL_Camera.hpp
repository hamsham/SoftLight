
#ifndef SL_CAMERA_HPP
#define SL_CAMERA_HPP

#include "lightsky/script/Script.h"

#include "lightsky/math/vec2.h"
#include "lightsky/math/vec3.h"
#include "lightsky/math/mat4.h"



/*-----------------------------------------------------------------------------
 Forward Declarations
-----------------------------------------------------------------------------*/
class SL_Camera;
class SL_BoundingBox;
class SL_SceneGraph;
class SL_Transform;

namespace slscript
{
    template <ls::script::hash_t, typename var_type>
    class SL_SceneGraphScript;
} // end slscript namespace



/*-----------------------------------------------------------------------------
 Enums
-----------------------------------------------------------------------------*/
enum SL_FrustumPlane
{
    SL_FRUSTUM_PLANE_LEFT,
    SL_FRUSTUM_PLANE_RIGHT,
    SL_FRUSTUM_PLANE_BOTTOM,
    SL_FRUSTUM_PLANE_TOP,
    SL_FRUSTUM_PLANE_NEAR,
    SL_FRUSTUM_PLANE_FAR,
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
void sl_extract_frustum_planes(const ls::math::mat4& projection, ls::math::vec4 planes[6]) noexcept;

bool sl_is_visible(const ls::math::vec4& worldSpacePoint, const ls::math::vec4 planes[6]) noexcept;

bool sl_is_visible(const SL_BoundingBox& bb, const ls::math::mat4& mvMatrix, const ls::math::vec4 planes[6]) noexcept;



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
bool sl_is_visible(const ls::math::vec4& point, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept;

inline bool sl_is_visible(const ls::math::vec3& point, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept
{
    return sl_is_visible(ls::math::vec4{point[0], point[1], point[2], 1.f}, mvpMatrix, fovDivisor);
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
bool sl_is_visible(const SL_BoundingBox& bb, const ls::math::mat4& mvpMatrix, const float fovDivisor = 1.f) noexcept;



/*-------------------------------------
 * Radar-based frustum culling method as described by Hernandez-Rudomin in
 * their paper "A Rendering Pipeline for Real-time Crowds."
 *
 * https://pdfs.semanticscholar.org/4fae/54e3f9e79ba09ead5702648664b9932a1d3f.pdf
-------------------------------------*/
bool sl_is_visible(
    const SL_BoundingBox& bounds,
    const SL_Transform& camTrans,
    const ls::math::mat4& modelMat,
    float aspect,
    float fov) noexcept;



/**----------------------------------------------------------------------------
 * @brief View modes for SL_Camera objects
-----------------------------------------------------------------------------*/
enum SL_ProjectionType : uint32_t
{
    SL_PROJECTION_ORTHOGONAL,
    SL_PROJECTION_PERSPECTIVE,
    SL_PROJECTION_LOGARITHMIC_PERSPECTIVE,

    SL_PROJECTION_DEFAULT = SL_PROJECTION_PERSPECTIVE,
};



/**----------------------------------------------------------------------------
 * @brief Camera transformation class
-----------------------------------------------------------------------------*/
class SL_Camera
{
    friend class slscript::SL_SceneGraphScript<LS_SCRIPT_HASH_FUNC("SL_SceneGraph"), SL_SceneGraph*>;

    friend class SceneFileLoader;

  public:
    /**
     * @brief Default SL_Camera aspect width.
     */
    static constexpr float DEFAULT_ASPECT_WIDTH = 4.f;

    /**
     * @brief Default SL_Camera aspect height.
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
    static constexpr float DEFAULT_VIEW_ANGLE = ls::math::radians(60.f);

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
    SL_ProjectionType mProjType;

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
    ~SL_Camera();

    /**
     * @brief Constructor
     */
    SL_Camera();

    /**
     * @brief Copy Constructor
     */
    SL_Camera(const SL_Camera&);

    /**
     * @brief Move Constructor
     */
    SL_Camera(SL_Camera&&);

    /**
     * @brief Copy Operator
     */
    SL_Camera& operator=(const SL_Camera&);

    /**
     * @brief Move Operator
     */
    SL_Camera& operator=(SL_Camera&&);

    /**
     * @brief Assign a projection type for the current camera.
     * 
     * @param p
     * A value from the SL_ProjectionType enumeration which determines what
     * type of frustum to use for the projection matrix.
     */
    void projection_type(const SL_ProjectionType p);

    /**
     * @brief Retrieve the current frustum type for the projection matrix.
     * 
     * @return A value from the SL_ProjectionType enumeration which determines
     * what type of frustum to use for the projection matrix.
     */
    SL_ProjectionType projection_type() const;

    /**
     * @brief Retrieve the camera's projection matrix for external use.
     *
     * @return A 4x4 projection matrix which is used by *this for
     * projecting geometry data.
     */
    const ls::math::mat4& proj_matrix() const;

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
    void fov(float viewAngle);

    /**
     * @brief Retrieve the horizontal field of view of the camera.
     *
     * @return A floating point number, representing the number of radians
     * of the field of view of the camera.
     */
    float fov() const;

    /**
     * @brief Set the aspect ration of the internal camera.
     *
     * @param w
     * The width of the projection matrix frustum.
     *
     * @param h
     * The height of the projection matrix frustum.
     */
    void aspect_ratio(float w, float h);

    /**
     * @brief Set the aspect ration of the internal camera.
     *
     * @param aspect
     * A 2D vector containing both the width and height of the projection
     * matrix frustum.
     */
    void aspect_ratio(const ls::math::vec2& aspect);

    /**
     * @brief Get the aspect ratio of the camera's projection matrix.
     *
     * @return A floating-point value, representing the width of the
     * projection matrix frustum, divided by its height.
     */
    float aspect_ratio() const;

    /**
     * @brief Get the width of the projection matrix frustum.
     *
     * @return A floating-point value, representing the width of the
     * projection matrix frustum.
     */
    float aspect_width() const;

    /**
     * @brief Get the height of the projection matrix frustum.
     *
     * @return A floating-point value, representing the height of the
     * projection matrix frustum.
     */
    float aspect_height() const;

    /**
     * @brief Set the distance to the camera's near-clipping plane.
     *
     * @param inZnear
     * A floating point value, representing the distance to the camera's
     * near-clipping plane.
     */
    void near_plane(float inZNear);

    /**
     * @brief Get the distance to the camera's near-clipping plane.
     *
     * @return A floating point value, representing the distance to the
     * camera's near-clipping plane.
     */
    float near_plane() const;

    /**
     * @brief Set the distance to the camera's far-clipping plane.
     *
     * @param inZfar
     * A floating point value, representing the distance to the camera's
     * far-clipping plane.
     */
    void far_plane(float inZFar);

    /**
     * @brief Get the distance to the camera's far-clipping plane.
     *
     * @return A floating point value, representing the distance to the
     * camera's far-clipping plane.
     */
    float far_plane() const;

    /**
     * Determine if *this SL_Camera object needs an update.
     * 
     * @return TRUE if a call to "update()" is needed to apply any pending
     * projection updates, FALSE if not.
     */
    bool is_dirty() const noexcept;

    /**
     * Force the camera to require an update by a scene graph.
     */
    void force_dirty() noexcept;

    /**
     * @brief Apply all pending updates to the camera's view+projection
     * matrix.
     */
    void update();
};



/*-------------------------------------
 * Get the projection matrix
-------------------------------------*/
inline const ls::math::mat4& SL_Camera::proj_matrix() const
{
    return mProjection;
}



/*-------------------------------------
 * Set the FOV
-------------------------------------*/
inline void SL_Camera::fov(float viewAngle)
{
    mIsDirty = true;
    mFov = viewAngle;
}



/*-------------------------------------
 * Get the FOV
-------------------------------------*/
inline float SL_Camera::fov() const
{
    return mFov;
}



/*-------------------------------------
 * Set the aspect ratio
-------------------------------------*/
inline void SL_Camera::aspect_ratio(float w, float h)
{
    mIsDirty = true;
    mAspectW = w;
    mAspectH = h;
}



/*-------------------------------------
 * Set the aspect ratio
-------------------------------------*/
inline void SL_Camera::aspect_ratio(const ls::math::vec2& aspect)
{
    mIsDirty = true;
    mAspectW = aspect[0];
    mAspectH = aspect[1];
}



/*-------------------------------------
 * Get the current aspect ratio.
-------------------------------------*/
inline float SL_Camera::aspect_ratio() const
{
    return mAspectW / mAspectH;
}



/*-------------------------------------
 * Get the current aspect width.
-------------------------------------*/
inline float SL_Camera::aspect_width() const
{
    return mAspectW;
}



/*-------------------------------------
 * Get the current aspect height.
-------------------------------------*/
inline float SL_Camera::aspect_height() const
{
    return mAspectH;
}



/*-------------------------------------
 * Set the near plane distance
-------------------------------------*/
inline void SL_Camera::near_plane(float inZNear)
{
    mIsDirty = true;
    mZNear = inZNear;
}



/*-------------------------------------
 * Get the near plane distance
-------------------------------------*/
inline float SL_Camera::near_plane() const
{
    return mZNear;
}



/*-------------------------------------
 * Set the far plane distance
-------------------------------------*/
inline void SL_Camera::far_plane(float inZFar)
{
    mIsDirty = true;
    mZFar = inZFar;
}



/*-------------------------------------
 * Get the distance to the far plane.
-------------------------------------*/
inline float SL_Camera::far_plane() const
{
    return mZFar;
}



/*-------------------------------------
 * SL_Camera Update Inquiry
-------------------------------------*/
inline bool SL_Camera::is_dirty() const noexcept
{
    return mIsDirty;
}



/*-------------------------------------
 * SL_Camera Update Inquiry
-------------------------------------*/
inline void SL_Camera::force_dirty() noexcept
{
    mIsDirty = true;
}


#endif // SL_CAMERA_HPP
