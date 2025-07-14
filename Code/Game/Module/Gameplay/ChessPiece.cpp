#include "ChessPiece.hpp"

#include <algorithm>

#include "Engine/Math/MathUtils.hpp"
#include "ChessMatch.hpp"
#include "ChessPlayer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/Easing.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Component/CollisionComponent.hpp"
#include "Game/Core/Component/MeshComponent.hpp"
#include "Game/Module/Definition/ChessPieceDefinition.hpp"
#include "Game/Module/Lib/ChessMatchCommon.hpp"

using namespace ChessMatchCommon;
static int Signi(int v) { return (v > 0) - (v < 0); }

ChessPiece::ChessPiece()
{
    // box that could used in collision and crude highlight draw
    AABB3 box;
    box.SetDimensions(Vec3(0.5f, 0.5f, 1.0f));

    // Highlight, write shader code in the future
    std::vector<Vertex_PCU> vertex;
    vertex.reserve(256);
    AddVertsForCube3DWireFrame(vertex, box, Rgba8::DEBUG_GREEN);
    m_squareHighlight = AddComponent<MeshComponent>();
    m_squareHighlight->AppendVertices(vertex);
    m_squareHighlight->SetPosition(Vec3(0.f, 0.f, 0.5f));
    m_squareHighlight->SetEnable(false);

    m_collisionComponent->SetCollisionBox(box);
    m_collisionComponent->SetPosition(Vec3(0.f, 0.f, 0.5f));
    m_collisionComponent->SetEnableDebugDraw(false);
    m_collisionComponent->SetDebugColor(Rgba8::DEBUG_GREEN);
    m_animationTimer = new Timer(m_animationTime, g_theGame->m_clock);
    m_animationTimer->Stop();
}

ChessPiece::~ChessPiece()
{
}

static bool IsPathClear(ChessGrid& grid, IntVec2 from, IntVec2 to)
{
    int     dx  = Signi(to.x - from.x);
    int     dy  = Signi(to.y - from.y);
    IntVec2 cur = from + IntVec2(dx, dy);
    while (cur != to)
    {
        if (grid[cur.x][cur.y] != nullptr) return false;
        cur += IntVec2(dx, dy);
    }
    return true;
}


Actor* ChessPiece::FromXML(const XmlElement& element)
{
    m_definition = ChessPieceDefinition::GetByName(ParseXmlAttribute(element, "name", std::string()));
    m_faction    = ParseXmlAttribute(element, "faction", m_faction);
    UpdateGlyph();
    m_meshComponent->m_color                = GetFaction(m_faction, _outer)->m_color;
    m_meshComponent->m_shader               = m_definition->m_shader;
    m_meshComponent->m_diffuseTexture       = m_definition->m_texture;
    m_meshComponent->m_normalTexture        = m_definition->m_normalTexture;
    m_meshComponent->m_specGlossEmitTexture = m_definition->m_specGlossEmitTexture;
    m_meshComponent->Build(m_definition->m_model);

    return this;
}

