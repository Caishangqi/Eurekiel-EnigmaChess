#include "BakeModelPawn.hpp"

#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakeModelPawn::BakeModelPawn()
{
    name = "Pawn";
}

BakeModelPawn::~BakeModelPawn()
{
}

void BakeModelPawn::Build()
{
    BakedModel::Build();
    std::vector<Vertex_PCUTBN>& verts   = m_vertices[0];
    std::vector<unsigned int>&  indices = m_indices[0];

    ZCylinder cylinderBase;
    cylinderBase.m_center = Vec3(0.f, 0.f, 0.f);
    cylinderBase.m_radius = 0.35f;
    cylinderBase.m_height = 0.25f;
    cylinderBase.BuildVertices(verts, indices);

    ZCylinder cylinderMiddle;
    cylinderMiddle.m_center = Vec3(0.f, 0.f, 0.25f);
    cylinderMiddle.m_radius = 0.20f;
    cylinderMiddle.m_height = 0.5f;
    cylinderMiddle.BuildVertices(verts, indices);

    ZCylinder cylinderTop;
    cylinderTop.m_center = Vec3(0.f, 0.f, 0.5f);
    cylinderTop.m_radius = 0.25f;
    cylinderTop.m_height = 0.1f;
    cylinderTop.BuildVertices(verts, indices);

    Sphere sphere;
    sphere.m_position = Vec3(0.f, 0.f, 0.65f);
    sphere.m_radius   = 0.20f;
    sphere.BuildVertices(verts, indices, 256);
}
