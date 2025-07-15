#include "ChessMatchCommon.hpp"

#include <regex>

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/NamedStrings.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Network/NetworkSubsystem.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Player.hpp"
#include "Game/Module/Definition/ChessPieceDefinition.hpp"
#include "Game/Module/Gameplay/ChessMatch.hpp"
#include "Game/Module/Gameplay/ChessPiece.hpp"
#include "Game/Module/Gameplay/ChessPlayer.hpp"

IntVec2 ChessMatchCommon::GetGridPosition(std::string strPos)
{
    if (!GetStringPositionValidation(strPos)) return IntVec2::INVALID;

    char col = static_cast<char>(std::toupper(strPos[0]));
    int  row = std::stoi(strPos.substr(1));

    int x = col - 'A'; // 'A'->0, 'B'->1, ..., 'H'->7
    int y = row - 1; // '1'->0, '2'->1, ..., '8'->7

    return IntVec2(x, y);
}

bool ChessMatchCommon::GetStringPositionValidation(std::string strPos)
{
    if (strPos.length() < 2 || strPos.length() > 3) return false;

    char col = static_cast<char>(std::toupper(strPos[0]));
    if (col < 'A' || col > 'H') return false;

    std::string rowPart = strPos.substr(1);
    for (char ch : rowPart)
    {
        if (!isdigit(ch)) return false;
    }

    int row = std::stoi(rowPart);
    if (row < 1 || row > 8) return false;

    return true;
}

Faction* ChessMatchCommon::GetFaction(int fromID, ChessMatch* match)
{
    for (Faction& faction : match->m_factions)
    {
        if (faction.m_id == fromID) return &faction;
    }
    return nullptr;
}

Mat44 ChessMatchCommon::GetCameraTransform(ECameraState state, Vec3& position, EulerAngles& rotation, ChessMatch* match, std::string configValue)
{
    Mat44 mat;
    if (g_theGame->cameraMode == ECameraMode::FREE)
    {
        return mat;
    }
    switch (state)
    {
    case ECameraState::NONE:
        return mat;
    case ECameraState::PER_PLAYER:
        {
            if (!match)
                return mat;
            mat      = match->GetCurrentTurnPlayer()->GetModelToWorldTransform();
            position = match->GetCurrentTurnPlayer()->m_position;
            rotation = match->GetCurrentTurnPlayer()->m_orientation;
            return mat;
        }
    case ECameraState::CONFIGURED:
        {
            XmlElement*       root              = g_theGame->m_chessMatchConfig.RootElement();
            const XmlElement* cameraPosElements = FindChildElementByName(*root, "CameraPositions");
            const XmlElement* elementCameraPos  = cameraPosElements->FirstChildElement();
            while (elementCameraPos != nullptr)
            {
                std::string strConfigValue = ParseXmlAttribute(*elementCameraPos, "name", std::string("INVALID"));
                if (strConfigValue == configValue)
                {
                    position = ParseXmlAttribute(*elementCameraPos, "position", Vec3(0, 0, 0));
                    rotation = EulerAngles(ParseXmlAttribute(*elementCameraPos, "orientation", Vec3(0, 0, 0)));
                    mat.AppendTranslation3D(position);
                    mat.Append(rotation.GetAsMatrix_IFwd_JLeft_KUp());
                    return mat;
                }
                elementCameraPos = elementCameraPos->NextSiblingElement();
            }
        }
        break;
    }
    return mat;
}

bool ChessMatchCommon::GetAllPieces(ChessGrid& grid, std::vector<ChessPiece*>& pieces)
{
    for (std::vector<Actor*>& actors : grid)
    {
        for (Actor* actor : actors)
        {
            auto piece = dynamic_cast<ChessPiece*>(actor);
            if (piece != nullptr)
                pieces.push_back(piece);
        }
    }
    if (pieces.empty()) return false;
    return true;
}

