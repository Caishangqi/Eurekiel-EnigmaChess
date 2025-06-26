#include "BakedModelKnight.hpp"

#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/OBB3.hpp"
#include "Engine/Math/ZCylinder.hpp"

BakedModelKnight::BakedModelKnight()
{
    name = "Knight";
}

BakedModelKnight::~BakedModelKnight()
{
}

void BakedModelKnight::Build()
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

    OBB3 obbTop0;
    obbTop0.m_center         = Vec3(0.f, 0.f, 0.7f);
    obbTop0.m_halfDimensions = Vec3(0.10f, 0.10f, 0.20f);
    auto eulerAngles0        = EulerAngles(0.f, 0.f, -30.f);
    Vec3 i, j, k;
    eulerAngles0.GetAsVectors_IFwd_JLeft_KUp(i, j, k);
    obbTop0.m_iBasisNormal = i;
    obbTop0.m_jBasisNormal = j;
    obbTop0.m_kBasisNormal = k;
    obbTop0.BuildVertices(verts, indices);


    OBB3 obbTop1;
    obbTop1.m_center         = Vec3(0.f, 0.18f, 0.76f);
    obbTop1.m_halfDimensions = Vec3(0.08f, 0.12f, 0.04f);
    auto eulerAngles1        = EulerAngles(0.f, 0.f, -30.f);
    Vec3 i1, j1, k1;
    eulerAngles1.GetAsVectors_IFwd_JLeft_KUp(i1, j1, k1);
    obbTop1.m_iBasisNormal = i1;
    obbTop1.m_jBasisNormal = j1;
    obbTop1.m_kBasisNormal = k1;
    obbTop1.BuildVertices(verts, indices);
}
