#pragma once
#include "ChessMatch.hpp"
#include "Game/Core/Actor/Actor.hpp"

class Camera;

class ChessPlayer : public Actor
{
public:
    ChessPlayer(ChessMatch* match);
    ~ChessPlayer() override;

    void OnTick(float deltaTime) override;

    Faction m_faction;

    bool GetEnableTeleportCheat() const { return m_bEnableTeleportCheat; }

protected:
    void HandlePlayerClickSelect(); // Handles the logic through select a pieces
    void HandlePlayerClickMove(); // Handles the pieces place

private:
    ChessMatch* m_match                = nullptr;
    Camera*     m_spectatorCamera      = nullptr;
    ChessPiece* m_hitPiece             = nullptr;
    bool        m_bEnableTeleportCheat = false;
};
