
#ifndef SR_BOUNDING_BOX_HPP
#define SR_BOUNDING_BOX_HPP

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"



/**
 * @brief Bounding Box Class
 *
 * The orientation for a bounding box follows OpenGL coordinates, where the
 * positive XYZ coordinates point towards the top, right, front of the viewport.
 */
class SR_BoundingBox
{
  private:
    ls::math::vec4 mTopRearRight;

    ls::math::vec4 mBotFrontLeft;

  public:
    /**
     * @brief Destructor
     *
     * Defaulted
     */
    ~SR_BoundingBox() noexcept = default;

    /**
     * @brief Constructor
     */
    SR_BoundingBox() noexcept;

    /**
     * @brief Copy Constructor
     *
     * Copies data from another bounding box.
     *
     * @param bb
     * A constant reference to a fully constructed bounding box
     * object.
     */
    SR_BoundingBox(const SR_BoundingBox& bb) noexcept;

    /**
     * @brief Move Constructor
     *
     * Copies data from another bounding box (no moves are performed).
     *
     * @param An r-value reference to a fully constructed bounding box
     */
    SR_BoundingBox(SR_BoundingBox&&) noexcept;

    /**
     * @brief Copy Operator
     *
     * Copies data from another bounding box.
     *
     * @param A constant reference to a fully constructed bounding box
     * object.
     *
     * @return A reference to *this.
     */
    SR_BoundingBox& operator=(const SR_BoundingBox&) noexcept;

    /**
     * @brief Move Operator
     *
     * @param An R-Value reference to a bounding box that is about to go
     * out of scope.
     *
     * @return A reference to *this.
     */
    SR_BoundingBox& operator=(SR_BoundingBox&&) noexcept;

    /**
     * @brief Check if a point is within this box.
     *
     * @param A constant reference to a vec3 object.
     *
     * @return TRUE if the point is within *this, or FALSE if otherwise.
     */
    bool is_in_box(const ls::math::vec3&) const noexcept;

    /**
     * @brief Check if a point is within this box.
     *
     * @param A constant reference to a vec4 object.
     *
     * @return TRUE if the point is within *this, or FALSE if otherwise.
     */
    bool is_in_box(const ls::math::vec4&) const noexcept;

    /**
     * Check if a portion of another bounding box is within *this.
     *
     * @param A constant reference to another bounding box.
     *
     * @return TRUE if a portion of the bounding box is within *this, or
     * FALSE if it isn't.
     */
    bool is_in_box(const SR_BoundingBox&) const noexcept;

    /**
     * Set the top-rear-right point of this bounding box.
     *
     * @param A constant reference to a point that will be used as the top,
     * rear, right point of this bounding box.
     */
    void set_top_rear_right(const ls::math::vec3&) noexcept;

    /**
     * Set the top-rear-right point of this bounding box.
     *
     * @param A constant reference to a point that will be used as the top,
     * rear, right point of this bounding box.
     */
    void set_top_rear_right(const ls::math::vec4&) noexcept;

    /**
     * Get the top-rear-right point of this bounding box.
     *
     * @return A constant reference to the top, rear, right point of this
     * bounding box.
     */
    const ls::math::vec4& get_top_rear_right() const noexcept;

    /**
     * Set the bottom, front, left point of this bounding box.
     *
     * @param A constant reference to a point that will be used as the
     * bottom, front, left point of this bounding box.
     */
    void set_bot_front_left(const ls::math::vec3&) noexcept;

    /**
     * Set the bottom, front, left point of this bounding box.
     *
     * @param A constant reference to a point that will be used as the
     * bottom, front, left point of this bounding box.
     */
    void set_bot_front_left(const ls::math::vec4&) noexcept;

    /**
     * Get the bottom, front, left point of this bounding box.
     *
     * @return A constant reference to the bottom, front, left point of this
     * bounding box.
     */
    const ls::math::vec4& get_bot_front_left() const noexcept;

    /**
     * Reset the bounds of this bounding box to their default values.
     */
    void reset_size() noexcept;

    /**
     * Compare a point to the current set of vertices.
     * If any of the components within the parameter are larger than the
     * components of this box, the current set of points will be enlarged.
     *
     * @param point
     * A point who's individual components should be used to update the
     * size of this bounding box.
     */
    void compare_and_update(const ls::math::vec3& point) noexcept;

    /**
     * Compare a point to the current set of vertices.
     * If any of the components within the parameter are larger than the
     * components of this box, the current set of points will be enlarged.
     *
     * @param point
     * A point who's individual components should be used to update the
     * size of this bounding box.
     */
    void compare_and_update(const ls::math::vec4& point) noexcept;
};



