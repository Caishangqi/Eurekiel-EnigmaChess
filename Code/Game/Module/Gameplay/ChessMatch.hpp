#pragma once
#include <map>
#include <vector>

#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Renderer/Light/Light.hpp"
#include "Game/Core/Serilization/Serializable.hpp"
#include "Game/Module/Lib/ChessMatchCommon.hpp"

class EffectBloom;
class Game;
class ChessPlayer;
class ChessPiece;
class ChessBoard;
class Actor;

struct Faction
{
    std::string m_displayName     = "";
    int         m_id              = -1;
    Rgba8       m_color           = Rgba8::WHITE;
    Vec3        m_viewPosition    = Vec3::ZERO;
    EulerAngles m_viewOrientation = EulerAngles();

    friend bool operator==(const Faction& lhs, const Faction& rhs)
    {
        return lhs.m_id == rhs.m_id;
    }

    friend bool operator!=(const Faction& lhs, const Faction& rhs)
    {
        return !(lhs == rhs);
    }
};

class ChessMatch : public ISerializable
{
    friend class ChessPiece;
    friend class WidgetDebugPanel;
    friend class ChessPlayer;
    friend class ChessBoard;

public:
    ChessMatch(Game* game);
    ~ChessMatch() override;

    void        FromXML(const XmlElement& xmlElement) override;
    XmlElement* ToXML() const override;

    void Update();

    /// Spawn Actor in Map (Match)
        /// @param position 
        /// @param orientation 
        /// @param actor 
        /// @return The pointer of the Actor;
    Actor* SpawnActor(Vec3 position, EulerAngles orientation, Actor* actor);
    ChessPiece* AddChessPieceToMatch(ChessPiece* chessPiece, std::string gridPosition);
    ChessPiece* ExecuteChessMove(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo, Strings meta); // Move the Chess Actor, return the Moved Chess Actor if move success.
    ChessMatchCommon::MoveResult ExecuteChessTeleport(IntVec2 fromPos, IntVec2 toPos, std::string strFrom, std::string strTo, Strings meta);
    ChessPlayer* GetCurrentTurnPlayer() { return m_players[m_currentPlayerIndex]; }
    const ChessPlayer* StepNextTurn();

    /// Raycast
    [[nodiscard]]
    ChessMatchCommon::RaycastResultChess Raycast(const Vec3& origin, const Vec3& direction, float maxDistance) const;

    std::vector<Faction>      m_factions;
    std::vector<ChessPlayer*> m_players;
    int                       m_currentPlayerIndex = 0;
    int                       m_turnCounter        = 0;

protected:
    ChessBoard*         m_chessBoard = nullptr;
    std::vector<Actor*> m_actors; /// Board data Layout
    ChessGrid           m_chessGrid;

    /// Select and highlight
    IntVec2     m_impactSquare      = IntVec2::INVALID;
    IntVec2     m_highLightedSquare = IntVec2::INVALID;
    ChessPiece* m_selectedPiece     = nullptr;


    Game* m_game = nullptr;

    void ClearPawnDoubleMoveFlags(); ///< Called at the end of each round

    /// Test Lights
    Light m_pointLight;
    Light m_spotLight;

    /// Test PostProcess
    EffectBloom* m_bloomEffect = nullptr;
};
