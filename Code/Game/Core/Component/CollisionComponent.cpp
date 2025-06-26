#include "CollisionComponent.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/Actor/Actor.hpp"
#include "Game/Core/Render/RenderContext.hpp"
#include "Game/Core/Render/RenderSubsystem.hpp"

CollisionComponent::CollisionComponent()
{
    m_debugVertexes.reserve(1024);
}

CollisionComponent::~CollisionComponent()
{
}

void CollisionComponent::OnInit()
{
    IComponent::OnInit();
}

void CollisionComponent::OnAttach(Actor& owner)
{
    IComponent::OnAttach(owner);
    g_theRenderSubsystem->Register(this);
}

void CollisionComponent::OnDetach()
{
    IComponent::OnDetach();
    g_theRenderSubsystem->Unregister(this);
}

void CollisionComponent::OnDestroy()
{
    IComponent::OnDestroy();
    g_theRenderSubsystem->Unregister(this);
}

void CollisionComponent::Tick(float deltaTime)
{
    IComponent::Tick(deltaTime);
}

IComponent* CollisionComponent::FromXML(const XmlElement& xmlElement)
{
    UNUSED(xmlElement)
    return this;
}

XmlElement* CollisionComponent::ToXML() const
{
    return NULL;
}

void CollisionComponent::Render(const RenderContext& ctx)
{
    if (!m_enableDebugDraw) return; // not enable debug draw
    if (m_collisionBox.GetDimensions().x == 0.f || m_collisionBox.GetDimensions().y == 0.f || m_collisionBox.GetDimensions().z == 0.f) return; // not valid collision box
    ctx.SetModel(Mat44(), Rgba8::WHITE); // We want the actual aabb3 transform, and out vertices build from world space not local.
    ctx.renderer.BindTexture(nullptr);
    ctx.renderer.BindShader(nullptr);
    m_debugVertexes.clear();
    AABB3 box = GetCollisionBox();
    AddVertsForCube3DWireFrame(m_debugVertexes, box, m_debugColor);
    ctx.renderer.DrawVertexArray(m_debugVertexes);
}

CollisionComponent* CollisionComponent::SetCollisionBox(AABB3& collisionBox)
{
    m_collisionBox = collisionBox;
    m_collisionBox.SetCenter(Vec3::ZERO);
    return this;
}

RaycastResult3D CollisionComponent::Raycast(const Vec3& origin, const Vec3& direction, float maxDistance)
{
    // We turn the box to world space rather turn raycast to local space
    AABB3 worldBox = GetCollisionBox();
    worldBox.SetCenter(GetWorldPosition());
    return worldBox.Raycast(origin, direction, maxDistance);
}

AABB3 CollisionComponent::GetCollisionBox(bool worldSpace)
{
    if (worldSpace)
    {
        AABB3 worldBox = m_collisionBox;
        worldBox.SetCenter(GetWorldPosition());
        return worldBox;
    }
    return m_collisionBox;
}
