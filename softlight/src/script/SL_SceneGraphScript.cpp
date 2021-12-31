//
// Created by hammy on 2021-10-03.
//

#include "lightsky/utils/Log.h"

#include "lightsky/script/ScriptFactory.h"

#include "softlight/SL_Camera.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/script/SL_SceneGraphScript.hpp"



/*-----------------------------------------------------------------------------
 * Scene Graph Helper Functions for Serialization
-----------------------------------------------------------------------------*/
bool _sl_serialize_camera(std::ostream& ostr, const SL_Camera& cam)
{
    {
        uint8_t isDirty = cam.is_dirty() ? 0xFF : 0x00;
        ostr.write((const char*)&isDirty, sizeof(isDirty));
        if (!ostr.good())
        {
            return false;
        }
    }

    if (ostr.good())
    {
        uint32_t projType = static_cast<uint32_t>(cam.projection_type());
        ostr.write((const char*)&projType, sizeof(projType));
    }

    if (ostr.good())
    {
        float fov = cam.fov();
        ostr.write((const char*)&fov, sizeof(fov));
    }

    if (ostr.good())
    {
        float aspectW = cam.aspect_width();
        ostr.write((const char*)&aspectW, sizeof(aspectW));
    }

    if (ostr.good())
    {
        float aspectH = cam.aspect_height();
        ostr.write((const char*)&aspectH, sizeof(aspectH));
    }

    if (ostr.good())
    {
        float zNear = cam.near_plane();
        ostr.write((const char*)&zNear, sizeof(zNear));
    }

    if (ostr.good())
    {
        float zFar = cam.far_plane();
        ostr.write((const char*)&zFar, sizeof(zFar));
    }

    return ostr.good();
}

bool _sl_deserialize_camera(std::istream& istr, SL_Camera& cam)
{
    uint8_t isDirty = 0;
    istr.read(reinterpret_cast<char*>(&isDirty), sizeof(isDirty));
    if (!istr.good())
    {
        return false;
    }

    uint32_t projType = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&projType), sizeof(projType));
    }

    float fov = 0.f;
    float aspectW = 0.f;
    float aspectH = 0.f;
    float zNear = 0.f;
    float zFar = 0.f;

    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&fov), sizeof(fov));
    }

    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&aspectW), sizeof(aspectW));
    }

    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&aspectH), sizeof(aspectH));
    }

    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&zNear), sizeof(zNear));
    }

    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&zFar), sizeof(zFar));
    }

    if (!istr.good())
    {
        return false;
    }

    if (isDirty)
    {
        cam.force_dirty();
    }

    if (projType == static_cast<uint32_t>(SL_ProjectionType::SL_PROJECTION_PERSPECTIVE))
    {
        projType = (uint32_t)SL_ProjectionType::SL_PROJECTION_PERSPECTIVE;
    }
    else if (projType == static_cast<uint32_t>(SL_ProjectionType::SL_PROJECTION_ORTHOGONAL))
    {
        projType = (uint32_t)SL_ProjectionType::SL_PROJECTION_ORTHOGONAL;
    }
    else if (projType == static_cast<uint32_t>(SL_ProjectionType::SL_PROJECTION_LOGARITHMIC_PERSPECTIVE))
    {
        projType = (uint32_t)SL_ProjectionType::SL_PROJECTION_LOGARITHMIC_PERSPECTIVE;
    }
    else
    {
        projType = (uint32_t)SL_ProjectionType::SL_PROJECTION_DEFAULT;
    }

    cam.projection_type(static_cast<SL_ProjectionType>(projType));
    cam.fov(fov);
    cam.aspect_ratio(aspectW, aspectH);
    cam.near_plane(zNear);
    cam.far_plane(zFar);

    return true;
}



/*-----------------------------------------------------------------------------
 * Scene Graph Load/Save Implementations
-----------------------------------------------------------------------------*/
LS_SCRIPT_DEFINE_VAR(SL_SceneGraphScript, SL_SceneGraph*);

/*-------------------------------------
 * Scene Graph Serialization
-------------------------------------*/
LS_SCRIPT_OVERRIDE_VAR_SAVE(LS_SCRIPT_STATIC_API, SL_SceneGraph*)
{
    const SL_SceneGraph* pGraph = data;
    size_t numCams = pGraph->mCameras.size();

    ostr.write(reinterpret_cast<const char*>(&numCams), sizeof numCams);
    if (ostr.good())
    {
        for (const SL_Camera& cam : pGraph->mCameras)
        {
            if (!_sl_serialize_camera(ostr, cam))
            {
                LS_LOG_ERR("Unable to complete camera serialization.");
                return false;
            }
        }
    }

    return ostr.good();
}

/*-------------------------------------
 * Scene Graph Deserialization
-------------------------------------*/
LS_SCRIPT_OVERRIDE_VAR_LOAD(LS_SCRIPT_EXPORT_API, SL_SceneGraph*)
{
    (void)varImporter;
    (void)funcImporter;
    SL_SceneGraph* pGraph = data;

    size_t numCams = 0;
    istr.read(reinterpret_cast<char*>(&numCams), sizeof numCams);
    if (istr.good())
    {
        pGraph->mCameras.resize(numCams);
        for (SL_Camera& cam : pGraph->mCameras)
        {
            if (!_sl_deserialize_camera(istr, cam))
            {
                LS_LOG_ERR("Unable to complete camera de-serialization.");
                return false;
            }
        }
    }

    return istr.good() || istr.eof();
}