bool ChessMatchCommon::GetChessMoveValid(MoveResult result)
{
    switch (result.m_moveResult)
    {
    case ChessMoveResult::VALID_MOVE_NORMAL:
    case ChessMoveResult::VALID_MOVE_TELEPORT:
    case ChessMoveResult::VALID_CAPTURE_TELEPORT:
    case ChessMoveResult::VALID_MOVE_PROMOTION:
    case ChessMoveResult::VALID_CAPTURE_PROMOTION:
    case ChessMoveResult::VALID_CASTLE_KINGSIDE:
    case ChessMoveResult::VALID_CASTLE_QUEENSIDE:
    case ChessMoveResult::VALID_CAPTURE_NORMAL:
    case ChessMoveResult::VALID_CAPTURE_ENPASSANT:
        return true;

    case ChessMoveResult::INVALID_MOVE_BAD_LOCATION:
    case ChessMoveResult::INVALID_MOVE_NO_PIECE:
    case ChessMoveResult::INVALID_MOVE_NOT_YOUR_PIECE:
    case ChessMoveResult::INVALID_MOVE_ZERO_DISTANCE:
    case ChessMoveResult::INVALID_MOVE_WRONG_MOVE_SHAPE:
    case ChessMoveResult::INVALID_MOVE_DESTINATION_BLOCKED:
    case ChessMoveResult::INVALID_MOVE_PATH_BLOCKED:
    case ChessMoveResult::INVALID_MOVE_ENDS_IN_CHECK:
    case ChessMoveResult::INVALID_ENPASSANT_STALE:
    case ChessMoveResult::INVALID_CASTLE_KING_HAS_MOVED:
    case ChessMoveResult::INVALID_CASTLE_ROOK_HAS_MOVED:
    case ChessMoveResult::INVALID_CASTLE_PATH_BLOCKED:
    case ChessMoveResult::INVALID_CASTLE_THROUGH_CHECK:
    case ChessMoveResult::INVALID_CASTLE_OUT_OF_CHECK:
        return false;

    case ChessMoveResult::UNKNOWN:
        ERROR_AND_DIE(Stringf( "Unhandled ChessMoveResult enum value #%d", result.m_moveResult ))
    }
    return false;
}

void ChessMatchCommon::PrintChessGrid(ChessGrid& grid)
{
    std::string ABC = "  ABCDEFGH  ";
    std::string upB = " +--------+ ";
    g_theDevConsole->AddLine(Rgba8::ORANGE, ABC);
    g_theDevConsole->AddLine(Rgba8::ORANGE, upB);
    for (int i = 7; i >= 0; i--)
    {
        std::string line = std::to_string(i + 1) + "|";
        for (int j = 0; j < static_cast<int>(grid[i].size()); j++)
        {
            auto        chessPiece = dynamic_cast<ChessPiece*>(grid[j][i]);
            std::string gly;
            if (chessPiece)
            {
                gly = chessPiece->GetGlyph();
            }
            else
            {
                gly = ".";
            }
            line.append(gly);
        }
        g_theDevConsole->AddLine(Rgba8::ORANGE, line + "|" + std::to_string(i + 1));
    }
    g_theDevConsole->AddLine(Rgba8::ORANGE, upB);
    g_theDevConsole->AddLine(Rgba8::ORANGE, ABC);
}

int ChessMatchCommon::GetCommandArgsWith(EventArgs& inArgs, std::string key, std::pair<std::string, std::string>& pair, std::string& outMessage, char split)
{
    std::string arg = inArgs.GetValue("args", std::string(""));
    if (arg == "")
    {
        outMessage = "Invalid args,the correct usage is > ChessMove from=<> to=<> promoteTo=<>";
        return -1;
    }
    Strings args = SplitStringOnDelimiter(arg, ' ');
    if (args.size() == 0)
    {
        outMessage = "Invalid args,the correct usage is > ChessMove from=<> to=<> promoteTo=<>";
        return -1;
    }
    // we search the key value
    for (int i = 0; i < static_cast<int>(args.size()); i++)
    {
        Strings keyValue = SplitStringOnDelimiter(args[i], split);
        if (keyValue.size() != 2)
        {
            outMessage = "Invalid args,the correct usage is > ChessMove from=<> to=<> promoteTo=<>";
            return -1;
        }
        if (Common::ToUpper(keyValue[0]) == Common::ToUpper(key))
        {
            pair.first  = Common::ToUpper(keyValue[0]);
            pair.second = Common::ToUpper(keyValue[1]);
            return i;
        }
    }
    outMessage = "Unknown args";
    return -1;
}

bool ChessMatchCommon::GetCommandHasValidSubArgs(EventArgs& inArgs, Strings& validSubArgs)
{
    UNUSED(validSubArgs)
    UNUSED(inArgs)
    return false;
}

int ChessMatchCommon::GetCommandStringsWith(Strings& inStrings, std::string key, std::pair<std::string, std::string>& pair, std::string& outMessage, char split)
{
    if (inStrings.size() == 0)
    {
        outMessage = "Invalid args, caused by upstream code :(";
        return -1;
    }
    for (int i = 0; i < static_cast<int>(inStrings.size()); i++)
    {
        Strings keyValue = SplitStringOnDelimiter(inStrings[i], split);
        if (keyValue.size() != 2)
        {
            outMessage = "Invalid args, the key value pair should have maximum of 2.";
            return -1;
        }
        if (Common::ToUpper(keyValue[0]) == Common::ToUpper(key))
        {
            pair.first  = Common::ToUpper(keyValue[0]);
            pair.second = Common::ToUpper(keyValue[1]);
            return i;
        }
    }
    outMessage = "Unknown args";
    return -1;
}

bool ChessMatchCommon::IsTrueString(std::string& inString)
{
    return Common::ToUpper(inString) == "TRUE";
}

