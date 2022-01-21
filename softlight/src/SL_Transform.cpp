
#include <utility> // std::move()

#include "lightsky/setup/Macros.h" // LS_ENUM_VAL

#include "lightsky/utils/Assertions.h"

#include "lightsky/math/vec_utils.h"
#include "lightsky/math/mat_utils.h"
#include "lightsky/math/quat_utils.h"

#include "softlight/SL_Transform.hpp"



namespace math = ls::math;


/*-----------------------------------------------------------------------------
 * Anonymous helper stuff for the Camera class.
-----------------------------------------------------------------------------*/
namespace
{


enum class SL_ModelViewAxis : unsigned
{
    MV_X_AXIS = 0,
    MV_Y_AXIS = 1,
    MV_Z_AXIS = 2
};



/**
 * @brief Retrieve a camera's post-transformed x, y, z, or position vectors
 * from its model-view matrix.
 *
 * @param rotationMat
 * A constant reference to the camera's model-view matrix.
 *
 * @param axis
 * The specific axis of rotation, or position.
 *
 * @return A 3D vector, containing the transformed model-view matrix position
 * or axis of rotation.
 */
inline math::vec3 extractMVVector(const math::mat4& viewMat, const SL_ModelViewAxis axis)
{
    const unsigned a = LS_ENUM_VAL(axis);
    const math::mat3&& rotationMat = math::mat3{viewMat};
    const math::vec3&& mvVec = -rotationMat[a];

    return math::transpose(rotationMat) * mvVec;
}


} // end anonymous namespace



/*-----------------------------------------------------------------------------
 * SL_Transform Class
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Destructor
-------------------------------------*/
/*
SL_Transform::~SL_Transform() noexcept
{
}
*/



/*-------------------------------------
 * Constructor
-------------------------------------*/
SL_Transform::SL_Transform(const SL_TransformType transformType) noexcept :
    mFlags{0},
    mType{transformType},
    mPosition{0.f},
    mScaling{1.f, 1.f, 1.f},
    mOrientation{0.f, 0.f, 0.f, 1.f},
    mModelMat{1.f}
{}



/*-------------------------------------
 * Matrix Constructor
-------------------------------------*/
SL_Transform::SL_Transform(const math::mat4& modelMat, const SL_TransformType transformType) noexcept :
    SL_Transform{transformType}
{
    extract_transforms(modelMat);
}



/*-------------------------------------
 * Copy Constructor
-------------------------------------*/
SL_Transform::SL_Transform(const SL_Transform& t) noexcept :
    mFlags{t.mFlags},
    mType{t.mType},
    mPosition{t.mPosition},
    mScaling{t.mScaling},
    mOrientation{t.mOrientation},
    mModelMat{t.mModelMat}
{}



/*-------------------------------------
 * Move Constructor
-------------------------------------*/
SL_Transform::SL_Transform(SL_Transform&& t) noexcept :
    mFlags{t.mFlags},
    mType{t.mType},
    mPosition{std::move(t.mPosition)},
    mScaling{std::move(t.mScaling)},
    mOrientation{std::move(t.mOrientation)},
    mModelMat{std::move(t.mModelMat)}
{
    //t.mParentId = 0;
    //t.mFlags = 0;
    //t.mType = SL_TRANSFORM_TYPE_MODEL;
}



/*-------------------------------------
 * Copy Operator
-------------------------------------*/
SL_Transform& SL_Transform::operator=(const SL_Transform& t) noexcept
{
    mFlags = t.mFlags;
    mType = t.mType;
    mPosition = t.mPosition;
    mScaling = t.mScaling;
    mOrientation = t.mOrientation;
    mModelMat = t.mModelMat;

    return *this;
}



/*-------------------------------------
 * Move Operator
-------------------------------------*/
SL_Transform& SL_Transform::operator=(SL_Transform&& t) noexcept
{
    /*
    mFlags = t.mFlags;
    //t.mFlags = 0;

    mType = t.mType;
    //t.mType = SL_TRANSFORM_TYPE_MODEL;

    mPosition = std::move(t.mPosition);
    mScaling = std::move(t.mScaling);
    mOrientation = std::move(t.mOrientation);
    mModelMat = std::move(t.mModelMat);
    */
    mFlags = t.mFlags;
    mType = t.mType;
    mPosition = t.mPosition;
    mScaling = t.mScaling;
    mOrientation = t.mOrientation;
    mModelMat = t.mModelMat;

    return *this;
}



