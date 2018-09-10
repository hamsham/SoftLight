
#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "lightsky/math/vec4.h"
#include "lightsky/math/mat_utils.h" // perspective transformation functions

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Camera Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Test the visibility of a point
-------------------------------------*/
bool sr_is_visible(const math::vec3& point, const math::mat4& mvpMatrix, const float fovDivisor)
{
    math::vec4&& temp = mvpMatrix * math::vec4{point[0], point[1], point[2], 1.f};

    // Debug multipliers to reduce the frustum planes
    temp[0] *= fovDivisor;
    temp[1] *= fovDivisor;

    return
        temp[0] > -temp[3] && temp[0] < temp[3] &&
        temp[1] > -temp[3] && temp[1] < temp[3] &&
        temp[2] > -temp[3] && temp[2] < temp[3];
}



/*-------------------------------------
 * Test the visibility of a Bounding Box
-------------------------------------*/
bool sr_is_visible(const SR_BoundingBox& bb, const math::mat4& mvpMatrix, const float fovDivisor)
{
    const math::vec3& trr = bb.get_top_rear_right();
    const math::vec3& bfl = bb.get_bot_front_left();

    const math::vec3 points[] = {
        {trr[0], bfl[1], bfl[2]},
        {trr[0], trr[1], bfl[2]},
        {trr[0], trr[1], trr[2]},
        {bfl[0], trr[1], trr[2]},
        {bfl[0], bfl[1], trr[2]},
        {bfl[0], bfl[1], bfl[2]},

        {trr[0], bfl[1], trr[2]},
        {bfl[0], trr[1], bfl[2]},
    };

    for (unsigned i = 0; i < LS_ARRAY_SIZE(points); ++i)
    {
        if (sr_is_visible(points[i], mvpMatrix, fovDivisor))
        {
            return true;
        }
    }

    return false;
}



/*-----------------------------------------------------------------------------
 * Camera Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Default Camera Perspective
-------------------------------------*/
const math::mat4 SR_Camera::DEFAULT_PERSPECTIVE{
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
SR_Camera::~SR_Camera()
{
}



/*-------------------------------------
 * Constructor
-------------------------------------*/
SR_Camera::SR_Camera() :
    mIsDirty{false},
    mProjType{SR_ProjectionType::SR_PROJECTION_DEFAULT},
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
SR_Camera::SR_Camera(const SR_Camera& c)
{
    *this = c;
}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SR_Camera::SR_Camera(SR_Camera&& c)
{
    *this = c;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SR_Camera& SR_Camera::operator=(const SR_Camera& c)
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
SR_Camera& SR_Camera::operator=(SR_Camera&& c)
{
    mIsDirty = c.mIsDirty;
    c.mIsDirty = false;

    mProjType = c.mProjType;
    c.mProjType = SR_ProjectionType::SR_PROJECTION_DEFAULT;

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
void SR_Camera::set_projection_type(const SR_ProjectionType p)
{
    mIsDirty = true;
    mProjType = p;
}



/*-------------------------------------
 * Update Implementation
-------------------------------------*/
void SR_Camera::update()
{
    mIsDirty = false;

    switch (mProjType)
    {
        case SR_PROJECTION_ORTHOGONAL:
            mProjection = math::ortho(-mAspectW, mAspectW, -mAspectH, mAspectH, mZNear, mZFar);
            break;

        case SR_PROJECTION_PERSPECTIVE:
            mProjection = math::perspective(mFov, mAspectW / mAspectH, mZNear, mZFar);
            break;

        case SR_PROJECTION_LOGARITHMIC_PERSPECTIVE:
            mProjection = math::infinite_perspective(mFov, mAspectW / mAspectH, mZNear);
            break;

        default:LS_DEBUG_ASSERT(false);
            break;
    }
}