/**
 * Processes a remote command that can be sent over the network and executed on a remote computer's DevConsole.
 * Constructs a DevConsole command string based on the provided arguments, omitting the "RemoteCmd" and "cmd=" segments
 * from the final string. Once received by the remote system, the command string will have "remote=true" appended,
 * and then executed in the remote DevConsole system.
 *
 * @param args The arguments used to construct the command string. It should include the "cmd" key specifying
 *             the command name, and optionally additional key-value pairs for command parameters.
 * @return Returns true if the command was successfully processed and sent. Returns false if the command processing failed.
 */
bool ChessMatchCommon::Command_RemoteCmd(EventArgs& args)
{
    std::pair<std::string, std::string> cmd;
    std::string                         outMessage;
    GetCommandArgsWith(args, "cmd", cmd, outMessage);

    if (cmd.second.empty() || cmd.first.empty())
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR,
                                 "Fail to send remote command, invalid arguments");
        return false;
    }

    std::string arg = args.GetValue("args", std::string(""));
    std::regex  re("cmd=", std::regex_constants::icase);
    arg = std::regex_replace(arg, re, "");

    // 使用新的字符串发送 API（自动处理消息边界）
    if (g_theNetworkSubsystem->GetClientState() == ClientState::CONNECTED)
    {
        g_theNetworkSubsystem->SendStringToServer(arg);
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                 "Command sent to server: " + arg);
        return true;
    }

    if (g_theNetworkSubsystem->GetServerState() == ServerState::LISTENING)
    {
        g_theNetworkSubsystem->BroadcastStringToClients(arg);
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                 "Command broadcasted to clients: " + arg);
        return true;
    }

    g_theDevConsole->AddLine(DevConsole::COLOR_ERROR,
                             "Not connected to send command");
    return false;
}

/**
 * Handles the "ChessPlayerInfo" command to set player names in a chess match session. When the command
 * is received locally, it sets the local player's name based on the provided arguments. If the command
 * is received from a remote host with the "remote=true" argument, it instead sets the opponent's player name.
 *
 * @param args The arguments used to configure the player name. It must include the "name" key
 *             specifying the player name. Optionally, it can include the "remote" key to denote
 *             if the operation is for a remote opponent.
 * @return Returns true if the player name was configured successfully. Returns false if the
 *         configuration failed.
 */
bool ChessMatchCommon::Command_ChessPlayerInfo(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> name;
    std::pair<std::string, std::string> remote;

    // 解析命令参数
    GetCommandArgsWith(args, "name", name, outMessage);
    GetCommandArgsWith(args, "remote", remote, outMessage);

    if (name.second.empty())
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR,
                                 "ChessPlayerInfo requires name parameter. Usage: ChessPlayerInfo name=<playerName>");
        return false;
    }

    // 检查是否是远程命令
    bool isRemote = !remote.second.empty() && IsTrueString(remote.second);

    if (!g_theGame || !g_theGame->match)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, "No active chess match found");
        return false;
    }

    ChessMatch* match = g_theGame->match;

    if (isRemote)
    {
        // 远程命令：设置对手的名字
        if (match->m_players.size() >= 2)
        {
            int          localFactionId = g_theGame->m_localPlayerFactionId;
            ChessPlayer* opponentPlayer = nullptr;

            // 找到不是本地玩家的另一个玩家
            for (ChessPlayer* player : match->m_players)
            {
                if (player && player->m_faction.m_id != localFactionId)
                {
                    opponentPlayer = player;
                    break;
                }
            }

            if (opponentPlayer)
            {
                opponentPlayer->m_faction.m_displayName = name.second;
                outMessage                              = Stringf("Opponent name set to: %s (Faction ID: %d)",
                                     name.second.c_str(), opponentPlayer->m_faction.m_id);
                g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

                // 更新对应的faction
                for (auto& faction : match->m_factions)
                {
                    if (faction.m_id == opponentPlayer->m_faction.m_id)
                    {
                        faction.m_displayName = name.second;
                        break;
                    }
                }
            }
            else
            {
                g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                         Stringf("Could not find opponent player (Local faction: %d)", localFactionId));
            }
        }
        else
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Not enough players to set opponent name");
        }
    }
    else
    {
        // 本地命令：设置自己的名字并通过RemoteCmd发送给远程
        if (!match->m_players.empty())
        {
            int          localFactionId = g_theGame->m_localPlayerFactionId;
            ChessPlayer* localPlayer    = nullptr;

            for (ChessPlayer* player : match->m_players)
            {
                if (player && player->m_faction.m_id == localFactionId)
                {
                    localPlayer = player;
                    break;
                }
            }

            if (localPlayer)
            {
                localPlayer->m_faction.m_displayName = name.second;
                outMessage                           = Stringf("Local player name set to: %s (Faction ID: %d)",
                                     name.second.c_str(), localPlayer->m_faction.m_id);
                g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

                // 更新对应的faction
                for (auto& faction : match->m_factions)
                {
                    if (faction.m_id == localPlayer->m_faction.m_id)
                    {
                        faction.m_displayName = name.second;
                        break;
                    }
                }

                // ✅ 使用RemoteCmd系统发送给远程
                std::string remoteCommand = Stringf("ChessPlayerInfo name=%s", name.second.c_str());
                SendRemoteCommand(remoteCommand);
            }
            else
            {
                g_theDevConsole->AddLine(DevConsole::COLOR_ERROR,
                                         Stringf("Could not find local player (Faction ID: %d)", localFactionId));
            }
        }
    }

    return true;
}

