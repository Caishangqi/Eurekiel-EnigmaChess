#pragma once
#include "Game/Core/Actor/Actor.hpp"

class CollisionComponent;
class ChessMatch;
class MeshComponent;

class ChessObject : public Actor
{
public:
    ChessObject();

    ChessObject* SetOuter(ChessMatch* outer)
    {
        _outer = outer;
        return this;
    }

    ChessMatch* GetOuter() { return _outer; }

    ~ChessObject() override;

protected:
    /// Component
    MeshComponent*      m_meshComponent      = nullptr;
    CollisionComponent* m_collisionComponent = nullptr;

    ChessMatch* _outer = nullptr;
};
