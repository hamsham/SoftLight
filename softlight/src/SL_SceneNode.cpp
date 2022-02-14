
#include "softlight/SL_SceneNode.hpp"



/*-------------------------------------
    Reset
-------------------------------------*/
void sl_reset(SL_SceneNode& n) noexcept
{
    n.type = SL_SceneNodeType::NODE_TYPE_EMPTY;
    n.dataId = SL_SceneNodeProp::SCENE_NODE_ROOT_ID;
}
