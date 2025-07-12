#pragma once
#include "Game/Core/Actor/Actor.hpp"

class CollisionComponent;
class MeshComponent;



class TestModelActor : public Actor
{
public:
    ~TestModelActor() override;
    void        Initialize() override;
    void        OnTick(float deltaTime) override;
    XmlElement* ToXML() const override;
    Actor*      FromXML(const XmlElement& element) override;

private:
    MeshComponent*      m_meshComponent      = nullptr;
    CollisionComponent* m_collisionComponent = nullptr;
};
