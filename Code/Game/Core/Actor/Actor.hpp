#pragma once
#include <memory>
#include <vector>

#include "Engine/Core/XmlUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"

struct Mat44;
class IComponent;

///  Actor ― The entity that owns a set of Components
class Actor
{
public:
    Actor();
    virtual ~Actor();

    /// Component management
    template <typename T, typename... Args>
    T* AddComponent(Args&&... args)
    {
        static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");

        // TODO: uniqueness check
        // if constexpr (T::IsUnique) { ... }

        auto comp = std::make_unique<T>(std::forward<Args>(args)...);
        T*   raw  = comp.get();

        // Inject back-pointer & lifecycle hook
        raw->OnAttach(*this);
        m_components.emplace_back(std::move(comp));

        return raw;
    }

    template <typename T>
    T* GetComponent() const
    {
        for (const auto& c : m_components)
            if (c->GetTypeID() == T::TypeID())
                return static_cast<T*>(c.get());
        return nullptr;
    }

    template <typename T>
    bool HasComponent() const
    {
        return GetComponent<T>() != nullptr;
    }


    template <typename T>
    bool RemoveComponent()
    {
        const auto id = T::TypeID();
        auto       it = std::find_if(m_components.begin(), m_components.end(),
                               [id](const auto& p) { return p->GetTypeID() == id; });
        if (it == m_components.end()) return false;

        (*it)->OnDetach();
        m_components.erase(it);
        return true;
    }

    /// Life Hook Functions

    // Called immediately after World::SpawnActor()
    virtual void Initialize();

    // Called every frame by World
    void         Tick(float deltaTime);
    virtual void OnTick(float deltaTime); // Allow User to override
    bool         SetEnabled(bool newEnable = true);
    bool         GetIsGarbage() const { return m_isGarbage; } // Should use action handler
    void         Destroy();

    /// Hierarchy
    void                                       SetParent(Actor* newParent);
    Actor*                                     GetParent() const { return m_parent; }
    const std::vector<std::unique_ptr<Actor>>& GetChildren() const { return m_children; }
    Actor*                                     CreateChild();

    /// Identity
    using ActorID = unsigned int;
    ActorID GetID() const { return m_id; }

    /// Serialization
    virtual XmlElement* ToXML() const;
    virtual Actor*      FromXML(const XmlElement& element);

    /// Transform
    Mat44 GetModelToWorldTransform();

    Vec3        m_position;
    EulerAngles m_orientation;

private:
    using ComponentPtr = std::unique_ptr<IComponent>;
    std::vector<ComponentPtr> m_components;

    // Transform can also be split into separate components; placeholders are reserved here
    // TransformComponent* m_transform = nullptr;

    std::vector<std::unique_ptr<Actor>> m_children;
    Actor*                              m_parent = nullptr;

    ActorID        m_id;
    static ActorID s_nextID;
    bool           m_isInitialized = false;
    bool           m_isGarbage     = false;
    bool           m_isEnable      = true;
};
