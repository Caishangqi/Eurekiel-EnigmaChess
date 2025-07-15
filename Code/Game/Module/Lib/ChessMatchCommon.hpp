#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Mat44.hpp"
#include "Engine/Math/Vec3.hpp"

class ChessPlayer;
class ChessObject;
class ChessPiece;
enum class ECameraState;
class ChessMatch;
struct Faction;
class Actor;
struct IntVec2;

using ChessGrid = std::vector<Actor*>[8];

namespace ChessMatchCommon
{
    enum class EGameMode
    {
        SINGLE_PLAYER, // 单人模式（本地AI或练习模式）
        MULTIPLAYER_HOST, // 多人模式 - 作为主机
        MULTIPLAYER_CLIENT, // 多人模式 - 作为客户端
        SPECTATOR // 观战模式
    };

    enum class ChessMoveResult
    {
        VALID_MOVE_NORMAL,
        VALID_MOVE_TELEPORT,
        VALID_MOVE_PROMOTION,
        VALID_CAPTURE_PROMOTION,
        VALID_CASTLE_KINGSIDE,
        VALID_CASTLE_QUEENSIDE,
        VALID_CAPTURE_NORMAL,
        VALID_CAPTURE_TELEPORT,
        VALID_CAPTURE_ENPASSANT,

        UNKNOWN,
        INVALID_MOVE_BAD_LOCATION,
        INVALID_MOVE_NO_PIECE,
        INVALID_MOVE_NOT_YOUR_PIECE,
        INVALID_MOVE_ZERO_DISTANCE,
        INVALID_MOVE_WRONG_MOVE_SHAPE,
        INVALID_MOVE_DESTINATION_BLOCKED,
        INVALID_MOVE_PATH_BLOCKED,
        INVALID_MOVE_ENDS_IN_CHECK,
        INVALID_ENPASSANT_STALE,
        INVALID_CASTLE_KING_HAS_MOVED,
        INVALID_CASTLE_ROOK_HAS_MOVED,
        INVALID_CASTLE_PATH_BLOCKED,
        INVALID_CASTLE_THROUGH_CHECK,
        INVALID_CASTLE_OUT_OF_CHECK
    };

    inline const char* to_string(ChessMoveResult e)
    {
        switch (e)
        {
        case ChessMoveResult::VALID_MOVE_NORMAL: return "Valid move";
        case ChessMoveResult::VALID_MOVE_PROMOTION: return "Valid move, resulting in pawn promotion";
        case ChessMoveResult::VALID_CAPTURE_PROMOTION: return "Valid capture promotion, resulting in pawn promotion";
        case ChessMoveResult::VALID_MOVE_TELEPORT: return "Valid move with teleporting";
        case ChessMoveResult::VALID_CAPTURE_TELEPORT: return "Valid move, capturing enemy piece with teleports";
        case ChessMoveResult::VALID_CASTLE_KINGSIDE: return "Valid move, castling kingside";
        case ChessMoveResult::VALID_CASTLE_QUEENSIDE: return "Valid move, castling queenside";
        case ChessMoveResult::VALID_CAPTURE_NORMAL: return "Valid move, capturing enemy piece";
        case ChessMoveResult::VALID_CAPTURE_ENPASSANT: return "Valid move, capturing enemy pawn en passant";
        case ChessMoveResult::INVALID_MOVE_BAD_LOCATION: return "Invalid move; invalid board location given";
        case ChessMoveResult::INVALID_MOVE_NO_PIECE: return "Invalid move; no piece at location given";
        case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE: return "Invalid move; can't move opponent's piece";
        case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE: return "Invalid move; didn't go anywhere";
        case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE: return "Invalid move; invalid shape for that pieces";
        case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED: return "Invalid move; destination is blocked by your piece";
        case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED: return "Invalid move; path is blocked by your piece";
        case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK: return "Invalid move; can't leave yourself in check";
        case ChessMoveResult::INVALID_ENPASSANT_STALE: return "Invalid move; en passant must immediately follow a pawn double-move";
        case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED: return "Invalid castle; king has moved previously";
        case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED: return "Invalid castle; that rook has moved previously";
        case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED: return "Invalid castle; pieces in-between king and rook";
        case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK: return "Invalid castle; king can't move through check";
        case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK: return "Invalid castle; king can't castle out of check";
        case ChessMoveResult::UNKNOWN:
            ERROR_AND_DIE(Stringf("Unhandled ChessMoveResult enum value #%d", e))
        }
        return "UNKNOWN";
    }