MoveResult ChessPiece::ChessMove(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo)
{
    MoveResult r;
    r.m_fromPosition       = fromPos;
    r.m_toPosition         = toPos;
    r.m_fromPositionString = strFrom;
    r.m_toPositionString   = strTo;
    r.m_piecesMove         = this;

    // Coordinate validity
    if (fromPos == toPos)
    {
        r.m_moveResult = ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE;
        return r;
    }
    if (fromPos.x < 0 || fromPos.x > 7 || fromPos.y < 0 || fromPos.y > 7 ||
        toPos.x < 0 || toPos.x > 7 || toPos.y < 0 || toPos.y > 7)
    {
        r.m_moveResult = ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
        return r;
    }

    ChessGrid& grid     = _match->m_chessGrid;
    auto       dstPiece = dynamic_cast<ChessPiece*>(grid[toPos.x][toPos.y]);
    if (dstPiece && dstPiece->m_faction == m_faction)
    {
        r.m_moveResult = ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED;
        return r;
    }
    r.m_piecesCapture = dstPiece;

    const int dx  = toPos.x - fromPos.x;
    const int dy  = toPos.y - fromPos.y;
    const int adx = std::abs(dx);
    const int ady = std::abs(dy);

    // Different chess pieces validation
    auto pathClear = [&]()-> bool { return IsPathClear(grid, fromPos, toPos); };

    /* Knight */
    if (m_definition->m_name == "Knight")
    {
        if (!((adx == 2 && ady == 1) || (adx == 1 && ady == 2)))
            r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
        else
            r.m_moveResult = dstPiece
                                 ? ChessMoveResult::VALID_CAPTURE_NORMAL
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
    }

    /* Bishop */
    else if (m_definition->m_name == "Bishop")
    {
        if (adx != ady) r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
        else if (!pathClear()) r.m_moveResult = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
        else
            r.m_moveResult = dstPiece
                                 ? ChessMoveResult::VALID_CAPTURE_NORMAL
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
    }

    /* Rook */
    else if (m_definition->m_name == "Rook")
    {
        if (adx != 0 && ady != 0) r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
        else if (!pathClear()) r.m_moveResult = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
        else
            r.m_moveResult = dstPiece
                                 ? ChessMoveResult::VALID_CAPTURE_NORMAL
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
    }

    /* Queen */
    else if (m_definition->m_name == "Queen")
    {
        bool diagonal = (adx == ady);
        bool straight = (adx == 0 || ady == 0);
        if (!(diagonal || straight)) r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
        else if (!pathClear()) r.m_moveResult = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
        else
            r.m_moveResult = dstPiece
                                 ? ChessMoveResult::VALID_CAPTURE_NORMAL
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
    }

    /* Pawn */
    else if (m_definition->m_name == "Pawn")
    {
        int dir      = (m_faction == 0) ? +1 : -1; // 白向 +y
        int startRow = (m_faction == 0) ? 1 : 6;

        // Single step forward
        if (dx == 0 && dy == dir && !dstPiece)
        {
            r.m_moveResult = (toPos.y == 0 || toPos.y == 7)
                                 ? ChessMoveResult::VALID_MOVE_PROMOTION
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
        }
        // First round double step
        else if (dx == 0 && dy == 2 * dir && !dstPiece && fromPos.y == startRow)
        {
            IntVec2 mid = fromPos + IntVec2(0, dir);
            if (grid[mid.x][mid.y] != nullptr)
                r.m_moveResult = ChessMoveResult::INVALID_MOVE_PATH_BLOCKED;
            else
                r.m_moveResult = ChessMoveResult::VALID_MOVE_NORMAL;
        }
        // Diagonal capture / Passing pawn
        else if (adx == 1 && dy == dir)
        {
            // Normal capture
            if (dstPiece && dstPiece->m_faction != m_faction)
            {
                bool reachBackRank = (toPos.y == 0 || toPos.y == 7);
                r.m_moveResult     = reachBackRank
                                     ? ChessMoveResult::VALID_CAPTURE_PROMOTION // 斜吃 + 升变
                                     : ChessMoveResult::VALID_CAPTURE_NORMAL; // 普通斜吃
            }
            /* en passant */
            else
            {
                IntVec2 sidePos  = fromPos + IntVec2(dx, 0); // 旁边那格
                auto    sidePawn = dynamic_cast<ChessPiece*>(grid[sidePos.x][sidePos.y]);
                if (sidePawn && sidePawn->m_definition->m_name == "Pawn" &&
                    sidePawn->m_faction != m_faction &&
                    sidePawn->m_movedTwoSquaresLastTurn &&
                    sidePawn->m_lastMoveTurn == _match->m_turnCounter - 1)
                {
                    r.m_piecesCapture = sidePawn;
                    r.m_moveResult    = ChessMoveResult::VALID_CAPTURE_ENPASSANT;
                }
                else
                    r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            }
        }
        else r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;

        // Updated the soldier's first move mark and double step mark
        if (GetChessMoveValid(r))
        {
            m_hasMoved                = true;
            m_movedTwoSquaresLastTurn = (dx == 0 && std::abs(dy) == 2);
        }
    }

    /* King */
    else if (m_definition->m_name == "King")
    {
        bool single = max(adx, ady) == 1;
        if (single)
        {
            r.m_moveResult = dstPiece
                                 ? ChessMoveResult::VALID_CAPTURE_NORMAL
                                 : ChessMoveResult::VALID_MOVE_NORMAL;
        }
        else
        {
            // Castling
            if (m_hasMoved) { r.m_moveResult = ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED; }
            else if (ady != 0 || !(adx == 2 || adx == 3))
            {
                r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;
            }
            else
            {
                bool    kingSide = dx > 0;
                IntVec2 rookPos  = kingSide ? IntVec2(7, fromPos.y) : IntVec2(0, fromPos.y);
                auto    rook     = dynamic_cast<ChessPiece*>(grid[rookPos.x][rookPos.y]);
                if (!rook || rook->m_definition->m_name != "Rook")
                    r.m_moveResult = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
                else if (rook->m_hasMoved)
                    r.m_moveResult = ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED;
                else if (!IsPathClear(grid, fromPos, rookPos))
                    r.m_moveResult = ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED;
                else
                    r.m_moveResult = kingSide
                                         ? ChessMoveResult::VALID_CASTLE_KINGSIDE
                                         : ChessMoveResult::VALID_CASTLE_QUEENSIDE;
            }
        }
    }

    // record move information
    if (GetChessMoveValid(r))
    {
        m_lastMoveTurn = _match->m_turnCounter;
        m_hasMoved     = true;
    }

    // If still not set, keep UNKNOWN (considered invalid)
    if (r.m_moveResult == ChessMoveResult::UNKNOWN)
        r.m_moveResult = ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE;

    return r;
}

