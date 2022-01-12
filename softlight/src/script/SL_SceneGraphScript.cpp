//
// Created by hammy on 2021-10-03.
//

#include "lightsky/utils/Log.h"

#include "lightsky/script/ScriptFactory.h"

#include "softlight/SL_BoundingBox.hpp"
#include "softlight/SL_Camera.hpp"
#include "softlight/SL_Mesh.hpp"
#include "softlight/SL_SceneGraph.hpp"
#include "softlight/script/SL_SceneGraphScript.hpp"



/*-----------------------------------------------------------------------------
 * Scene Graph Helper Functions for Serialization
-----------------------------------------------------------------------------*/
/*-------------------------------------
 * Camera Serialization
-------------------------------------*/
bool _sl_serialize_camera(std::ostream& ostr, const SL_Camera& cam)
{
    if (ostr.good())
    {
        uint8_t isDirty = cam.is_dirty() ? 0xFF : 0x00;
        ostr.write((const char*)&isDirty, sizeof(isDirty));
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

/*-------------------------------------
 * Camera Loading
-------------------------------------*/
bool _sl_deserialize_camera(std::istream& istr, SL_Camera& cam)
{
    uint8_t isDirty = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&isDirty), sizeof(isDirty));
    }

    uint32_t projType = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&projType), sizeof(projType));
    }

    float fov = 0.f;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&fov), sizeof(fov));
    }

    float aspectW = 0.f;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&aspectW), sizeof(aspectW));
    }

    float aspectH = 0.f;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&aspectH), sizeof(aspectH));
    }

    float zNear = 0.f;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&zNear), sizeof(zNear));
    }

    float zFar = 0.f;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&zFar), sizeof(zFar));
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

    return istr.good();
}



/*-------------------------------------
 * Mesh Serialization
-------------------------------------*/
bool _sl_serialize_mesh(std::ostream& ostr, const SL_Mesh& mesh)
{
    if (ostr.good())
    {
        uint64_t vaoId = static_cast<uint64_t>(mesh.vaoId);
        ostr.write((const char*)&vaoId, sizeof(vaoId));
    }

    if (ostr.good())
    {
        uint64_t elementBegin = static_cast<uint64_t>(mesh.elementBegin);
        ostr.write((const char*)&elementBegin, sizeof(elementBegin));
    }

    if (ostr.good())
    {
        uint64_t elementEnd = static_cast<uint64_t>(mesh.elementEnd);
        ostr.write((const char*)&elementEnd, sizeof(elementEnd));
    }

    if (ostr.good())
    {
        uint64_t mode = static_cast<uint64_t>(mesh.mode);
        ostr.write((const char*)&mode, sizeof(mode));
    }

    if (ostr.good())
    {
        uint64_t materialId = static_cast<uint64_t>(mesh.materialId);
        ostr.write((const char*)&materialId, sizeof(materialId));
    }

    return ostr.good();
}

/*-------------------------------------
 * Mesh Loading
-------------------------------------*/
bool _sl_deserialize_mesh(std::istream& istr, SL_Mesh& mesh)
{
    uint64_t vaoId = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&vaoId), sizeof(vaoId));
    }

    uint64_t elementBegin = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&elementBegin), sizeof(elementBegin));
    }

    uint64_t elementEnd = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&elementEnd), sizeof(elementEnd));
    }

    uint64_t mode = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&mode), sizeof(mode));
    }

    uint64_t materialId = 0;
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(&materialId), sizeof(materialId));
    }

    mesh.vaoId = static_cast<size_t>(vaoId);
    mesh.elementBegin = static_cast<size_t>(elementBegin);
    mesh.elementEnd = static_cast<size_t>(elementEnd);
    mesh.mode = static_cast<SL_RenderMode>(mode);
    mesh.materialId = static_cast<uint32_t>(materialId);

    return istr.good();
}



/*-------------------------------------
 * Bounding Box Serialization
-------------------------------------*/
bool _sl_serialize_bbox(std::ostream& ostr, const SL_BoundingBox& box)
{
    static_assert(sizeof(ls::math::vec4) == sizeof(uint32_t)*4, "4D vectors do not contain 32-bit floats!");

    if (ostr.good())
    {
        const ls::math::vec4 minPt = box.min_point();
        ostr.write((const char*)&minPt.v, sizeof(minPt));
    }

    if (ostr.good())
    {
        const ls::math::vec4 maxPt = box.max_point();
        ostr.write((const char*)&maxPt.v, sizeof(maxPt));
    }

    return ostr.good();
}

/*-------------------------------------
 * Bounding Box Loading
-------------------------------------*/
bool _sl_deserialize_bbox(std::istream& istr, SL_BoundingBox& box)
{
    ls::math::vec4 minPt{0.f};
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(minPt.v), sizeof(minPt));
    }

    ls::math::vec4 maxPt{0.f};
    if (istr.good())
    {
        istr.read(reinterpret_cast<char*>(maxPt.v), sizeof(maxPt));
    }

    box.min_point(minPt);
    box.max_point(maxPt);

    return istr.good();
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

    if (ostr.good())
    {
        const uint64_t numCams = pGraph->mCameras.size();
        ostr.write(reinterpret_cast<const char*>(&numCams), sizeof(numCams));

        for (const SL_Camera& cam : pGraph->mCameras)
        {
            if (!_sl_serialize_camera(ostr, cam))
            {
                LS_LOG_ERR("Unable to complete camera serialization.");
                return false;
            }
        }
    }

    if (ostr.good())
    {
        const uint64_t numMeshes = pGraph->mMeshes.size();
        ostr.write(reinterpret_cast<const char*>(&numMeshes), sizeof(numMeshes));

        for (const SL_Mesh& mesh : pGraph->mMeshes)
        {
            if (!_sl_serialize_mesh(ostr, mesh))
            {
                LS_LOG_ERR("Unable to complete mesh serialization.");
                return false;
            }
        }
    }

    if (ostr.good())
    {
        const uint64_t numBoxes = pGraph->mMeshBounds.size();
        ostr.write(reinterpret_cast<const char*>(&numBoxes), sizeof(numBoxes));

        for (const SL_BoundingBox& box : pGraph->mMeshBounds)
        {
            if (!_sl_serialize_bbox(ostr, box))
            {
                LS_LOG_ERR("Unable to complete bounding-box serialization.");
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

    if (istr.good())
    {
        uint64_t numCams = 0;
        istr.read(reinterpret_cast<char*>(&numCams), sizeof(numCams));
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

    if (istr.good())
    {
        uint64_t numMeshes = 0;
        istr.read(reinterpret_cast<char*>(&numMeshes), sizeof(numMeshes));
        pGraph->mMeshes.resize(numMeshes);

        for (SL_Mesh& mesh : pGraph->mMeshes)
        {
            sl_reset(mesh);

            if (!_sl_deserialize_mesh(istr, mesh))
            {
                LS_LOG_ERR("Unable to complete mesh de-serialization.");
                return false;
            }
        }
    }

    if (istr.good())
    {
        uint64_t numBoxes = 0;
        istr.read(reinterpret_cast<char*>(&numBoxes), sizeof(numBoxes));
        pGraph->mMeshBounds.resize(numBoxes);

        for (SL_BoundingBox& box : pGraph->mMeshBounds)
        {
            box.reset_size();

            if (!_sl_deserialize_bbox(istr, box))
            {
                LS_LOG_ERR("Unable to complete bounding-box de-serialization.");
                return false;
            }
        }
    }

    return istr.good() || istr.eof();
}
