#include "ChessMatch.hpp"

#include "ChessBoard.hpp"
#include "ChessPiece.hpp"
#include "ChessPlayer.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Actor/Actor.hpp"
#include "Game/Core/Component/CollisionComponent.hpp"
#include "Game/Core/Component/MeshComponent.hpp"
#include "Game/Module/Definition/ChessPieceDefinition.hpp"
#include "Game/Module/Test/TestModelActor.hpp"

ChessMatch::ChessMatch(Game* game) : m_game(game)
{
    m_chessBoard = new ChessBoard();
    m_chessBoard->FromXML(*g_theGame->m_chessMatchConfig.RootElement());
    m_chessBoard->SetOuter(this);
    SpawnActor(Vec3(0.f, 0.f, 0.f), EulerAngles(0, 0, 0), m_chessBoard);

    /// Populate Chess board data layout
    for (auto& m_chess_grid : m_chessGrid)
    {
        m_chess_grid.resize(8, nullptr);
    }
    ChessMatch::FromXML(*g_theGame->m_chessMatchConfig.RootElement());

    /// Create Player
    for (Faction faction : m_factions)
    {
        auto player       = new ChessPlayer(this);
        player->m_faction = faction;
        SpawnActor(faction.m_viewPosition, faction.m_viewOrientation, player);
        m_players.emplace_back(player);
        LOG(LogGame, Info, "Create Player with faction = [ %d ] display name = [ %s ]", faction.m_id, faction.m_displayName.c_str());
    }
    g_theGame->EnterCameraState(ECameraState::PER_PLAYER);
    ChessMatchCommon::GetCameraTransform(g_theGame->cameraState, g_theGame->m_player->m_position, g_theGame->m_player->m_orientation, this);
    g_theDevConsole->AddLine(Rgba8::WHITE, Stringf("Current Player = [ %s ]", GetCurrentTurnPlayer()->m_faction.m_displayName.c_str()));
    ChessMatchCommon::PrintChessGrid(m_chessGrid);

    /// Initialize Lights
    m_pointLight
        .SetPosition(Vec3(7.0f, 3.0f, 1.0f))
        .SetDirection(Vec3(0.0f, 0.0f, 0.0f))
        .SetColor(Rgba8(255, 255, 255, 200))
        .SetInnerRadius(1.0f)
        .SetOuterRadius(4.0f);

    m_spotLight
        .SetPosition(Vec3(0.0f, 0.0f, 1.0f))
        .SetDirection(Vec3(1.0f, 1.0f, -0.5f))
        .SetColor(Rgba8(0, 255, 255, 200))
        .SetInnerRadius(1.0f)
        .SetOuterRadius(8.0f)
        .SetInnerAngle(10.f)
        .SetOuterAngle(20.f);

    /// Debug Model Loader
    /// TODO: Remove at release
    TestModelActor* testModelActor = new TestModelActor();
    SpawnActor(Vec3(4, 4, 4), EulerAngles(), testModelActor);
}

ChessMatch::~ChessMatch()
{
    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        m_actors[i]->Destroy();
        POINTER_SAFE_DELETE(m_actors[i])
    }
}

void ChessMatch::FromXML(const XmlElement& xmlElement)
{
    UNUSED(xmlElement)
    if (!g_theGame->m_chessMatchConfig.RootElement())
        return;
    XmlElement*       root                 = g_theGame->m_chessMatchConfig.RootElement();
    const XmlElement* chessBoardElement    = FindChildElementByName(*root, "ChessBoard");
    const XmlElement* chessFractionElement = FindChildElementByName(*chessBoardElement, "Factions");
    const XmlElement* chessPiecesElement   = FindChildElementByName(*chessBoardElement, "ChessPieces");
    const XmlElement* element              = chessPiecesElement->FirstChildElement();
    const XmlElement* elementFraction      = chessFractionElement->FirstChildElement();
    while (elementFraction != nullptr)
    {
        Faction faction           = {};
        faction.m_displayName     = ParseXmlAttribute(*elementFraction, "display", faction.m_displayName);
        faction.m_id              = ParseXmlAttribute(*elementFraction, "id", faction.m_id);
        faction.m_color           = ParseXmlAttribute(*elementFraction, "color", faction.m_color);
        faction.m_viewPosition    = ParseXmlAttribute(*elementFraction, "viewPosition", faction.m_viewPosition);
        faction.m_viewOrientation = EulerAngles(ParseXmlAttribute(*elementFraction, "viewOrientation", Vec3::ZERO));
        m_factions.push_back(faction);
        elementFraction = elementFraction->NextSiblingElement();
    }
    Texture* chessPiecesTexture = g_theRenderer->CreateOrGetTexture(ParseXmlAttribute(*chessPiecesElement, "texture", std::string()).c_str());
    while (element != nullptr)
    {
        std::string position                     = ParseXmlAttribute(*element, "position", position);
        auto        piece                        = new ChessPiece();
        piece->m_meshComponent->m_diffuseTexture = chessPiecesTexture;
        piece->SetOuter(this);
        piece->FromXML(*element); // Read XMl and find definition
        AddChessPieceToMatch(piece, position);
        element = element->NextSiblingElement();
    }
}