/**
 * Handles the "ChessBegin" command to start a new chess match.
 * When executed locally, it starts a new game and sends the command to remote.
 * When received remotely, it starts the game without re-sending.
 *
 * @param args The arguments that may include "firstPlayer" to specify who goes first.
 *             If omitted, the local player goes first.
 * @return Returns true if the game was started successfully, false otherwise.
 */
bool ChessMatchCommon::Command_ChessBegin(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> firstPlayer;
    std::pair<std::string, std::string> remote;

    // 解析命令参数
    GetCommandArgsWith(args, "firstPlayer", firstPlayer, outMessage);
    GetCommandArgsWith(args, "remote", remote, outMessage);

    bool isRemote = !remote.second.empty() && IsTrueString(remote.second);

    if (!g_theGame || !g_theGame->match)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, "No active chess match found");
        return false;
    }

    ChessMatch* match = g_theGame->match;

    // 检查是否有足够的玩家
    if (match->m_players.size() < 2)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                 "Need at least 2 players to start a chess game");
        return false;
    }

    // 重置游戏状态
    match->m_turnCounter = 0;

    // 确定首发玩家
    std::string firstPlayerName     = firstPlayer.second;
    int         startingPlayerIndex = 0;

    if (!firstPlayerName.empty())
    {
        // 查找指定的首发玩家（支持大小写不敏感匹配）
        bool found = false;
        for (int i = 0; i < static_cast<int>(match->m_players.size()); i++)
        {
            std::string playerName = match->m_players[i]->m_faction.m_displayName;

            // 大小写不敏感比较
            if (Common::ToUpper(playerName) == Common::ToUpper(firstPlayerName))
            {
                startingPlayerIndex = i;
                found               = true;
                break;
            }
        }

        if (!found)
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                     Stringf("Player '%s' not found, using default starting player", firstPlayerName.c_str()));
        }
    }
    else if (!isRemote)
    {
        // 如果是本地命令且没有指定首发玩家，使用本地玩家
        int localFactionId = g_theGame->m_localPlayerFactionId;
        for (int i = 0; i < static_cast<int>(match->m_players.size()); i++)
        {
            if (match->m_players[i]->m_faction.m_id == localFactionId)
            {
                startingPlayerIndex = i;
                firstPlayerName     = match->m_players[i]->m_faction.m_displayName;
                break;
            }
        }
    }

    // 设置首发玩家
    match->m_currentPlayerIndex = startingPlayerIndex;

    // 重置棋盘
    //g_theGame->ChessMatchReset();

    // 输出游戏开始信息
    outMessage = Stringf("Chess game started! First player: %s (Faction ID: %d)",
                         match->GetCurrentTurnPlayer()->m_faction.m_displayName.c_str(),
                         match->GetCurrentTurnPlayer()->m_faction.m_id);
    g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

    // 更新游戏状态
    //g_theGame->EnterState(EGameState::MATCH);

    // 如果是本地命令，通过RemoteCmd发送给远程
    if (!isRemote)
    {
        std::string remoteCommand;
        if (firstPlayerName.empty())
        {
            remoteCommand = "ChessBegin";
        }
        else
        {
            remoteCommand = Stringf("ChessBegin firstPlayer=%s", firstPlayerName.c_str());
        }

        SendRemoteCommand(remoteCommand);
    }

    return true;
}