/*-------------------------------------
    Constructor
-------------------------------------*/
inline SR_BoundingBox::SR_BoundingBox() noexcept :
    mTopRearRight{1.f, 1.f, 1.f, 0.f},
    mBotFrontLeft{-1.f, -1.f, -1.f, 0.f}
{}



/*-------------------------------------
    Copy Constructor
-------------------------------------*/
inline SR_BoundingBox::SR_BoundingBox(const SR_BoundingBox& bb) noexcept :
    mTopRearRight{bb.mTopRearRight},
    mBotFrontLeft{bb.mBotFrontLeft}
{}



/*-------------------------------------
    Move Constructor
-------------------------------------*/
inline SR_BoundingBox::SR_BoundingBox(SR_BoundingBox&& bb) noexcept :
    mTopRearRight{std::move(bb.mTopRearRight)},
    mBotFrontLeft{std::move(bb.mBotFrontLeft)}
{
    bb.reset_size();
}



/*-------------------------------------
    Copy Operator
-------------------------------------*/
inline SR_BoundingBox& SR_BoundingBox::operator=(const SR_BoundingBox& bb) noexcept
{
    mTopRearRight = bb.mTopRearRight;
    mBotFrontLeft = bb.mBotFrontLeft;

    return *this;
}



/*-------------------------------------
    Move Operator
-------------------------------------*/
inline SR_BoundingBox& SR_BoundingBox::operator=(SR_BoundingBox&& bb) noexcept
{
    mTopRearRight = std::move(bb.mTopRearRight);
    mBotFrontLeft = std::move(bb.mBotFrontLeft);

    bb.reset_size();

    return *this;
}



/*-------------------------------------
    Check if a portion of another bounding box is within *this.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const ls::math::vec3& v) const noexcept
{
    return
        v[0] < mTopRearRight[0] && v[1] < mTopRearRight[1] && v[2] < mTopRearRight[2]
        &&
        v[0] >= mBotFrontLeft[0] && v[1] >= mBotFrontLeft[1] && v[2] >= mBotFrontLeft[2];
}



/*-------------------------------------
    Check if a portion of another bounding box is within *this.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const ls::math::vec4& v) const noexcept
{
    return v < mTopRearRight && v >= mBotFrontLeft;
}



/*-------------------------------------
    Check if a point is within this box.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const SR_BoundingBox& bb) const noexcept
{
    return is_in_box(bb.mTopRearRight) || is_in_box(bb.mBotFrontLeft);
}



/*-------------------------------------
    Set the top-rear-right point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::set_top_rear_right(const ls::math::vec3& v) noexcept
{
    mTopRearRight = ls::math::vec4{v[0], v[1], v[2], 0.f};
}



/*-------------------------------------
    Set the top-rear-right point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::set_top_rear_right(const ls::math::vec4& v) noexcept
{
    mTopRearRight = v;
}



/*-------------------------------------
    Get the top-rear-right point of this bounding box.
-------------------------------------*/
inline const ls::math::vec4& SR_BoundingBox::get_top_rear_right() const noexcept
{
    return mTopRearRight;
}



/*-------------------------------------
    Set the bottom, front, left point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::set_bot_front_left(const ls::math::vec3& v) noexcept
{
    mBotFrontLeft = ls::math::vec4{v[0], v[1], v[2], 0.f};
}



/*-------------------------------------
    Set the bottom, front, left point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::set_bot_front_left(const ls::math::vec4& v) noexcept
{
    mBotFrontLeft = v;
}



/*-------------------------------------
    Get the bottom, front, left point of this bounding box.
-------------------------------------*/
inline const ls::math::vec4& SR_BoundingBox::get_bot_front_left() const noexcept
{
    return mBotFrontLeft;
}



/*-------------------------------------
    Reset the bounds of this bounding box to their default values.
-------------------------------------*/
inline void SR_BoundingBox::reset_size() noexcept
{
    set_top_rear_right(ls::math::vec4{1.f, 1.f, 1.f, 0.f});
    set_bot_front_left(ls::math::vec4{-1.f, -1.f, -1.f, 0.f});
}



/*-------------------------------------
    Compare a point to the current set of vertices.
-------------------------------------*/
inline void SR_BoundingBox::compare_and_update(const ls::math::vec3& point) noexcept
{
    compare_and_update(ls::math::vec4{point[0], point[1], point[2], 0.f});
}



/*-------------------------------------
    Compare a point to the current set of vertices.
-------------------------------------*/
inline void SR_BoundingBox::compare_and_update(const ls::math::vec4& point) noexcept
{
    mTopRearRight = ls::math::max(mTopRearRight, point);
    mBotFrontLeft = ls::math::min(mBotFrontLeft, point);
}



#endif  /* SR_BOUNDING_BOX_HPP */
