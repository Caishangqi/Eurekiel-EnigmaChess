#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/Vec3.hpp"

///
/// IComponent ― Every concrete component derives from this pure-virtual base
class Actor;

class IComponent
{
    friend class Actor;

public:
    /// Life-Cycle Hook
    virtual      ~IComponent() = default;
    virtual void OnInit();
    virtual void OnAttach(Actor& owner);
    virtual void OnDetach();
    virtual void OnDestroy();
    virtual void Tick(float deltaTime);
    virtual void OnTick(float deltaTime);

    void SetEnable(bool enable);
    bool GetEnable() const;

    /// Debug
    bool SetEnableDebugDraw(bool enable);
    void SetDebugColor(Rgba8 color) { m_debugColor = color; }

    void SetPosition(const Vec3& position);

    using ComponentTypeID = int;

    virtual ComponentTypeID GetTypeID() const = 0;

    /// @brief Helper: static per-class id generator
    template <typename T>
    static ComponentTypeID StaticTypeID()
    {
        // C++17 guarantees that function-local static variables are initialised
        // exactly once in a thread-safe manner.
        static ComponentTypeID s_typeID = s_nextTypeID++;
        return s_typeID;
    }

    [[nodiscard]]
    Actor* GetOwner() const;
    Vec3   GetWorldPosition() const;

    /// Serialization
    virtual IComponent* FromXML(const XmlElement& xmlElement) = 0;
    virtual XmlElement* ToXML() const = 0;

protected:
    IComponent() = default; // Protected default constructor, only derived class constructors are allowed

    /// Debug
    bool  m_enableDebugDraw = false; // Component debug draw, negated by component not implement IRenderable interface.
    Rgba8 m_debugColor      = Rgba8::WHITE;

    Vec3 m_position = Vec3::ZERO; // Component Local space
private:
    inline static ComponentTypeID s_nextTypeID = 0; // Globally incremented type counter
    Actor*                        m_owner      = nullptr; // Non-owning pointer to owning Actor
    bool                          m_enabled    = true; // Component ticking switch
};

#define COMPONENT_CLASS(body)                                            \
public:                                                                  \
using Super = body;                                                  \
static IComponent::ComponentTypeID TypeID()                          \
{                                                                    \
return IComponent::StaticTypeID<body>();                         \
}                                                                    \
IComponent::ComponentTypeID GetTypeID() const override               \
{                                                                    \
return body::TypeID();                                           \
}