bool ChessMatchCommon::Command_ChessMove(EventArgs& args)
{
    Strings     validSubcommand  = {"from", "to", "promoteTo", "teleport", "remote"};
    std::string errorInvalidArgs = "Invalid args, the correct usage is > ChessMove from=<> to=<> promoteTo=<>";

    /// Game state Checking
    if (g_theGame->gameState != EGameState::MATCH)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "ChessMove can only be used when in Match");
        return true;
    }

    /// Check if we have an active match
    if (!g_theGame->match)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, "No active chess match found");
        return false;
    }

    std::string arg = args.GetValue("args", std::string(""));

    /// Syntax Checking
    if (arg == "")
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return true;
    }

    std::string outMessage;

    /// Parse remote parameter to check if this is a remote command
    std::pair<std::string, std::string> remote;
    bool                                isRemoteCommand = (GetCommandArgsWith(args, "remote", remote, outMessage) != -1) &&
        IsTrueString(remote.second);

    /// Parse movement parameters
    std::pair<std::string, std::string> fromPair;
    std::pair<std::string, std::string> toPair;

    int fromResult = GetCommandArgsWith(args, "from", fromPair, outMessage);
    int toResult   = GetCommandArgsWith(args, "to", toPair, outMessage);

    if (fromResult == -1 || toResult == -1)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return false;
    }

    /// Parse teleport parameter
    std::pair<std::string, std::string> teleportPair;
    bool                                isTeleportMove = (GetCommandArgsWith(args, "teleport", teleportPair, outMessage) != -1) &&
        IsTrueString(teleportPair.second);

    /// Parse promotion parameter
    std::pair<std::string, std::string> promotionPair;
    int                                 promotionResult = GetCommandArgsWith(args, "promoteTo", promotionPair, outMessage);
    bool                                hasPromotion    = (promotionResult != -1);

    if (hasPromotion)
    {
        ChessPieceDefinition* def = ChessPieceDefinition::GetByName(promotionPair.second);
        if (!def || def->m_name == "Pawn" || def->m_name == "King")
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                     "Invalid promotion piece. Valid pieces: Queen, Rook, Bishop, Knight");
            return false;
        }
    }

    /// Convert chess notation to grid positions
    IntVec2 fromPos = StringToGridPos(fromPair.second);
    IntVec2 toPos   = StringToGridPos(toPair.second);

    if (fromPos == IntVec2::INVALID || toPos == IntVec2::INVALID)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Invalid chess square notation");
        return false;
    }

    /// Network mode checking
    bool isMultiplayerMode = IsMultiplayerMode();

    /// In multiplayer mode, check if it's the correct player's turn
    if (isMultiplayerMode && isRemoteCommand)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Executing remote chess move");
    }
    else if (isMultiplayerMode && !isRemoteCommand)
    {
        ChessMatch*  match         = g_theGame->match;
        ChessPlayer* currentPlayer = match->GetCurrentTurnPlayer();

        if (!IsLocalPlayerTurn(currentPlayer))
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "It's not your turn to move");
            return false;
        }
    }

    /// Execute the move locally
    ChessMatch* match      = g_theGame->match;
    ChessPiece* movedPiece = nullptr;

    Strings meta;
    if (hasPromotion)
    {
        meta.push_back("promoteTo=" + promotionPair.second);
    }

    if (isTeleportMove)
    {
        auto teleportResult = match->ExecuteChessTeleport(fromPos, toPos, fromPair.second, toPair.second, meta);
        if (teleportResult.m_moveResult != ChessMoveResult::VALID_MOVE_TELEPORT &&
            teleportResult.m_moveResult != ChessMoveResult::VALID_CAPTURE_TELEPORT)
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING,
                                     "Teleport move failed: " + std::string(to_string(teleportResult.m_moveResult)));
            return false;
        }
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Teleport move executed successfully");
    }
    else
    {
        movedPiece = match->ExecuteChessMove(fromPos, toPos, fromPair.second, toPair.second, meta);
        if (!movedPiece)
        {
            return false;
        }

        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                 Stringf("Chess move executed: %s to %s",
                                         fromPair.second.c_str(), toPair.second.c_str()));
    }

    /// Network synchronization for multiplayer mode using RemoteCmd
    if (isMultiplayerMode && !isRemoteCommand)
    {
        // Build the remote command string
        std::string remoteCommand = Stringf("ChessMove from=%s to=%s",
                                            fromPair.second.c_str(), toPair.second.c_str());

        // Add promotion if specified
        if (hasPromotion)
        {
            remoteCommand += Stringf(" promoteTo=%s", promotionPair.second.c_str());
        }

        // Add teleport if specified
        if (isTeleportMove)
        {
            remoteCommand += " teleport=true";
        }

        SendRemoteCommand(remoteCommand);
    }
    else if (isRemoteCommand)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Remote chess move processed successfully");
    }

    return true;
}

bool ChessMatchCommon::IsMultiplayerMode()
{
    if (!g_theGame)
        return false;

    return g_theGame->GetGameMode() == EGameMode::MULTIPLAYER_HOST ||
        g_theGame->GetGameMode() == EGameMode::MULTIPLAYER_CLIENT;
}