XmlElement* ChessMatch::ToXML() const
{
    return nullptr;
}

void ChessMatch::Update()
{
    for (int i = 0; i < static_cast<int>(m_actors.size()); i++)
    {
        if (m_actors[i])
        {
            m_actors[i]->Tick(g_theGame->m_clock->GetDeltaSeconds());
        }
    }
    if (g_theInput->WasKeyJustPressed(115))
    {
        if (g_theGame->cameraMode == ECameraMode::FREE)
        {
            g_theGame->cameraMode = ECameraMode::AUTO;
            ChessMatchCommon::GetCameraTransform(g_theGame->cameraState, g_theGame->m_player->m_position, g_theGame->m_player->m_orientation, this);
        }
        else
        {
            g_theGame->cameraMode = ECameraMode::FREE;
        }
    }

    /// Update the light constants
    g_theGame->m_lightingConstants.NumLights = 2;
    g_theGame->m_lightingConstants.lights[0] = m_spotLight;
    g_theGame->m_lightingConstants.lights[1] = m_pointLight;
}

Actor* ChessMatch::SpawnActor(Vec3 position, EulerAngles orientation, Actor* actor)
{
    actor->m_position    = position;
    actor->m_orientation = orientation;
    m_actors.emplace_back(actor);
    actor->Initialize();
    return actor;
}

ChessPiece* ChessMatch::AddChessPieceToMatch(ChessPiece* chessPiece, std::string gridPosition)
{
    IntVec2     girdPos = ChessMatchCommon::GetGridPosition(gridPosition);
    EulerAngles orientation;
    if (chessPiece->m_faction == 1)
        orientation = EulerAngles(180, 0, 0);
    SpawnActor(Vec3(static_cast<float>(girdPos.x) + 0.5f, static_cast<float>(girdPos.y) + 0.5f, 0), orientation, chessPiece);
    m_chessGrid[girdPos.x][girdPos.y] = chessPiece;
    chessPiece->m_gridCurrentPosition = girdPos;
    chessPiece->_match                = this;
    LOG(LogGame, Info, Stringf("Add Chess piece [ %s ]      to [ %s ] / grid = [ %d, %d ] world = [ %.2f, %.2f ]", chessPiece->m_definition->m_name.c_str(), gridPosition.c_str(), girdPos.x, girdPos.y,
            chessPiece->m_position.x, chessPiece->m_position.y).c_str());
    return chessPiece;
}

