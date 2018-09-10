
#include "soft_render/SR_SceneNode.hpp"



/*-------------------------------------
    Reset
-------------------------------------*/
void sr_reset(SR_SceneNode& n) noexcept
{
    n.type = SR_SceneNodeType::NODE_TYPE_EMPTY;
    n.nodeId = SR_SceneNodeProp::SCENE_NODE_ROOT_ID;
    n.dataId = SR_SceneNodeProp::SCENE_NODE_ROOT_ID;
    n.animListId = SR_SceneNodeProp::SCENE_NODE_ROOT_ID;
}
