
#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "lightsky/math/vec4.h"
#include "lightsky/math/mat_utils.h" // perspective transformation functions

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Plane.hpp"
#include "softlight/SL_Transform.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Camera Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Extract Frustum Planes
-------------------------------------*/
void sl_extract_frustum_planes(const ls::math::mat4& projection, ls::math::vec4 planes[6]) noexcept
{
    //planes[SL_FRUSTUM_PLANE_LEFT]   = projection[3] + projection[0];
    //planes[SL_FRUSTUM_PLANE_RIGHT]  = projection[3] - projection[0];
    //planes[SL_FRUSTUM_PLANE_BOTTOM] = projection[3] + projection[1];
    //planes[SL_FRUSTUM_PLANE_TOP]    = projection[3] - projection[1];
    //planes[SL_FRUSTUM_PLANE_NEAR]   = projection[3] + projection[2];
    //planes[SL_FRUSTUM_PLANE_FAR]    = projection[3] - projection[2];

    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_LEFT][i]   = projection[i][3] + projection[i][0];
    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_RIGHT][i]  = projection[i][3] - projection[i][0];

    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_BOTTOM][i] = projection[i][3] + projection[i][1];
    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_TOP][i]    = projection[i][3] - projection[i][1];

    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_NEAR][i]   = projection[i][3] + projection[i][2];
    for (unsigned i = 4; i--;) planes[SL_FRUSTUM_PLANE_FAR][i]    = projection[i][3] - projection[i][2];

    for (unsigned i = 6; i--;)
    {
        const float lenInv = math::rcp(math::length(math::vec3_cast(planes[i])));
        planes[i] = planes[i] * lenInv;
    }
}



bool sl_is_visible(const ls::math::vec4& p, const ls::math::vec4 planes[6]) noexcept
{
    for (unsigned i = 6; i--;)
    {
        float dist = math::dot(planes[i], p);
        if (dist < 0.f) return false;
    }

    return true;
}



bool sl_is_visible(const SL_BoundingBox& bb, const ls::math::mat4& mvpMatrix, const ls::math::vec4 planes[6]) noexcept
{
    const math::vec4& boxMax = mvpMatrix * bb.max_point();
    const math::vec4& boxMin = mvpMatrix * bb.min_point();

    const math::vec3&& center = math::vec3_cast((boxMax + boxMin) * 0.5f);
    const float radius = math::length(math::vec3_cast(boxMax - boxMin) * 0.5f);

    for (unsigned i = 0; i < 6; ++i)
    {
        const SL_Plane& plane = planes[i];
        const math::vec3&& plane3 = math::vec3_cast(plane);

        if ((math::dot(center, plane3) + plane[3] + radius) <= 0.f)
        {
            return false;
        }
    }

    return true;
}



/*-------------------------------------
 * Test the visibility of a point
-------------------------------------*/
bool sl_is_visible(const math::vec4& point, const math::mat4& mvpMatrix, const float fovDivisor) noexcept
{
    math::vec4&& temp = mvpMatrix * point;

    // Debug multipliers to reduce the frustum planes
    temp[0] *= fovDivisor;
    temp[1] *= fovDivisor;
    const bool x = temp[0] >= -temp[3] && temp[0] <= temp[3];
    const bool y = temp[1] >= -temp[3] && temp[1] <= temp[3];
    const bool z = temp[2] >= -temp[3] && temp[2] <= temp[3];

    return (x && y && z) || (temp[3] >= 1.f && x && y);
}