ChessPiece* ChessMatch::ExecuteChessMove(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo, Strings meta)
{
    using namespace ChessMatchCommon;
    ChessPiece* mover = dynamic_cast<ChessPiece*>(m_chessGrid[fromPos.x][fromPos.y]);
    if (!mover)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                 Stringf("[ %s ] do not have any piece", strFrom.c_str()));
        return nullptr;
    }
    if (mover->m_faction != GetCurrentTurnPlayer()->m_faction.m_id)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Not your turn for that piece");
        return nullptr;
    }

    MoveResult res = mover->ChessMove(fromPos, toPos, strFrom, strTo);
    if (!GetChessMoveValid(res))
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(res.m_moveResult));
        return nullptr;
    }
    // Clear the double step markers from the previous round
    ClearPawnDoubleMoveFlags();

    // Process capture
    if (res.m_piecesCapture)
    {
        // Passing soldier capture: delete the soldier next to it
        if (res.m_moveResult == ChessMoveResult::VALID_CAPTURE_ENPASSANT)
        {
            IntVec2 capPos                  = res.m_piecesCapture->m_gridCurrentPosition;
            m_chessGrid[capPos.x][capPos.y] = nullptr;
        }
        res.m_piecesCapture->Destroy();
    }

    // King and Rook Castling Synchronous Rook Movement
    if (res.m_moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE ||
        res.m_moveResult == ChessMoveResult::VALID_CASTLE_QUEENSIDE)
    {
        bool    kingSide = (res.m_moveResult == ChessMoveResult::VALID_CASTLE_KINGSIDE);
        int     dir      = kingSide ? +1 : -1;
        IntVec2 rookFrom = kingSide ? IntVec2(7, fromPos.y) : IntVec2(0, fromPos.y);
        IntVec2 rookTo   = fromPos + IntVec2(dir, 0);

        ChessPiece* rook                    = dynamic_cast<ChessPiece*>(m_chessGrid[rookFrom.x][rookFrom.y]);
        m_chessGrid[rookFrom.x][rookFrom.y] = nullptr;
        m_chessGrid[rookTo.x][rookTo.y]     = rook;
        rook->m_gridCurrentPosition         = rookTo;
        //rook->m_position                    = Vec3((float)rookTo.x + 0.5f, (float)rookTo.y + 0.5f, 0.f);
        rook->ChessMoveInterpolate(rookFrom, rookTo);
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(res.m_moveResult));
        rook->m_hasMoved = true;
    }

    // Move the main chess piece
    m_chessGrid[fromPos.x][fromPos.y] = nullptr;
    m_chessGrid[toPos.x][toPos.y]     = mover;
    mover->ChessMoveInterpolate(fromPos, toPos);
    mover->m_gridPreviousPosition = fromPos;
    mover->m_gridCurrentPosition  = toPos;
    //mover->m_position             = Vec3((float)toPos.x + 0.5f, (float)toPos.y + 0.5f, 0.f);

    // Pawn promotion
    if (res.m_moveResult == ChessMoveResult::VALID_MOVE_PROMOTION || res.m_moveResult == ChessMoveResult::VALID_CAPTURE_PROMOTION)
    {
        std::pair<std::string, std::string> pair;
        std::string                         outMessage;
        int                                 valid = GetCommandStringsWith(meta, "promoteTo", pair, outMessage);
        if (valid != -1)
        {
            mover->SetChessPromotion(mover->m_definition, ChessPieceDefinition::GetByName(pair.second));
        }
    }
    else
    {
        std::pair<std::string, std::string> pair;
        std::string                         outMessage;
        int                                 valid = GetCommandStringsWith(meta, "promoteTo", pair, outMessage);
        if (valid != -1)
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Your move does not eligible for promotion, but your args have promotion.");
        }
    }

    // Mark double steps
    if (mover->m_definition->m_name == "Pawn")
    {
        if (std::abs(toPos.y - fromPos.y) == 2)
            mover->m_movedTwoSquaresLastTurn = true;
    }

    g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, to_string(res.m_moveResult));


    if (res.m_piecesCapture && res.m_piecesCapture->m_definition->m_name == "King")
    {
        g_theGame->EnterState(EGameState::SETTLEMENT);
        g_theGame->EnterCameraState(ECameraState::CONFIGURED);
        return mover;
    }

    // End of turn
    StepNextTurn();
    return mover;
}

