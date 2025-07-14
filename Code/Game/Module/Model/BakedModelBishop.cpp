#include "BakedModelBishop.hpp"

#include "Engine/Math/Sphere.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakedModelBishop::BakedModelBishop()
{
    name = "Bishop";
}

BakedModelBishop::~BakedModelBishop()
{
}

void BakedModelBishop::Build()
{
    BakedModel::Build();
    std::vector<Vertex_PCUTBN>& verts   = m_vertices[0];
    std::vector<unsigned int>&  indices = m_indices[0];

    ZCylinder cylinderBase;
    cylinderBase.m_center = Vec3(0.f, 0.f, 0.f);
    cylinderBase.m_radius = 0.40f;
    cylinderBase.m_height = 0.25f;
    cylinderBase.BuildVertices(verts, indices);

    ZCylinder cylinderMiddle;
    cylinderMiddle.m_center = Vec3(0.f, 0.f, 0.35f);
    cylinderMiddle.m_radius = 0.25f;
    cylinderMiddle.m_height = 0.5f;
    cylinderMiddle.BuildVertices(verts, indices);

    ZCylinder cylinderTop;
    cylinderTop.m_center = Vec3(0.f, 0.f, 0.65f);
    cylinderTop.m_radius = 0.35f;
    cylinderTop.m_height = 0.16f;
    cylinderTop.BuildVertices(verts, indices);

    Sphere sphere;
    sphere.m_position = Vec3(0.f, 0.f, 0.85f);
    sphere.m_radius   = 0.30f;
    sphere.BuildVertices(verts, indices, 256);

    Sphere sphereTop;
    sphereTop.m_position = Vec3(0.f, 0.f, 1.1f);
    sphereTop.m_radius   = 0.08f;
    sphereTop.BuildVertices(verts, indices, 256);
}