/*-------------------------------------
 * Test the visibility of a Bounding Box
-------------------------------------*/
bool sl_is_visible(const SL_BoundingBox& bb, const math::mat4& mvpMatrix, const float fovDivisor) noexcept
{
    const math::vec4& boxMax = bb.max_point();
    const math::vec4& boxMin = bb.min_point();

    const math::vec4 points[] = {
        {boxMax[0], boxMin[1], boxMin[2], 1.f},
        {boxMax[0], boxMax[1], boxMin[2], 1.f},
        {boxMax[0], boxMax[1], boxMax[2], 1.f},
        {boxMin[0], boxMax[1], boxMax[2], 1.f},
        {boxMin[0], boxMin[1], boxMax[2], 1.f},
        {boxMin[0], boxMin[1], boxMin[2], 1.f},
        {boxMax[0], boxMin[1], boxMax[2], 1.f},
        {boxMin[0], boxMax[1], boxMin[2], 1.f},
    };

    // Debug multipliers to reduce the frustum planes
    const math::vec4 fd = {fovDivisor, fovDivisor, 1.f, 1.f};

    unsigned haveLeft = 0;
    unsigned haveRight = 0;
    unsigned havebotom = 0;
    unsigned havetop = 0;

    unsigned numInFront = LS_ARRAY_SIZE(points);

    for (unsigned i = LS_ARRAY_SIZE(points); i--;)
    {
        math::vec4&& temp = mvpMatrix * points[i] * fd;

        /*
        const float wInv = 1.f / temp[3];
        temp[0] *= wInv;
        temp[1] *= wInv;
        temp[2] *= wInv;
        temp[3] = wInv;
        */

        //const unsigned outOfZBounds = temp[2] > temp[3] || temp[2] < -temp[3];
        const unsigned outOfZBounds = temp[3] < 1.f;
        if (outOfZBounds)
        {
            --numInFront;
            continue;
        }

        const unsigned xMin = temp[0] >= -temp[3];
        const unsigned xMax = temp[0] <= temp[3];
        const unsigned yMin = temp[1] >= -temp[3];
        const unsigned yMax = temp[1] <= temp[3];

        if (xMin && xMax && yMin && yMax)
        {
            return true;
        }

        haveLeft  = haveLeft  || !xMin;
        haveRight = haveRight || !xMax;
        havebotom = havebotom || !yMin;
        havetop   = havetop   || !yMax;
    }

    // A bounding box is visible if it's partially (or fully) front of the
    // near plane and within the bounds of the view frustum.
    return numInFront && (1 < (haveLeft + haveRight + havebotom + havetop));
}



/*-------------------------------------
 * Radar-based frustum culling method as described by Hernandez-Rudomin in
 * their paper "A Rendering Pipeline for Real-time Crowds."
 *
 * https://pdfs.semanticscholar.org/4fae/54e3f9e79ba09ead5702648664b9932a1d3f.pdf
-------------------------------------*/
bool sl_is_visible(
    const SL_BoundingBox& bounds,
    const SL_Transform& camTrans,
    const math::mat4& modelMat,
    float aspect,
    float fov) noexcept
{
    const float      viewAngle = math::tan(fov*0.5f);
    const math::vec3 c         = camTrans.absolute_position();
    const math::mat3 t         = math::mat3{math::transpose(camTrans.transform())};
    const math::vec3 cx        = t[0];
    const math::vec3 cy        = t[1];
    const math::vec3 cz        = -t[2];
    const math::vec4 boxMax    = bounds.max_point();
    const math::vec4 boxMin    = bounds.min_point();
    const float      delta     = 0.f;

    math::vec4 points[]  = {
        {boxMin[0], boxMin[1], boxMax[2], 1.f},
        {boxMax[0], boxMin[1], boxMax[2], 1.f},
        {boxMax[0], boxMax[1], boxMax[2], 1.f},
        {boxMin[0], boxMax[1], boxMax[2], 1.f},
        {boxMin[0], boxMin[1], boxMin[2], 1.f},
        {boxMax[0], boxMin[1], boxMin[2], 1.f},
        {boxMax[0], boxMax[1], boxMin[2], 1.f},
        {boxMin[0], boxMax[1], boxMin[2], 1.f}
    };

    float objX, objY, objZ, xAspect, yAspect;

    for (unsigned i = LS_ARRAY_SIZE(points); i--;)
    {
        points[i] = modelMat * points[i];
    }

    for (unsigned i = LS_ARRAY_SIZE(points); i--;)
    {
        const math::vec3& p = math::vec3_cast(points[i]);

        // compute vector from camera position to p
        const math::vec3&& v = p - c;

        // compute and test the Z coordinate
        objZ = math::dot(v, cz);
        if (objZ < 0.f)
        {
            continue;
        }

        // compute and test the Y coordinate
        objY = math::dot(v, cy);
        yAspect = objZ * viewAngle;
        yAspect += delta;
        if (objY > yAspect || objY < -yAspect)
        {
            continue;
        }

        // compute and test the X coordinate
        objX = math::dot(v, cx);
        xAspect = yAspect * aspect;
        xAspect += delta;
        if (objX > xAspect || objX < -xAspect)
        {
            continue;
        }

        return true;
    }

    const math::vec3 bboxMin = math::vec3_cast(modelMat * boxMin);
    const math::vec3 bboxMax = math::vec3_cast(modelMat * boxMax);

    return c > bboxMin && c < bboxMax;
}



