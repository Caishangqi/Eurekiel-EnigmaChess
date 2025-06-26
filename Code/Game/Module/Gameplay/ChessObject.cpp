#include "ChessObject.hpp"
#include "Game/Core/Component/MeshComponent.hpp"
#include "Game/Core/Component/CollisionComponent.hpp"

ChessObject::ChessObject()
{
    m_meshComponent      = AddComponent<MeshComponent>();
    m_collisionComponent = AddComponent<CollisionComponent>();
}

ChessObject::~ChessObject()
{
}
