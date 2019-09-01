
#ifndef SR_BOUNDING_BOX_HPP
#define SR_BOUNDING_BOX_HPP

#include "lightsky/math/vec3.h"
#include "lightsky/math/vec4.h"
#include "lightsky/math/vec_utils.h"



/**
 * @brief Bounding Box Class
 */
class SR_BoundingBox
{
  private:
    ls::math::vec4 mMaxPoint;

    ls::math::vec4 mMinPoint;

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
     * Set the maximum extent of this bounding box.
     *
     * @param A constant reference to a point that will be used as the maximum
     * ponit of this bounding box.
     */
    void max_point(const ls::math::vec3& v) noexcept;

    /**
     * Set the tmaximum extent of this bounding box.
     *
     * @param A constant reference to a point that will be used as the maximum
     * point of this bounding box.
     */
    void max_point(const ls::math::vec4& v) noexcept;

    /**
     * Get the maximum extent of this bounding box.
     *
     * @return A constant reference to the maximum point of this bounding box.
     */
    const ls::math::vec4& max_point() const noexcept;

    /**
     * Set the minimum extent of this bounding box.
     *
     * @param A constant reference to a point that will be used as the min
     * point of this bounding box.
     */
    void min_point(const ls::math::vec3& v) noexcept;

    /**
     * Set the minimum extent of this bounding box.
     *
     * @param A constant reference to a point that will be used as the min
     * point of this bounding box.
     */
    void min_point(const ls::math::vec4& v) noexcept;

    /**
     * Get the minimum extent of this bounding box.
     *
     * @return A constant reference to the min point of this bounding box.
     */
    const ls::math::vec4& min_point() const noexcept;

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
    mMaxPoint{1.f, 1.f, 1.f, 1.f},
    mMinPoint{-1.f, -1.f, -1.f, 1.f}
{}



/*-------------------------------------
    Copy Constructor
-------------------------------------*/
inline SR_BoundingBox::SR_BoundingBox(const SR_BoundingBox& bb) noexcept :
    mMaxPoint{bb.mMaxPoint},
    mMinPoint{bb.mMinPoint}
{}



/*-------------------------------------
    Move Constructor
-------------------------------------*/
inline SR_BoundingBox::SR_BoundingBox(SR_BoundingBox&& bb) noexcept :
    mMaxPoint{std::move(bb.mMaxPoint)},
    mMinPoint{std::move(bb.mMinPoint)}
{
    bb.reset_size();
}



/*-------------------------------------
    Copy Operator
-------------------------------------*/
inline SR_BoundingBox& SR_BoundingBox::operator=(const SR_BoundingBox& bb) noexcept
{
    mMaxPoint = bb.mMaxPoint;
    mMinPoint = bb.mMinPoint;

    return *this;
}



/*-------------------------------------
    Move Operator
-------------------------------------*/
inline SR_BoundingBox& SR_BoundingBox::operator=(SR_BoundingBox&& bb) noexcept
{
    mMaxPoint = std::move(bb.mMaxPoint);
    mMinPoint = std::move(bb.mMinPoint);

    bb.reset_size();

    return *this;
}



/*-------------------------------------
    Check if a portion of another bounding box is within *this.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const ls::math::vec3& v) const noexcept
{
    return
        v[0] < mMaxPoint[0] && v[1] < mMaxPoint[1] && v[2] < mMaxPoint[2]
        &&
        v[0] >= mMinPoint[0] && v[1] >= mMinPoint[1] && v[2] >= mMinPoint[2];
}



/*-------------------------------------
    Check if a portion of another bounding box is within *this.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const ls::math::vec4& v) const noexcept
{
    return v < mMaxPoint && v >= mMinPoint;
}



/*-------------------------------------
    Check if a point is within this box.
-------------------------------------*/
inline bool SR_BoundingBox::is_in_box(const SR_BoundingBox& bb) const noexcept
{
    return is_in_box(bb.mMaxPoint) || is_in_box(bb.mMinPoint);
}



/*-------------------------------------
    Set the max point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::max_point(const ls::math::vec3& v) noexcept
{
    mMaxPoint = ls::math::vec4{v[0], v[1], v[2], 1.f};
}



/*-------------------------------------
    Set the max point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::max_point(const ls::math::vec4& v) noexcept
{
    mMaxPoint = v;
}



/*-------------------------------------
    Get the max point of this bounding box.
-------------------------------------*/
inline const ls::math::vec4& SR_BoundingBox::max_point() const noexcept
{
    return mMaxPoint;
}



/*-------------------------------------
    Set the min point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::min_point(const ls::math::vec3& v) noexcept
{
    mMinPoint = ls::math::vec4{v[0], v[1], v[2], 1.f};
}



/*-------------------------------------
    Set the min point of this bounding box.
-------------------------------------*/
inline void SR_BoundingBox::min_point(const ls::math::vec4& v) noexcept
{
    mMinPoint = v;
}



/*-------------------------------------
    Get the min point of this bounding box.
-------------------------------------*/
inline const ls::math::vec4& SR_BoundingBox::min_point() const noexcept
{
    return mMinPoint;
}



/*-------------------------------------
    Reset the bounds of this bounding box to their default values.
-------------------------------------*/
inline void SR_BoundingBox::reset_size() noexcept
{
    max_point(ls::math::vec4{1.f, 1.f, 1.f, 1.f});
    min_point(ls::math::vec4{-1.f, -1.f, -1.f, 1.f});
}



/*-------------------------------------
    Compare a point to the current set of vertices.
-------------------------------------*/
inline void SR_BoundingBox::compare_and_update(const ls::math::vec3& point) noexcept
{
    compare_and_update(ls::math::vec4{point[0], point[1], point[2], 1.f});
}



/*-------------------------------------
    Compare a point to the current set of vertices.
-------------------------------------*/
inline void SR_BoundingBox::compare_and_update(const ls::math::vec4& point) noexcept
{
    mMaxPoint = ls::math::max(mMaxPoint, point);
    mMinPoint = ls::math::min(mMinPoint, point);
}



#endif  /* SR_BOUNDING_BOX_HPP */