/*-----------------------------------------------------------------------------
 * Camera Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Default Camera Perspective
-------------------------------------*/
const math::mat4 SL_Camera::DEFAULT_PERSPECTIVE{
    math::perspective(
        DEFAULT_VIEW_ANGLE,
        DEFAULT_ASPECT_WIDTH / DEFAULT_ASPECT_HEIGHT,
        DEFAULT_Z_NEAR,
        DEFAULT_Z_FAR
    )
};



/*-------------------------------------
 * Destructor
-------------------------------------*/
SL_Camera::~SL_Camera()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_Camera::SL_Camera() :
    mIsDirty{false},
    mProjType{SL_ProjectionType::SL_PROJECTION_DEFAULT},
    mFov{DEFAULT_VIEW_ANGLE},
    mAspectW{DEFAULT_ASPECT_WIDTH},
    mAspectH{DEFAULT_ASPECT_HEIGHT},
    mZNear{DEFAULT_Z_NEAR},
    mZFar{DEFAULT_Z_FAR},
    mProjection{math::perspective(
        DEFAULT_VIEW_ANGLE,
        DEFAULT_ASPECT_WIDTH / DEFAULT_ASPECT_HEIGHT,
        DEFAULT_Z_NEAR,
        DEFAULT_Z_FAR
    )}
{}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_Camera::SL_Camera(const SL_Camera& c)
{
    *this = c;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_Camera::SL_Camera(SL_Camera&& c)
{
    *this = c;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_Camera& SL_Camera::operator=(const SL_Camera& c)
{
    mIsDirty = c.mIsDirty;
    mProjType = c.mProjType;
    mFov = c.mFov;
    mAspectW = c.mAspectW;
    mAspectH = c.mAspectH;
    mZNear = c.mZNear;
    mZFar = c.mZFar;
    mProjection = c.mProjection;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_Camera& SL_Camera::operator=(SL_Camera&& c)
{
    mIsDirty = c.mIsDirty;
    c.mIsDirty = false;

    mProjType = c.mProjType;
    c.mProjType = SL_ProjectionType::SL_PROJECTION_DEFAULT;

    mFov = c.mFov;
    c.mFov = DEFAULT_VIEW_ANGLE;

    mAspectW = c.mAspectW;
    c.mAspectW = DEFAULT_ASPECT_WIDTH;

    mAspectH = c.mAspectH;
    c.mAspectH = DEFAULT_ASPECT_HEIGHT;

    mZNear = c.mZNear;
    c.mZNear = DEFAULT_Z_NEAR;

    mZFar = c.mZFar;
    c.mZFar = DEFAULT_Z_FAR;

    mProjection = c.mProjection;
    c.mProjection = math::perspective(
        DEFAULT_VIEW_ANGLE,
        DEFAULT_ASPECT_WIDTH / DEFAULT_ASPECT_HEIGHT,
        DEFAULT_Z_NEAR,
        DEFAULT_Z_FAR
    );

    return *this;
}



/*-------------------------------------
 * Set the current projection mode
-------------------------------------*/
void SL_Camera::set_projection_type(const SL_ProjectionType p)
{
    mIsDirty = true;
    mProjType = p;
}



/*-------------------------------------
 * Update Implementation
-------------------------------------*/
void SL_Camera::update()
{
    mIsDirty = false;

    switch (mProjType)
    {
        case SL_PROJECTION_ORTHOGONAL:
            mProjection = math::ortho(-mAspectW, mAspectW, -mAspectH, mAspectH, mZNear, mZFar);
            break;

        case SL_PROJECTION_PERSPECTIVE:
            mProjection = math::perspective(mFov, mAspectW / mAspectH, mZNear, mZFar);
            break;

        case SL_PROJECTION_LOGARITHMIC_PERSPECTIVE:
            mProjection = math::infinite_perspective(mFov, mAspectW / mAspectH, mZNear);
            break;

        default:LS_DEBUG_ASSERT(false);
            break;
    }
}