MoveResult ChessPiece::ChessMoveTeleport(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo)
{
    MoveResult result;
    ChessGrid& grid = g_theGame->match->m_chessGrid;

    auto mover = dynamic_cast<ChessPiece*>(grid[fromPos.x][fromPos.y]);
    if (!mover)
    {
        result.m_moveResult = ChessMoveResult::INVALID_MOVE_NO_PIECE;
        return result;
    }
    if (mover->m_faction != g_theGame->match->GetCurrentTurnPlayer()->m_faction.m_id)
    {
        result.m_moveResult = ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE;
        return result;
    }
    auto victim = dynamic_cast<ChessPiece*>(grid[toPos.x][toPos.y]);
    // Teleport capture
    if (victim)
    {
        if (victim->m_faction == g_theGame->match->GetCurrentTurnPlayer()->m_faction.m_id)
        {
            result.m_moveResult = ChessMoveResult::INVALID_MOVE_BAD_LOCATION;
            return result;
        }
        result.m_moveResult = ChessMoveResult::VALID_CAPTURE_TELEPORT;
    }
    else
    {
        result.m_moveResult = ChessMoveResult::VALID_MOVE_TELEPORT;
    }
    return result;
}

ChessPiece* ChessPiece::ChessMoveInterpolate(IntVec2 fromPos, IntVec2 toPos)
{
    m_animationTimer->Start();
    m_animationPosition[0] = Vec3(static_cast<float>(fromPos.x) + 0.5f, static_cast<float>(fromPos.y) + 0.5f, 0.0f);
    m_animationPosition[1] = Vec3(static_cast<float>(toPos.x) + 0.5f, static_cast<float>(toPos.y) + 0.5f, 0.0f);

    if (m_definition->m_slide)
        m_animationCallback = &ChessPiece::ChessMoveSlide;
    else
        m_animationCallback = &ChessPiece::ChessMoveJump;
    return this;
}

ChessPiece* ChessPiece::SetChessPromotion(ChessPieceDefinition* fromDef, ChessPieceDefinition* toDef)
{
    UNUSED(fromDef)
    LOG(LogGame, Info, "ChessPiece::SetChessPromotion to %s", toDef->m_name.c_str());
    m_definition = toDef;
    UpdateGlyph();
    m_meshComponent->SetModel(m_definition->m_model);
    if (m_faction == 1) // Need rotation
        m_orientation = EulerAngles(180, 0, 0);
    return this;
}

ChessPiece* ChessPiece::UpdateGlyph()
{
    switch (m_faction)
    {
    case 0:
        glyph = Common::ToUpper(m_definition->m_glyph);
        return this;
    case 1:
        glyph = Common::ToLower(m_definition->m_glyph);
        return this;
    default:
        break;
    }
    return this;
}

bool ChessPiece::SetEnableHighlight(bool newEnable)
{
    m_bIsHighlighted = newEnable;
    m_squareHighlight->SetEnable(newEnable);
    return newEnable;
}


void ChessPiece::OnTick(float deltaTime)
{
    ChessObject::OnTick(deltaTime);
    UpdateAnimation();
}

void ChessPiece::UpdateAnimation()
{
    if (m_animationTimer->IsStopped()) return;
    if (m_animationTimer->HasPeriodElapsed())
        m_animationTimer->Stop();
    else
    {
        if (m_animationCallback)
        {
            (this->*m_animationCallback)();
        }
    }
}

void ChessPiece::ChessMoveSlide()
{
    float fraction = m_animationTimer->GetElapsedFraction();
    m_position     = Interpolate(m_animationPosition[0], m_animationPosition[1], Hesitate3(fraction));
}

void ChessPiece::ChessMoveJump()
{
    float fraction = m_animationTimer->GetElapsedFraction();
    float xyInterp = SmoothStop2(fraction);

    m_position.x = Interpolate(m_animationPosition[0].x, m_animationPosition[1].x, xyInterp);
    m_position.y = Interpolate(m_animationPosition[0].y, m_animationPosition[1].y, xyInterp);

    // Use SmoothStep3 to create a parabola curve with a convex middle
    float parabolaFraction = 4.f * fraction * (1.f - fraction); // 0->1->0 curve, the peak is 1 when it is 0.5
    m_position.z           = m_animationJumpHeight * parabolaFraction;
}
