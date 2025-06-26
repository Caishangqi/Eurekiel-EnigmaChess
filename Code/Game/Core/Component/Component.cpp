#include "Component.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Game/Core/Actor/Actor.hpp"

void IComponent::OnInit()
{
}

void IComponent::OnAttach(Actor& owner)
{
    m_owner = &owner;
}

void IComponent::OnDetach()
{
}

void IComponent::OnDestroy()
{
}

void IComponent::Tick(float deltaTime)
{
    if (m_enabled)
    {
        OnTick(deltaTime);
    }
}

void IComponent::OnTick(float deltaTime)
{
    UNUSED(deltaTime)
}

void IComponent::SetEnable(bool enable)
{
    m_enabled = enable;
}

bool IComponent::GetEnable() const
{
    return m_enabled;
}

bool IComponent::SetEnableDebugDraw(bool enable)
{
    m_enableDebugDraw = enable;
    return enable;
}

void IComponent::SetPosition(const Vec3& position)
{
    m_position = position;
}

Actor* IComponent::GetOwner() const
{
    return m_owner;
}

Vec3 IComponent::GetWorldPosition() const
{
    if (!m_owner)
        return Vec3::INVALID;
    return m_owner->m_position + m_position;
}