    struct MoveResult
    {
        IntVec2         m_fromPosition = IntVec2::ZERO;
        std::string     m_fromPositionString;
        IntVec2         m_toPosition = IntVec2::ZERO;
        std::string     m_toPositionString;
        ChessMoveResult m_moveResult    = ChessMoveResult::UNKNOWN;
        ChessPiece*     m_piecesMove    = nullptr;
        ChessPiece*     m_piecesCapture = nullptr;
    };

    struct RaycastResultChess
    {
        ChessObject* m_hitObject = nullptr;
        // Basic raycast result information (required)
        bool  m_didImpact  = false;
        float m_impactDist = 0.f;
        Vec3  m_impactPos;
        Vec3  m_impactNormal;

        // Original raycast information (optional)
        Vec3  m_rayFwdNormal;
        Vec3  m_rayStartPos;
        float m_rayMaxLength = 1.f;
    };


    [[nodiscard]] IntVec2  GetGridPosition(std::string strPos);
    [[nodiscard]] bool     GetStringPositionValidation(std::string strPos);
    [[nodiscard]] Faction* GetFaction(int fromID, ChessMatch* match);
    Mat44                  GetCameraTransform(ECameraState state, Vec3& position, EulerAngles& rotation, ChessMatch* match, std::string configValue = "above");
    bool                   GetAllPieces(ChessGrid& grid, std::vector<ChessPiece*>& pieces);
    [[nodiscard]] bool     GetChessMoveValid(MoveResult result);
    void                   PrintChessGrid(ChessGrid& grid);
    /// Get the args pair with specific key and entire Event args, the function will automatically split the args by split char and find the key
    /// index in the event args.
    /// @param inArgs the command prompt input args.
    /// @param key the key user want to find in the Event args.
    /// @param pair the returned split pair, empty if not found.
    /// @param split the split Delimiter.
    /// @return -1 if not find the key in args, return the index valid kay value pairs in the Event args.
    int  GetCommandArgsWith(EventArgs& inArgs, std::string key, std::pair<std::string, std::string>& pair, std::string& outMessage, char split = '=');
    bool GetCommandHasValidSubArgs(EventArgs& inArgs, Strings& validSubArgs);
    int  GetCommandStringsWith(Strings& inStrings, std::string key, std::pair<std::string, std::string>& pair, std::string& outMessage, char split = '=');
    bool IsTrueString(std::string& inString);

    bool Command_RemoteCmd(EventArgs& args);
    bool Command_ChessPlayerInfo(EventArgs& args);
    bool Command_ChessBegin(EventArgs& args);
    bool Command_ChessMove(EventArgs& args);
    bool Command_ChessMatch(EventArgs& args);
    bool Command_ChessServerInfo(EventArgs& args);
    bool Command_ChessListen(EventArgs& args);
    bool Command_ChessConnect(EventArgs& args);
    bool Command_ChessDisconnect(EventArgs& args);

    [[maybe_unused]] bool SendRemoteCommand(const std::string& command);

    bool        IsMultiplayerMode();
    bool        IsLocalPlayerTurn(ChessPlayer* currentPlayer);
    std::string GetGameModeString();
    IntVec2     StringToGridPos(const std::string& notation);
    std::string GridPosToChessNotation(const IntVec2& pos);
}
