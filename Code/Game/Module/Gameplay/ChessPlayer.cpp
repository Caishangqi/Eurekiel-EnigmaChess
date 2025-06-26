#include "ChessPlayer.hpp"

#include "ChessBoard.hpp"
#include "ChessPiece.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"

ChessPlayer::ChessPlayer(ChessMatch* match) : m_match(match)
{
    LOG(LogActor, Info, ((Stringf("Create ChessPlayer Actor with faction id = %d",m_faction.m_id)).c_str()));
    m_spectatorCamera = g_theGame->m_spectatorCamera;
}

ChessPlayer::~ChessPlayer()
{
}

void ChessPlayer::OnTick(float deltaTime)
{
    Actor::OnTick(deltaTime);

    bool ctrlHold = g_theInput->IsKeyDown(KEYCODE_LEFT_CTRL) || g_theInput->IsKeyDown(KEYCODE_RIGHT_CTRL);
    if (ctrlHold)
        m_bEnableTeleportCheat = true;
    else
    {
        m_bEnableTeleportCheat = false;
    }


    if (m_match->m_currentPlayerIndex != m_faction.m_id) return; // not our turn, do not tick

    /// Reset
    m_match->m_highLightedSquare = IntVec2::INVALID;
    m_match->m_impactSquare      = IntVec2::INVALID;
    m_hitPiece                   = nullptr;

    Vec3 forward, left, up;
    m_spectatorCamera->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
    Vec3 start = m_spectatorCamera->m_position;
    //Vec3 end   = m_spectatorCamera->m_position + forward * 10.f;

    /// Raycast
    ChessMatchCommon::RaycastResultChess result = m_match->Raycast(start, forward, 10.f);
    if (result.m_didImpact)
    {
        ChessBoard* board = dynamic_cast<ChessBoard*>(result.m_hitObject);
        ChessPiece* piece = dynamic_cast<ChessPiece*>(result.m_hitObject);
        if (board)
        {
            bool validDirection = result.m_impactNormal.z > 0.f;
            if (validDirection)
            {
                m_match->m_impactSquare   = IntVec2((int)result.m_impactPos.x, (int)result.m_impactPos.y);
                ChessPiece* pieceOnSquare = dynamic_cast<ChessPiece*>(m_match->m_chessGrid[m_match->m_impactSquare.x][m_match->m_impactSquare.y]);
                if (pieceOnSquare && pieceOnSquare->m_faction == m_faction.m_id)
                    m_match->m_highLightedSquare = m_match->m_impactSquare;
            }
        }
        else if (piece)
        {
            m_hitPiece              = piece;
            m_match->m_impactSquare = piece->m_gridCurrentPosition;
            if (piece->m_faction == m_faction.m_id)
            {
                m_match->m_highLightedSquare = piece->m_gridCurrentPosition;
            }
        }
    }

    HandlePlayerClickSelect();
    HandlePlayerClickMove();

    if (m_match->m_highLightedSquare != IntVec2::INVALID)
    {
        EventArgs args;
        args.SetValue("position", m_match->m_highLightedSquare.toString());
        g_theEventSystem->FireEvent("event.highlight.enable", args);
    }
    else
    {
        g_theEventSystem->FireEvent("event.highlight.disable");
    }
}

void ChessPlayer::HandlePlayerClickSelect()
{
    bool leftClick  = g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE);
    bool rightClick = g_theInput->WasMouseButtonJustPressed(KEYCODE_RIGHT_MOUSE);

    if (m_match->m_selectedPiece == nullptr)
    {
        if (m_match->m_highLightedSquare != IntVec2::INVALID)
        {
            if (leftClick)
            {
                m_match->m_selectedPiece = dynamic_cast<ChessPiece*>(m_match->m_chessGrid[m_match->m_highLightedSquare.x][m_match->m_highLightedSquare.y]);
                if (m_match->m_selectedPiece->m_faction == m_faction.m_id)
                {
                    m_match->m_selectedPiece->SetEnableHighlight(true);
                }
                else
                {
                    m_match->m_selectedPiece->SetEnableHighlight(false);
                }
            }
        }
    }
    else
    {
        if (rightClick)
        {
            m_match->m_selectedPiece->SetEnableHighlight(false);
            m_match->m_selectedPiece = nullptr;
        }
    }
}

void ChessPlayer::HandlePlayerClickMove()
{
    using namespace ChessMatchCommon;
    if (!m_match->m_selectedPiece) return;

    if (m_hitPiece && m_hitPiece->m_faction != m_faction.m_id)
        m_match->m_highLightedSquare = m_hitPiece->m_gridCurrentPosition;


    ChessPiece* mover              = m_match->m_selectedPiece;
    bool        leftClick          = g_theInput->WasMouseButtonJustPressed(KEYCODE_LEFT_MOUSE);
    bool        validOnBoardSquare = m_match->m_impactSquare != IntVec2::INVALID;
    IntVec2     impactPos          = m_match->m_impactSquare;
    bool        movePredicate      = false;

    if (!validOnBoardSquare) return;

    MoveResult res;
    if (m_bEnableTeleportCheat)
        res = mover->ChessMoveTeleport(mover->m_gridCurrentPosition, impactPos, "INVALID", "INVALID");
    else
        res = mover->ChessMove(mover->m_gridCurrentPosition, impactPos, "INVALID", "INVALID");


    if (GetChessMoveValid(res))
        movePredicate = true;
    else
    {
        m_match->m_highLightedSquare = IntVec2::INVALID;
        return;
    }

    if (movePredicate)
        m_match->m_highLightedSquare = impactPos;

    if (leftClick)
    {
        m_match->m_selectedPiece = nullptr;
        Strings meta;
        if (m_bEnableTeleportCheat)
            m_match->ExecuteChessTeleport(mover->m_gridCurrentPosition, impactPos, "INVALID", "INVALID", meta);
        else
            m_match->ExecuteChessMove(mover->m_gridCurrentPosition, impactPos, "INVALID", "INVALID", meta);
        mover->SetEnableHighlight(false);
        m_match->m_highLightedSquare = IntVec2::INVALID;
    }
}
