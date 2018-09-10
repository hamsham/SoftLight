
#include <utility>

#include "soft_render/SR_BoundingBox.hpp"



namespace math = ls::math;



/*-------------------------------------
    Constructor
-------------------------------------*/
SR_BoundingBox::SR_BoundingBox() noexcept :
    mTopRearRight{1.f, 1.f, 1.f},
    mBotFrontLeft{-1.f, -1.f, -1.f}
{}



/*-------------------------------------
    Copy Constructor
-------------------------------------*/
SR_BoundingBox::SR_BoundingBox(const SR_BoundingBox& bb) noexcept :
    mTopRearRight{bb.mTopRearRight},
    mBotFrontLeft{bb.mBotFrontLeft}
{}



/*-------------------------------------
    Move Constructor
-------------------------------------*/
SR_BoundingBox::SR_BoundingBox(SR_BoundingBox&& bb) noexcept :
    mTopRearRight{std::move(bb.mTopRearRight)},
    mBotFrontLeft{std::move(bb.mBotFrontLeft)}
{
    bb.reset_size();
}



/*-------------------------------------
    Copy Operator
-------------------------------------*/
SR_BoundingBox& SR_BoundingBox::operator=(const SR_BoundingBox& bb) noexcept
{
    mTopRearRight = bb.mTopRearRight;
    mBotFrontLeft = bb.mBotFrontLeft;

    return *this;
}



/*-------------------------------------
    Move Operator
-------------------------------------*/
SR_BoundingBox& SR_BoundingBox::operator=(SR_BoundingBox&& bb) noexcept
{
    mTopRearRight = std::move(bb.mTopRearRight);
    mBotFrontLeft = std::move(bb.mBotFrontLeft);

    bb.reset_size();

    return *this;
}



/*-------------------------------------
    Check if a portion of another bounding box is within *this.
-------------------------------------*/
bool SR_BoundingBox::is_in_box(const math::vec3& v) const noexcept
{
    return
        v[0] <= mTopRearRight[0] && v[1] <= mTopRearRight[1] && v[2] <= mTopRearRight[2]
        &&
        v[0] >= mBotFrontLeft[0] && v[1] >= mBotFrontLeft[1] && v[2] >= mBotFrontLeft[2];
}



/*-------------------------------------
    Reset the bounds of this bounding box to their default values.
-------------------------------------*/
void SR_BoundingBox::reset_size() noexcept
{
    set_top_rear_right(math::vec3{1.f, 1.f, 1.f});
    set_bot_front_left(math::vec3{-1.f, -1.f, -1.f});
}



/*-------------------------------------
    Compare a point to the current set of vertices.
-------------------------------------*/
void SR_BoundingBox::compare_and_update(const math::vec3& point) noexcept
{
    math::vec3& trr = mTopRearRight;
    if (point[0] > trr[0])
    {
        trr[0] = point[0];
    }
    if (point[1] > trr[1])
    {
        trr[1] = point[1];
    }
    if (point[2] > trr[2])
    {
        trr[2] = point[2];
    }

    math::vec3& bfl = mBotFrontLeft;
    if (point[0] < bfl[0])
    {
        bfl[0] = point[0];
    }
    if (point[1] < bfl[1])
    {
        bfl[1] = point[1];
    }
    if (point[2] < bfl[2])
    {
        bfl[2] = point[2];
    }
}
