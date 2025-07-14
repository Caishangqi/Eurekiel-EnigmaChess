#pragma once
#include "Component.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Game/Core/Render/Renderable.hpp"

struct Vertex_PCU;

/// Simple collision box component
class CollisionComponent : public IComponent, public IRenderable
{
    COMPONENT_CLASS(CollisionComponent)

    // IComponent
    CollisionComponent();
    ~CollisionComponent() override;

    void        OnInit() override;
    void        OnAttach(Actor& owner) override;
    void        OnDetach() override;
    void        OnDestroy() override;
    void        Tick(float deltaTime) override;
    IComponent* FromXML(const XmlElement& xmlElement) override;
    XmlElement* ToXML() const override;

    // IRenderable

    void Render(const RenderContext& ctx) override;

    // Current
    CollisionComponent* SetCollisionBox(AABB3& collisionBox); // This method will automatically set the local space of aabb3
    RaycastResult3D     Raycast(const Vec3& origin, const Vec3& direction, float maxDistance);
    AABB3               GetCollisionBox(bool worldSpace = true);

private:
    AABB3                   m_collisionBox;
    std::vector<Vertex_PCU> m_debugVertexes; // Debug vertices for wireframe drawing
};