bool ChessMatchCommon::IsLocalPlayerTurn(ChessPlayer* currentPlayer)
{
    if (!currentPlayer || !g_theGame || !g_theGame->match)
        return false;

    // 在单人模式下，总是允许本地操作
    if (g_theGame->GetGameMode() == EGameMode::SINGLE_PLAYER)
    {
        return true;
    }

    // 在多人模式下，检查当前玩家是否是本地玩家
    if (g_theGame->GetGameMode() == EGameMode::MULTIPLAYER_HOST ||
        g_theGame->GetGameMode() == EGameMode::MULTIPLAYER_CLIENT)
    {
        // 使用存储的本地玩家阵营ID来判断
        int localFactionId   = g_theGame->m_localPlayerFactionId;
        int currentFactionId = currentPlayer->m_faction.m_id;

        bool isLocalTurn = (currentFactionId == localFactionId);

        // 调试信息（可选，便于调试）
        /*
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, 
                               Stringf("Turn check: Current=%d, Local=%d, IsLocalTurn=%s", 
                                      currentFactionId, localFactionId, 
                                      isLocalTurn ? "YES" : "NO"));
        */

        return isLocalTurn;
    }

    // 观战模式：不允许任何移动
    if (g_theGame->GetGameMode() == EGameMode::SPECTATOR)
    {
        return false;
    }

    // 默认情况：不允许移动
    return false;
}

std::string ChessMatchCommon::GetGameModeString()
{
    if (!g_theGame)
        return "UNKNOWN";

    switch (g_theGame->GetGameMode())
    {
    case EGameMode::SINGLE_PLAYER:
        return "SINGLE_PLAYER";
    case EGameMode::MULTIPLAYER_HOST:
        return "MULTIPLAYER_HOST";
    case EGameMode::MULTIPLAYER_CLIENT:
        return "MULTIPLAYER_CLIENT";
    case EGameMode::SPECTATOR:
        return "SPECTATOR";
    default:
        return "UNKNOWN";
    }
}

/// Helper function to convert chess notation to grid position
IntVec2 ChessMatchCommon::StringToGridPos(const std::string& notation)
{
    if (notation.length() != 2)
        return IntVec2::INVALID;

    char file = notation[0]; // a-h
    char rank = notation[1]; // 1-8

    if (file < 'a' || file > 'h' || rank < '1' || rank > '8')
        return IntVec2::INVALID;

    int x = file - 'a'; // 0-7
    int y = rank - '1'; // 0-7

    return IntVec2(x, y);
}

bool ChessMatchCommon::Command_ChessMatch(EventArgs& args)
{
    std::string strArg  = args.GetValue("args", std::string(""));
    Strings     strArgs = SplitStringOnDelimiter(strArg, ' ');
    if (strArgs.size() == 1)
    {
        if (strArgs[0] == "reset")
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR, "Reset Chess Match...");
            g_theGame->ChessMatchReset();
            return true;
        }
    }
    return true;
}

/**
 * Modifies or retrieves the server configuration information (IP address and port) for the Chess network system.
 * If no specific arguments are provided, it displays the current server configuration.
 * If a new IP or port is provided and the client is not connected, the server configuration will be updated.
 * Provides error feedback if a connected client attempts to modify the server configuration.
 *
 * @param args The event arguments containing the keys "ip" and/or "port" along with their values.
 *             If "ip" or "port" is not provided or empty, the function simply retrieves the current server info.
 * @return Returns true if operation was successful such as displaying the server information or updating the server config.
 *         Returns false if the operation failed, such as attempting to set configuration while connected to a server.
 */
bool ChessMatchCommon::Command_ChessServerInfo(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> ip;
    std::pair<std::string, std::string> port;
    GetCommandArgsWith(args, "ip", ip, outMessage);
    GetCommandArgsWith(args, "port", port, outMessage);

    if (ip.second.empty() && port.second.empty())
    {
        // 显示所有信息，包括网络模式
        NetworkStats stats = g_theNetworkSubsystem->GetNetworkStatistics();

        auto sendModeStr = "";
        switch (stats.currentSendMode)
        {
        case SendMode::BLOCKING: sendModeStr = "BLOCKING";
            break;
        case SendMode::NON_BLOCKING: sendModeStr = "NON_BLOCKING";
            break;
        case SendMode::ADAPTIVE: sendModeStr = "ADAPTIVE";
            break;
        }

        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_MINOR,
                                 Stringf("Server Info:\n"
                                         "  IP: %s, Port: %d\n"
                                         "  Send Mode: %s\n"
                                         "  Queue Size: %zu bytes\n"
                                         "  Connections: %zu\n"
                                         "  Performance Limited: %s",
                                         g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                         g_theNetworkSubsystem->GetConfig().serverPort,
                                         sendModeStr,
                                         stats.outgoingQueueSize,
                                         stats.activeConnections,
                                         stats.isNetworkLimited ? "YES" : "NO"));
        return true;
    }

    // 现有的设置IP/端口逻辑保持不变
    if (g_theNetworkSubsystem)
    {
        if (g_theNetworkSubsystem->GetClientState() == ClientState::CONNECTED)
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_ERROR,
                                     "You can not set Server Info when connected to a server");
            return false;
        }
        if (!ip.second.empty())
            g_theNetworkSubsystem->GetConfig().serverIp = ip.second;
        if (!port.second.empty())
            g_theNetworkSubsystem->GetConfig().serverPort = static_cast<uint16_t>(atoi(port.second.c_str()));

        g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN,
                                 Stringf("Set Server Config to: IP=%s, Port=%d",
                                         g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                         g_theNetworkSubsystem->GetConfig().serverPort));
        return true;
    }

    return false;
}

