#include "Actor.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Game/Core/Component/Component.hpp"
Actor::ActorID Actor::s_nextID = 0;

Actor::Actor(): m_id(s_nextID++)
{
}

Actor::~Actor() = default;

void Actor::Initialize()
{
    for (auto& comp : m_components)
        comp->OnInit();
    m_isInitialized = true;
}

void Actor::Tick(float dt)
{
    if (!m_isEnable) return;
    OnTick(dt);
    // Call Tick on every enabled component
    for (auto& comp : m_components)
        if (comp->GetEnable())
        {
            comp->Tick(dt);
        }
            
    // Tick children (depth-first)
    for (auto& child : m_children)
        child->Tick(dt);
}

void Actor::OnTick(float deltaTime)
{
    UNUSED(deltaTime)
}

bool Actor::SetEnabled(bool newEnable)
{
    m_isEnable = newEnable;
    return newEnable;
}

void Actor::Destroy()
{
    m_isGarbage = true;
    for (auto& comp : m_components)
    {
        comp->OnDestroy();
    }
}

void Actor::SetParent(Actor* newParent)
{
    m_parent = newParent;
    // Logic such as updating the level Transform can be handled here
}

Actor* Actor::CreateChild()
{
    auto child = std::make_unique<Actor>();
    child->SetParent(this);
    Actor* raw = child.get();
    m_children.emplace_back(std::move(child));
    return raw;
}

XmlElement* Actor::ToXML() const
{
    return nullptr;
}

Actor* Actor::FromXML(const XmlElement& element)
{
    UNUSED(element)
    return this;
}

Mat44 Actor::GetModelToWorldTransform()
{
    Mat44 matTranslation = Mat44::MakeTranslation3D(m_position);
    matTranslation.Append(m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
    return matTranslation;
}