ChessMatchCommon::MoveResult ChessMatch::ExecuteChessTeleport(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo, Strings meta)
{
    ChessMatchCommon::MoveResult result;
    using namespace ChessMatchCommon;
    ChessPiece* mover = dynamic_cast<ChessPiece*>(m_chessGrid[fromPos.x][fromPos.y]);
    if (!mover)
    {
        result.m_moveResult = ChessMoveResult::INVALID_MOVE_NO_PIECE;
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(result.m_moveResult));
        return result;
    }
    if (mover->m_faction != GetCurrentTurnPlayer()->m_faction.m_id)
    {
        result.m_moveResult = ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(result.m_moveResult));
        return result;
    }
    ChessPiece* victim = dynamic_cast<ChessPiece*>(m_chessGrid[toPos.x][toPos.y]);
    // Teleport capture
    if (victim)
    {
        if (victim->m_faction == GetCurrentTurnPlayer()->m_faction.m_id)
        {
            result.m_moveResult = ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(result.m_moveResult));
            return result;
        }
        else
        {
            mover->ChessMoveInterpolate(fromPos, toPos);
            mover->m_gridPreviousPosition     = fromPos;
            mover->m_gridCurrentPosition      = toPos;
            m_chessGrid[toPos.x][toPos.y]     = mover;
            m_chessGrid[fromPos.x][fromPos.y] = nullptr;
            result.m_piecesCapture            = victim;
            victim->Destroy();
            result.m_moveResult = ChessMoveResult::VALID_CAPTURE_TELEPORT;
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(result.m_moveResult));
        }
    }
    else
    {
        // Teleport move
        mover->ChessMoveInterpolate(fromPos, toPos);
        mover->m_gridPreviousPosition     = fromPos;
        mover->m_gridCurrentPosition      = toPos;
        m_chessGrid[toPos.x][toPos.y]     = mover;
        m_chessGrid[fromPos.x][fromPos.y] = nullptr;
        result.m_moveResult               = ChessMoveResult::VALID_MOVE_TELEPORT;
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, to_string(result.m_moveResult));
    }
    // Capture king
    if (result.m_piecesCapture && result.m_piecesCapture->m_definition->m_name == "King")
    {
        g_theGame->EnterState(EGameState::SETTLEMENT);
        g_theGame->EnterCameraState(ECameraState::CONFIGURED);
        return result;
    }
    StepNextTurn();
    return result;
}


const ChessPlayer* ChessMatch::StepNextTurn()
{
    m_turnCounter++;
    m_currentPlayerIndex = m_currentPlayerIndex + 1;
    m_currentPlayerIndex %= static_cast<int>(m_players.size());
    g_theDevConsole->AddLine(Rgba8::WHITE, Stringf("Current Player = [ %s ]", GetCurrentTurnPlayer()->m_faction.m_displayName.c_str()));
    ChessMatchCommon::PrintChessGrid(m_chessGrid);
    ChessMatchCommon::GetCameraTransform(g_theGame->cameraState, g_theGame->m_player->m_position, g_theGame->m_player->m_orientation, this);
    return GetCurrentTurnPlayer();
}

ChessMatchCommon::RaycastResultChess ChessMatch::Raycast(const Vec3& origin, const Vec3& direction, float maxDistance) const
{
    using namespace ChessMatchCommon;
    RaycastResultChess result;

    std::pair<RaycastResult3D, ChessObject*> resultPair;
    float                                    closestDistance = FLT_MAX;
    for (Actor* actor : m_actors)
    {
        if (!actor) continue;
        ChessObject* chessObject = dynamic_cast<ChessObject*>(actor);
        if (!chessObject)continue;
        if (chessObject->GetIsGarbage()) continue;
        CollisionComponent* collisionComponent = chessObject->GetComponent<CollisionComponent>();
        if (!collisionComponent) continue;

        RaycastResult3D geometryRaycastResult = collisionComponent->Raycast(origin, direction, maxDistance);
        if (geometryRaycastResult.m_didImpact)
        {
            if (geometryRaycastResult.m_impactDist < closestDistance)
            {
                closestDistance = geometryRaycastResult.m_impactDist;
                resultPair      = std::make_pair(geometryRaycastResult, chessObject);
            }
        }
    }


    /// Consider use union to alignment the memory?
    result.m_didImpact    = resultPair.first.m_didImpact;
    result.m_impactPos    = resultPair.first.m_impactPos;
    result.m_impactNormal = resultPair.first.m_impactNormal;
    result.m_hitObject    = resultPair.second;
    result.m_rayFwdNormal = resultPair.first.m_rayFwdNormal;
    result.m_rayMaxLength = resultPair.first.m_rayMaxLength;
    result.m_rayStartPos  = resultPair.first.m_rayStartPos;

    return result;
}


void ChessMatch::ClearPawnDoubleMoveFlags()
{
    std::vector<ChessPiece*> outPieces;
    ChessMatchCommon::GetAllPieces(m_chessGrid, outPieces);
    for (auto piece : outPieces)
    {
        if (piece && piece->m_definition->m_name == "Pawn")
            piece->m_movedTwoSquaresLastTurn = false;
    }
}