/**
 * Starts a server for the Chess network system and initializes a listen socket on a specified port.
 * If no port is provided in the arguments, the system uses a default port of 3100 or a previously overridden port.
 *
 * @param args The event arguments containing the "port" key and its corresponding value. If "port" is not provided or empty, the function uses the default or previously specified port.
 * @return Returns true if the server and listen socket were successfully started.
 *         Returns false if the operation failed.
 */
bool ChessMatchCommon::Command_ChessListen(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> port;
    GetCommandArgsWith(args, "port", port, outMessage);

    if (g_theNetworkSubsystem->GetServerState() != ServerState::IDLE)
    {
        outMessage = Stringf("Fail to Listen, Server is already running or not initialized");
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        return false;
    }

    bool result = false;

    if (port.second.empty())
    {
        // 使用默认配置启动服务器
        result = g_theNetworkSubsystem->StartServer(g_theNetworkSubsystem->GetConfig().serverPort);

        if (result)
        {
            // 服务器启动成功 - 设置为多人主机模式
            g_theGame->SetGameMode(EGameMode::MULTIPLAYER_HOST);
            g_theGame->m_localPlayerFactionId = 0; // 主机通常是第一个玩家（阵营0）

            outMessage = Stringf("Server started successfully! IP=%s, Port=%d, Mode=MULTIPLAYER_HOST",
                                 g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                 g_theNetworkSubsystem->GetConfig().serverPort);
            g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                     "Game mode set to MULTIPLAYER_HOST. Waiting for clients to connect...");
        }
        else
        {
            outMessage = Stringf("Failed to start server with IP=%s, Port=%d",
                                 g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                 g_theNetworkSubsystem->GetConfig().serverPort);
            g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        }
    }
    else
    {
        // 使用指定端口启动服务器
        uint16_t portNumber = static_cast<uint16_t>(atoi(port.second.c_str()));
        result              = g_theNetworkSubsystem->StartServer(portNumber);

        if (result)
        {
            // 服务器启动成功 - 设置为多人主机模式
            g_theGame->SetGameMode(EGameMode::MULTIPLAYER_HOST);
            g_theGame->m_localPlayerFactionId = 0; // 主机通常是第一个玩家（阵营0）

            outMessage = Stringf("Server started successfully! IP=%s, Port=%d, Mode=MULTIPLAYER_HOST",
                                 g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                 portNumber);
            g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                     "Game mode set to MULTIPLAYER_HOST. Waiting for clients to connect...");
        }
        else
        {
            outMessage = Stringf("Failed to start server with IP=%s, Port=%d",
                                 g_theNetworkSubsystem->GetConfig().serverIp.c_str(),
                                 portNumber);
            g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        }
    }

    return result;
}

/**
 * Establishes a connection to a remote chess server using the specified IP address and port.
 * If arguments for IP or port are omitted, default values are used: IP=127.0.0.1 and port=3100.
 * The command uses existing, overridden, or default values as appropriate. If both IP and port are provided,
 * it attempts to connect using the specified values. If only IP is provided, the default port or previous configuration is used.
 *
 * @param args The event arguments that may include "ip" (string, optional) and "port" (integer, optional).
 *             - "ip": The IP address to connect to. Defaults to 127.0.0.1 if not provided.
 *             - "port": The port number for the connection. Defaults to 3100 if not provided.
 * @return Returns true if the connection to the server was successfully established.
 *         Returns false if the connection attempt fails.
 */
bool ChessMatchCommon::Command_ChessConnect(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> ip;
    std::pair<std::string, std::string> port;
    GetCommandArgsWith(args, "ip", ip, outMessage);
    GetCommandArgsWith(args, "port", port, outMessage);

    if (g_theNetworkSubsystem->GetClientState() != ClientState::IDLE)
    {
        outMessage = Stringf("Fail to Connect, Client is already running or not initialized");
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        return false;
    }

    // 使用默认值或提供的值
    ip.second   = ip.second.empty() ? "127.0.0.1" : ip.second;
    port.second = port.second.empty() ? "3100" : port.second;

    if (g_theNetworkSubsystem)
    {
        bool result = g_theNetworkSubsystem->StartClient(ip.second, static_cast<uint16_t>(atoi(port.second.c_str())));

        if (result)
        {
            // 客户端连接成功 - 设置为多人客户端模式
            g_theGame->SetGameMode(EGameMode::MULTIPLAYER_CLIENT);
            g_theGame->m_localPlayerFactionId = 1; // 客户端通常是第二个玩家（阵营1）

            outMessage = Stringf("Connected to server successfully! IP=%s, Port=%d, Mode=MULTIPLAYER_CLIENT",
                                 ip.second.c_str(), atoi(port.second.c_str()));
            g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);

            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                                     "Game mode set to MULTIPLAYER_CLIENT. Ready for multiplayer chess!");
        }
        else
        {
            outMessage = Stringf("Failed to connect to server IP=%s, Port=%d",
                                 ip.second.c_str(), atoi(port.second.c_str()));
            g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        }

        return result;
    }

    return false;
}

