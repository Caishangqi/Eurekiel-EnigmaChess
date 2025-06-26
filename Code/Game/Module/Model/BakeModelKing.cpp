#include "BakeModelKing.hpp"

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakeModelKing::BakeModelKing()
{
    name = "King";
}

BakeModelKing::~BakeModelKing()
{
}

void BakeModelKing::Build()
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
    cylinderMiddle.m_center = Vec3(0.f, 0.f, 0.65f);
    cylinderMiddle.m_radius = 0.25f;
    cylinderMiddle.m_height = 1.0f;
    cylinderMiddle.BuildVertices(verts, indices);

    ZCylinder cylinderTop;
    cylinderTop.m_center = Vec3(0.f, 0.f, 1.2f);
    cylinderTop.m_radius = 0.35f;
    cylinderTop.m_height = 0.16f;
    cylinderTop.BuildVertices(verts, indices);

    AABB3 topCube0;
    topCube0.SetCenter(Vec3(0.f, 0.f, 1.3f));
    topCube0.SetDimensions(Vec3(0.54f, 0.2f, 0.3f));
    topCube0.BuildVertices(verts, indices);

    AABB3 topCube1;
    topCube1.SetCenter(Vec3(0.f, 0.f, 1.3f));
    topCube1.SetDimensions(Vec3(0.2f, 0.54f, 0.29f));
    topCube1.BuildVertices(verts, indices);
}
