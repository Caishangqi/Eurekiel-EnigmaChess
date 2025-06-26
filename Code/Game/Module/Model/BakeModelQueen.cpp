#include "BakeModelQueen.hpp"

#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakeModelQueen::BakeModelQueen()
{
    name = "Queen";
}

BakeModelQueen::~BakeModelQueen()
{
}

void BakeModelQueen::Build()
{
    BakedModel::Build();
    std::vector<Vertex_PCUTBN>& verts   = m_vertices[0];
    std::vector<unsigned int>&  indices = m_indices[0];
    ZCylinder                   cylinderBase;
    cylinderBase.m_center = Vec3(0.f, 0.f, 0.f);
    cylinderBase.m_radius = 0.40f;
    cylinderBase.m_height = 0.25f;
    cylinderBase.BuildVertices(verts, indices);

    ZCylinder cylinderMiddle;
    cylinderMiddle.m_center = Vec3(0.f, 0.f, 0.55f);
    cylinderMiddle.m_radius = 0.25f;
    cylinderMiddle.m_height = 0.8f;
    cylinderMiddle.BuildVertices(verts, indices);

    ZCylinder cylinderTop;
    cylinderTop.m_center = Vec3(0.f, 0.f, 1.0f);
    cylinderTop.m_radius = 0.35f;
    cylinderTop.m_height = 0.16f;
    cylinderTop.BuildVertices(verts, indices);

    //
    auto angles0 = EulerAngles();
    OBB3 obbTop0;
    obbTop0.m_center = Vec3(0.f, 0.f, 1.1f);
    obbTop0.SetOrientation(angles0);
    obbTop0.m_halfDimensions = Vec3(0.06f, 0.25f, 0.10f);
    obbTop0.BuildVertices(verts, indices);

    while (angles0.m_yawDegrees < 240.f)
    {
        angles0.m_yawDegrees += 60.f;
        obbTop0.SetOrientation(angles0);
        obbTop0.BuildVertices(verts, indices);
    }
}
