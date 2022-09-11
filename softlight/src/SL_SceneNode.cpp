
#include "softlight/SL_SceneNode.hpp"



/*-------------------------------------
    Reset SL_SceneNode
-------------------------------------*/
void sl_reset(SL_SceneNode& n) noexcept
{
    n.type = SL_SceneNodeType::NODE_TYPE_EMPTY;
    n.dataId = SL_SceneNodeProp::SCENE_NODE_ROOT_ID;
}



/*-------------------------------------
    Reset SL_SceneNode
-------------------------------------*/
void sl_reset(SL_SkeletonIndex& s) noexcept
{
    s.index = SL_SceneNodeProp::SCENE_NODE_ROOT_ID;
    s.count = 0;
}