/*-----------------------------------------------------------------------------
 * Positioning
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Adjust the position
-------------------------------------*/
void SL_Transform::move(const math::vec3& deltaPos, bool relative) noexcept
{
    if (mType == SL_TRANSFORM_TYPE_VIEW_ARC || mType == SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y)
    {
        relative = !relative;
    }

    if (!relative)
    {
        const math::vec3&& translation = {
            math::dot(math::get_x_axis(mOrientation), deltaPos),
            math::dot(math::get_y_axis(mOrientation), deltaPos),
            math::dot(math::get_z_axis(mOrientation), deltaPos)
        };
        mPosition += translation;
    }
    else
    {
        mPosition += deltaPos;
    }

    set_dirty();
}



/*-------------------------------------
 * Set the position
-------------------------------------*/
void SL_Transform::position(const math::vec3& newPos) noexcept
{
    mPosition = newPos;
    set_dirty();
}



/*-------------------------------------
 * Get the absolute position
-------------------------------------*/
math::vec3 SL_Transform::absolute_position() const noexcept
{
    if (mType == SL_TRANSFORM_TYPE_MODEL)
    {
        return math::vec3{mModelMat[3][0], mModelMat[3][1], mModelMat[3][2]};
    }

    const math::mat3&& rotationMat = math::mat3{mModelMat};
    const math::vec3&& mvVec = math::vec3{-mModelMat[3][0], -mModelMat[3][1], -mModelMat[3][2]};
    return math::transpose(rotationMat) * mvVec;
}



/*-----------------------------------------------------------------------------
 * Scaling
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Adjust the scaling
-------------------------------------*/
void SL_Transform::scale(const math::vec3& deltaScale) noexcept
{
    mScaling *= deltaScale;
    set_dirty();
}



/*-------------------------------------
 * Set the scaling
-------------------------------------*/
void SL_Transform::scaling(const math::vec3& newScale) noexcept
{
    mScaling = newScale;
    set_dirty();
}



/*-----------------------------------------------------------------------------
 * Orientation
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Adjust the orientation
-------------------------------------*/
void SL_Transform::rotate(const math::quat& deltaRotation) noexcept
{
    mOrientation = math::normalize(mOrientation * deltaRotation);
    set_dirty();
}



/*-------------------------------------
 * Adjust the orientation in percentages
-------------------------------------*/
void SL_Transform::rotate(const math::vec3& amount) noexcept
{
    const math::quat&& xAxis = math::quat{amount[1], 0.f, 0.f, 1.f};
    const math::quat&& yAxis = math::quat{0.f, amount[0], 0.f, 1.f};
    const math::quat&& zAxis = math::quat{0.f, 0.f, amount[2], 1.f};

    if (mType == SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y
    || mType == SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y)
    {
        const math::quat&& newOrientation = xAxis * mOrientation * yAxis * zAxis;
        orientation(math::normalize(newOrientation));
    }
    else
    {
        rotate(xAxis * yAxis * zAxis);
        set_dirty();
    }
}



/*-------------------------------------
 * Set the orientation
-------------------------------------*/
void SL_Transform::orientation(const math::quat& newRotation) noexcept
{
    mOrientation = newRotation;
    set_dirty();
}



/*-----------------------------------------------------------------------------
 * Final Transformations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Apply all transformations to the model matrix
-------------------------------------*/
void SL_Transform::apply_transform(bool useRST) noexcept
{
    if (mType == SL_TRANSFORM_TYPE_VIEW_FPS || mType == SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y)
    {
        useRST = !useRST;
    }

    mModelMat = useRST ? get_rst_matrix() : get_str_matrix();
    set_clean();
}



/*-------------------------------------
 * Pre-transform the matrix
-------------------------------------*/
void SL_Transform::apply_post_transform(const math::mat4& deltaTransform, bool useSRT) noexcept
{
    apply_transform(useSRT);
    mModelMat = mModelMat * deltaTransform;
}



/*-------------------------------------
 * Post-transform the matrix
-------------------------------------*/
void SL_Transform::apply_pre_transform(const math::mat4& deltaTransform, bool useSRT) noexcept
{
    apply_transform(useSRT);
    mModelMat = deltaTransform * mModelMat;
}



