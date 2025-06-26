#pragma once
#include "ChessObject.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Game/Module/Lib/ChessMatchCommon.hpp"

class CollisionComponent;
class Timer;
class ChessPieceDefinition;
class ChessMatch;
using AnimationCallback = void (ChessPiece::*)();

class ChessPiece : public ChessObject
{
    friend class ChessMatch;
    friend class ChessPlayer;

public:
    ChessPiece();
    ~ChessPiece() override;

    Actor* FromXML(const XmlElement& element) override;

    /// Rule determination, only "evaluation" but not actual movement
    ChessMatchCommon::MoveResult ChessMove(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo);
    ChessMatchCommon::MoveResult ChessMoveTeleport(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo);
    ChessPiece*                  ChessMoveInterpolate(IntVec2 fromPos, IntVec2 toPos);
    ChessPiece*                  SetChessPromotion(ChessPieceDefinition* fromDef, ChessPieceDefinition* toDef);
    ChessPiece*                  UpdateGlyph();

    /// Highlight
    bool SetEnableHighlight(bool newEnable);

    void                        OnTick(float deltaTime) override;
    std::string                 GetGlyph() { return glyph; }
    const ChessPieceDefinition* GetDefinition() const { return m_definition; }

    bool m_hasMoved                = false; /// Whether any chess piece has been moved
    int  m_lastMoveTurn            = -1; /// The number of turns the last move occurred
    bool m_movedTwoSquaresLastTurn = false; /// Whether the soldier moved two squares in the previous round

protected:
    ChessMatch*           _match                 = nullptr;
    std::string           glyph                  = "?";
    IntVec2               m_gridCurrentPosition  = IntVec2::INVALID;
    IntVec2               m_gridPreviousPosition = IntVec2::INVALID;
    ChessPieceDefinition* m_definition           = nullptr;
    int                   m_faction              = -1; ///< 0 = white, 1 = black

    /// Animation
    float             m_timeSinceMove        = 0.f;
    float             m_animationTime        = 1.0f;
    Timer*            m_animationTimer       = nullptr;
    float             m_animationJumpHeight  = 1.0f;
    Vec3              m_animationPosition[2] = {};
    AnimationCallback m_animationCallback    = nullptr;

    /// Highlight
    bool           m_bIsHighlighted  = false;
    MeshComponent* m_squareHighlight = nullptr;

private:
    void UpdateAnimation();
    void ChessMoveSlide();
    void ChessMoveJump();
};
