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


bool ChessMatchCommon::Command_ChessMove(EventArgs& args)
{
    Strings validSubcommand = {"from", "to", "promoteTo", "teleport"};

    std::string errorInvalidArgs = "Invalid args,the correct usage is > ChessMove from=<> to=<> promoteTo=<>";
    /// Game state Checking
    if (g_theGame->gameState != EGameState::MATCH)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "ChessMove can only used when in Match");
        return true;
    }

    std::string arg = args.GetValue("args", std::string(""));
    /// Syntax Checking
    if (arg == "")
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return true;
    }
    //  from=e2
    //  to=e4
    Strings fromTo = SplitStringOnDelimiter(arg, ' ');
    Strings addArgs;

    std::string outMessage;


    /// Find args with promotion
    std::pair<std::string, std::string> promotionArgPair;
    int                                 promotionArgResult = GetCommandArgsWith(args, "promoteTo", promotionArgPair, outMessage);
    if (promotionArgResult != -1)
    {
        ChessPieceDefinition* def = ChessPieceDefinition::GetByName(promotionArgPair.second);
        if (def && def->m_name != "Pawn" && def->m_name != "King")
        {
            addArgs.push_back(promotionArgPair.first + "=" + promotionArgPair.second);
        }
        else
        {
            g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Invalid Chess piece name promotion");
        }
    }


    if (fromTo.size() == 1 || fromTo.size() > 4)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return true;
    }
    //  from
    //  e2
    Strings from = SplitStringOnDelimiter(fromTo[0], '=');
    if (from.size() != 2)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return true;
    }

    if (from[0] != "from")
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("Invalid args, it should be [ from ] not [ %s ]", from[0].c_str()));
        return true;
    }
    //  to
    //  e4
    Strings to = SplitStringOnDelimiter(fromTo[1], '=');
    if (to.size() != 2)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, errorInvalidArgs);
        return true;
    }

    if (to[0] != "to")
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("Invalid args, it should be [ to ] not [ %s ]", to[0].c_str()));
        return true;
    }


    // From position valid ?
    if (!GetStringPositionValidation(from[1]))
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("Invalid Position, it should be [ A1 - H8 ] you enter [ %s ]", from[1].c_str()));
        return true;
    }

    if (!GetStringPositionValidation(to[1]))
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("Invalid Position, it should be [ A1 - H8 ] you enter [ %s ]", to[1].c_str()));
        return true;
    }
    if (!g_theGame->match)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, "Internal Error");
        return true;
    }

    /// Find args with teleport
    std::pair<std::string, std::string> teleportArgPair;
    int                                 teleportArgResult = GetCommandArgsWith(args, "teleport", teleportArgPair, outMessage);
    bool                                validTrueString   = IsTrueString(teleportArgPair.second);
    if (validTrueString && teleportArgResult != -1)
    {
        addArgs.push_back(teleportArgPair.first + "=" + teleportArgPair.second); // pack in the additional args
        g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "chess teleport is enable in this move (probably break chess rule data)!");
        g_theGame->match->ExecuteChessTeleport(GetGridPosition(from[1]), GetGridPosition(to[1]), from[1], to[1], addArgs);
        return true;
    }
    if (teleportArgResult != -1 && !validTrueString)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "If you want to enable Chess teleport, you should use teleport=true");
        return false;
    }

    g_theGame->match->ExecuteChessMove(GetGridPosition(from[1]), GetGridPosition(to[1]), from[1], to[1], addArgs);
    return true;
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

    if (port.second.empty())
    {
        bool result = g_theNetworkSubsystem->StartServer(g_theNetworkSubsystem->GetConfig().serverPort); // Default config
        if (result)
        {
            outMessage = Stringf("Start Server with ip = %s, port = %d", g_theNetworkSubsystem->GetConfig().serverIp.c_str(), g_theNetworkSubsystem->GetConfig().serverPort);
            g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);
            return result;
        }
        outMessage = Stringf("Fail to start Server with ip = %s, port = %d", g_theNetworkSubsystem->GetConfig().serverIp.c_str(), g_theNetworkSubsystem->GetConfig().serverPort);
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        return result;
    }
    return false;
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

    ip.second   = ip.second.empty() ? "127.0.0.1" : ip.second;
    port.second = port.second.empty() ? "3100" : port.second;

    if (g_theNetworkSubsystem)
    {
        bool result = g_theNetworkSubsystem->StartClient(ip.second, static_cast<uint16_t>(atoi(port.second.c_str())));
        if (result)
        {
            outMessage = Stringf("Connecting to Server with ip = %s, port = %d", ip.second.c_str(), atoi(port.second.c_str()));
            g_theDevConsole->AddLine(Rgba8::DEBUG_GREEN, outMessage);
            return result;
        }
        outMessage = Stringf("Fail to connect to Server with ip = %s, port = %d", ip.second.c_str(), atoi(port.second.c_str()));
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, outMessage);
        return result;
    }
    return false;
}

bool ChessMatchCommon::Command_ChessDisconnect(EventArgs& args)
{
    UNUSED(args)
    return false;
}