/**
 * Handles the "ChessDisconnect" command to disconnect from the current network session.
 * When executed locally, it sends a disconnect message to remote and then disconnects.
 * When received remotely, it only disconnects without re-sending.
 *
 * @param args The arguments that may include "reason" to specify disconnection reason.
 * @return Returns true if disconnection was handled successfully, false otherwise.
 */
bool ChessMatchCommon::Command_ChessDisconnect(EventArgs& args)
{
    std::string                         outMessage;
    std::pair<std::string, std::string> reason;
    std::pair<std::string, std::string> remote;

    // 解析命令参数
    GetCommandArgsWith(args, "reason", reason, outMessage);
    GetCommandArgsWith(args, "remote", remote, outMessage);

    bool        isRemote         = !remote.second.empty() && IsTrueString(remote.second);
    std::string disconnectReason = reason.second.empty() ? "No reason given" : reason.second;

    // 检查网络连接状态
    bool isConnectedAsClient = (g_theNetworkSubsystem->GetClientState() == ClientState::CONNECTED);
    bool isRunningAsServer   = (g_theNetworkSubsystem->GetServerState() == ServerState::LISTENING);

    if (!isConnectedAsClient && !isRunningAsServer)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Not connected to disconnect from");
        return false;
    }

    if (isRemote)
    {
        // 远程命令：对方要求断开连接
        outMessage = Stringf("Remote disconnection request received. Reason: %s", disconnectReason.c_str());
        g_theDevConsole->AddLine(Rgba8::YELLOW, outMessage);

        // 直接断开连接
        if (isConnectedAsClient)
        {
            g_theNetworkSubsystem->DisconnectClient();
            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Client connection closed");
        }
        else if (isRunningAsServer)
        {
            g_theNetworkSubsystem->StopServer();
            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Server stopped");
        }

        // 重置GameMode
        g_theGame->SetGameMode(EGameMode::SINGLE_PLAYER);
        g_theGame->m_localPlayerFactionId = 0;
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Game mode reset to SINGLE_PLAYER");
    }
    else
    {
        // 本地命令：通过RemoteCmd先发送断开消息
        outMessage = Stringf("Disconnecting... Reason: %s", disconnectReason.c_str());
        g_theDevConsole->AddLine(Rgba8::YELLOW, outMessage);

        std::string remoteCommand;
        if (reason.second.empty())
        {
            remoteCommand = "ChessDisconnect";
        }
        else
        {
            if (reason.second.find(' ') != std::string::npos)
            {
                remoteCommand = Stringf("ChessDisconnect reason=\"%s\"", reason.second.c_str());
            }
            else
            {
                remoteCommand = Stringf("ChessDisconnect reason=%s", reason.second.c_str());
            }
        }

        SendRemoteCommand(remoteCommand);

        // 然后断开连接
        if (isConnectedAsClient)
        {
            g_theNetworkSubsystem->DisconnectClient();
            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Client disconnected");
        }
        else if (isRunningAsServer)
        {
            g_theNetworkSubsystem->StopServer();
            g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Server stopped");
        }

        // 重置GameMode
        g_theGame->SetGameMode(EGameMode::SINGLE_PLAYER);
        g_theGame->m_localPlayerFactionId = 0;
        g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG, "Game mode reset to SINGLE_PLAYER");
    }

    return true;
}

bool ChessMatchCommon::SendRemoteCommand(const std::string& command)
{
    if (!g_theNetworkSubsystem)
        return false;

    // 检查网络连接状态
    bool isConnectedAsClient = (g_theNetworkSubsystem->GetClientState() == ClientState::CONNECTED);
    bool isRunningAsServer   = (g_theNetworkSubsystem->GetServerState() == ServerState::LISTENING);

    if (!isConnectedAsClient && !isRunningAsServer)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Not connected - cannot send remote command");
        return false;
    }

    // 构造RemoteCmd命令字符串
    std::string remoteCmdString = Stringf("RemoteCmd cmd=%s", command.c_str());

    // 使用DevConsole执行RemoteCmd，这会调用Command_RemoteCmd函数
    g_theDevConsole->Execute(remoteCmdString);

    g_theDevConsole->AddLine(DevConsole::COLOR_INFO_LOG,
                             Stringf("Sent via RemoteCmd: %s", command.c_str()));

    return true;
}
