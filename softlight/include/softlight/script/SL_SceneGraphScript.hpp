//
// Created by hammy on 2021-10-03.
//

#ifndef SL_SCENE_GRAPH_SCRIPT_HPP
#define SL_SCENE_GRAPH_SCRIPT_HPP

#include "lightsky/script/ScriptVariable.h"

class SL_SceneGraph;



LS_SCRIPT_OVERRIDE_VAR_LOAD(LS_STATIC_API, SL_SceneGraph*);

LS_SCRIPT_OVERRIDE_VAR_SAVE(LS_STATIC_API, SL_SceneGraph*);

LS_SCRIPT_DECLARE_VAR(SL_SceneGraphScript, SL_SceneGraph*);



#endif /* SL_SCENE_GRAPH_SCRIPT_HPP */
