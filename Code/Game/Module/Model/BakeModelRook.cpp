#include "BakeModelRook.hpp"

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakeModelRook::BakeModelRook()
{
    name = "Rook";
}

BakeModelRook::~BakeModelRook()
{
}

void BakeModelRook::Build()
{
    BakedModel::Build();
    std::vector<Vertex_PCUTBN>& verts   = m_vertices[0];
    std::vector<unsigned int>&  indices = m_indices[0];

    ZCylinder cylinderBase;
    cylinderBase.m_center = Vec3(0.f, 0.f, 0.f);
    cylinderBase.m_radius = 0.40f;
    cylinderBase.m_height = 0.25f;
    cylinderBase.BuildVertices(verts, indices);

    AABB3 cubeMiddle;
    cubeMiddle.SetCenter(Vec3(0.f, 0.f, 0.34f));
    cubeMiddle.SetDimensions(Vec3(0.4f, 0.4f, 0.6f));
    cubeMiddle.BuildVertices(verts, indices);

    AABB3 cubeTop1;
    cubeMiddle.SetCenter(Vec3(0.f, 0.f, 0.65f));
    cubeMiddle.SetDimensions(Vec3(0.45f, 0.45f, 0.15f));
    cubeMiddle.BuildVertices(verts, indices);

    AABB3 cubeTop2;
    cubeMiddle.SetCenter(Vec3(0.f, 0.f, 0.75f));
    cubeMiddle.SetDimensions(Vec3(0.4f, 0.4f, 0.1f));
    cubeMiddle.BuildVertices(verts, indices);
}
