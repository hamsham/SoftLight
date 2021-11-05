//
// Created by hammy on 2021-10-03.
//

#include "lightsky/script/ScriptFactory.h"

#include "softlight/SL_SceneGraph.hpp"
#include "softlight/script/SL_SceneGraphScript.hpp"



LS_SCRIPT_DEFINE_VAR(SL_SceneGraphScript, SL_SceneGraph*);

/*-------------------------------------
 * Scene Graph Serialization
-------------------------------------*/
LS_SCRIPT_OVERRIDE_VAR_SAVE(LS_STATIC_API, SL_SceneGraph*)
{
    const SL_SceneGraph* pGraph = data;
    ostr << (const void*)pGraph;
    return ostr.good();
}

/*-------------------------------------
 * Scene Graph Deserialization
-------------------------------------*/
LS_SCRIPT_OVERRIDE_VAR_LOAD(LS_EXPORT_API, SL_SceneGraph*)
{
    (void)varImporter;
    (void)funcImporter;
    istr.read((char*)data, sizeof(SL_SceneGraph*));

    return istr.good() || istr.eof();
}