/*-------------------------------------
 * Extract the transformation parameters (math::mat3)
-------------------------------------*/
void SL_Transform::extract_transforms(math::mat3 rotationMatrix) noexcept
{
    // Scaling must be done uniformly, otherwise this will produce strange
    // results.
    mScaling[0] = math::length(rotationMatrix[0]);
    mScaling[1] = math::length(rotationMatrix[1]);
    mScaling[2] = math::length(rotationMatrix[2]);

    if (math::determinant(rotationMatrix) < 0.f)
    {
        mScaling = -mScaling;
    }

    if (mScaling[0])
    {
        rotationMatrix[0] /= mScaling[0];
    }
    if (mScaling[1])
    {
        rotationMatrix[1] /= mScaling[1];
    }
    if (mScaling[2])
    {
        rotationMatrix[2] /= mScaling[2];
    }

    mOrientation = math::mat_to_quat(rotationMatrix);

    set_dirty();
}



/*-------------------------------------
 * Extract the transformation parameters (math::mat4)
-------------------------------------*/
void SL_Transform::extract_transforms(const math::mat4& newTransform) noexcept
{
    mPosition[0] = newTransform[3][0];
    mPosition[1] = newTransform[3][1];
    mPosition[2] = newTransform[3][2];

    // delegate for maintenance purposes
    extract_transforms(math::mat3{newTransform});
}



/*-------------------------------------
 * Generate a SRT matrix for use in *this
-------------------------------------*/
math::mat4 SL_Transform::get_rst_matrix() const noexcept
{
    return math::mat4{
        mScaling[0], 0.f, 0.f, 0.f,
        0.f, mScaling[1], 0.f, 0.f,
        0.f, 0.f, mScaling[2], 0.f,
        mPosition[0], mPosition[1], mPosition[2], 1.f
    } * math::quat_to_mat4(mOrientation);
}



/*-------------------------------------
 * Generate a TRS Matrix
-------------------------------------*/
math::mat4 SL_Transform::get_str_matrix() const noexcept
{
    return math::quat_to_mat4(mOrientation) * math::mat4{
        mScaling[0], 0.f, 0.f, 0.f,
        0.f, mScaling[1], 0.f, 0.f,
        0.f, 0.f, mScaling[2], 0.f,
        mPosition[0], mPosition[1], mPosition[2], 1.f
    };
}



/*-----------------------------------------------------------------------------
 * Axis Orientations
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Get the forward direction
-------------------------------------*/
math::vec3 SL_Transform::forward_direction() const noexcept
{
    return mType == SL_TRANSFORM_TYPE_MODEL
           ? math::get_z_axis(mOrientation)
           : extractMVVector(mModelMat, SL_ModelViewAxis::MV_Z_AXIS);
}



/*-------------------------------------
 * Retrieve the camera's up vector
-------------------------------------*/
math::vec3 SL_Transform::up_direction() const noexcept
{
    return mType == SL_TRANSFORM_TYPE_MODEL
           ? math::get_y_axis(mOrientation)
           : extractMVVector(mModelMat, SL_ModelViewAxis::MV_Y_AXIS);
}



/*-------------------------------------
 * Retrieve the camera's right vector
 *
 * TODO: Test this, make sure it's not returning left-handed coordinates
-------------------------------------*/
math::vec3 SL_Transform::right_direction() const noexcept
{
    return mType == SL_TRANSFORM_TYPE_MODEL
           ? math::get_x_axis(mOrientation)
           : extractMVVector(mModelMat, SL_ModelViewAxis::MV_X_AXIS);
}



/*-------------------------------------
 * Y-Axis Locking
-------------------------------------*/
void SL_Transform::lock_y_axis(const bool isLocked) noexcept
{
    LS_DEBUG_ASSERT(mType != SL_TRANSFORM_TYPE_MODEL);

    if (isLocked)
    {
        if (mType == SL_TRANSFORM_TYPE_VIEW_ARC)
        {
            mType = SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y;
        }
        else if (mType == SL_TRANSFORM_TYPE_VIEW_FPS)
        {
            mType = SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y;
        }
    }
    else
    {
        if (mType == SL_TRANSFORM_TYPE_VIEW_ARC_LOCKED_Y)
        {
            mType = SL_TRANSFORM_TYPE_VIEW_ARC;
        }
        else if (mType == SL_TRANSFORM_TYPE_VIEW_FPS_LOCKED_Y)
        {
            mType = SL_TRANSFORM_TYPE_VIEW_FPS;
        }
    }
}



/*-------------------------------------
 * Looking at targets
-------------------------------------*/
void SL_Transform::look_at(const math::vec3& eye, const math::vec3& target, const math::vec3& up, bool absolutePosition)
{
    if (absolutePosition)
    {
        extract_transforms(math::look_at(eye, target, up));
    }
    else
    {
        extract_transforms(math::look_from(eye, target, up));
    }
}
