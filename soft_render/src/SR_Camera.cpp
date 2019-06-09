
#include "lightsky/utils/Assertions.h"
#include "lightsky/utils/Log.h"

#include "lightsky/math/vec4.h"
#include "lightsky/math/mat_utils.h" // perspective transformation functions

#include "soft_render/SR_BoundingBox.hpp"
#include "soft_render/SR_Camera.hpp"
#include "soft_render/SR_Transform.hpp"



namespace math = ls::math;



/*-----------------------------------------------------------------------------
 * Camera Functions
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Test the visibility of a point
-------------------------------------*/
bool sr_is_visible(const math::vec4& point, const math::mat4& mvpMatrix, const float fovDivisor) noexcept
{
    math::vec4&& temp = mvpMatrix * point;

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
bool sr_is_visible(const SR_BoundingBox& bb, const math::mat4& mvpMatrix, const float fovDivisor) noexcept
{
    const math::vec4& trr = mvpMatrix * bb.get_top_rear_right();
    const math::vec4& bfl = mvpMatrix * bb.get_bot_front_left();

    const math::vec4 points[] = {
        {trr[0], bfl[1], bfl[2], 1.f},
        {trr[0], trr[1], bfl[2], 1.f},
        {trr[0], trr[1], trr[2], 1.f},
        {bfl[0], trr[1], trr[2], 1.f},
        {bfl[0], bfl[1], trr[2], 1.f},
        {bfl[0], bfl[1], bfl[2], 1.f},

        {trr[0], bfl[1], trr[2], 1.f},
        {bfl[0], trr[1], bfl[2], 1.f},
    };

    // Debug multipliers to reduce the frustum planes
    const math::vec4 fd = {fovDivisor, fovDivisor, 1.f, 1.f};

    for (unsigned i = 0; i < LS_ARRAY_SIZE(points); ++i)
    {
        math::vec4&& temp = points[i] * fd;

        if (temp[0] > -temp[3] && temp[0] < temp[3] &&
            temp[1] > -temp[3] && temp[1] < temp[3] &&
            temp[2] > -temp[3] && temp[2] < temp[3])
        {
            return true;
        }
    }

    return false;
}



/*-------------------------------------
 * Radar-based frustum culling method as described by Hernandez-Rudomin in
 * their paper "A Rendering Pipeline for Real-time Crowds."
 *
 * https://pdfs.semanticscholar.org/4fae/54e3f9e79ba09ead5702648664b9932a1d3f.pdf
-------------------------------------*/
bool sr_is_visible(
    float aspect,
    float fov,
    const SR_Transform& camTrans,
    const math::mat4& modelMat,
    const SR_BoundingBox& bounds) noexcept
{
    const float      viewAngle = math::tan(fov*0.5f);
    const math::vec3 c         = camTrans.get_abs_position();
    const math::mat3 t         = math::mat3{math::transpose(camTrans.get_transform())};
    const math::vec3 cx        = t[0];
    const math::vec3 cy        = t[1];
    const math::vec3 cz        = -t[2];
    const math::vec4 trr       = bounds.get_top_rear_right();
    const math::vec4 bfl       = bounds.get_bot_front_left();
    const float      delta     = 0.f;

    math::vec4 points[]  = {
        {bfl[0], bfl[1], trr[2], 1.f},
        {trr[0], bfl[1], trr[2], 1.f},
        {trr[0], trr[1], trr[2], 1.f},
        {bfl[0], trr[1], trr[2], 1.f},
        {bfl[0], bfl[1], bfl[2], 1.f},
        {trr[0], bfl[1], bfl[2], 1.f},
        {trr[0], trr[1], bfl[2], 1.f},
        {bfl[0], trr[1], bfl[2], 1.f}
    };

    float objX, objY, objZ, xAspect, yAspect;

    for (unsigned i = 0; i < LS_ARRAY_SIZE(points); ++i)
    {
        points[i] = modelMat * points[i];
    }

    for (unsigned i = 0; i < LS_ARRAY_SIZE(points); ++i)
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

    const math::vec3 bboxMin = math::vec3_cast(modelMat * bfl);
    const math::vec3 bboxMax = math::vec3_cast(modelMat * trr);

    return c > bboxMin && c < bboxMax;
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
