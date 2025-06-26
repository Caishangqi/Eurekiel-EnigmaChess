#include "DebugCommon.hpp"

#include "ChessMatchCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"

bool DebugCommon::Command_Debug(EventArgs& args)
{
    if (!g_theGame)
    {
        g_theDevConsole->AddLine(DevConsole::COLOR_ERROR, "Debug can only used when in the Game");
        return false;
    }
    Game*                               game = g_theGame;
    std::string                         outMessage;
    std::pair<std::string, std::string> pair;
    int                                 result = ChessMatchCommon::GetCommandArgsWith(args, "viewMode", pair, outMessage);
    if (result == 0)
    {
        if (!pair.second.empty())
        {
            if (Common::ToUpper(pair.second) == "DEFAULT")
            {
                g_theGame->m_debugViewMode = debug::DebugViewMode::Default;
                g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "Set Debug view mode to: " + pair.second);
            }
            else if (Common::ToUpper(pair.second) == "LIT")
            {
                g_theGame->m_debugViewMode = debug::DebugViewMode::Lit;
                g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "Set Debug view mode to: " + pair.second);
            }
            else if (Common::ToUpper(pair.second) == "UNLIT")
            {
                g_theGame->m_debugViewMode = debug::DebugViewMode::Unlit;
                g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "Set Debug view mode to: " + pair.second);
            }
            else if (Common::ToUpper(pair.second) == "WIREFRAME")
            {
                g_theGame->m_debugViewMode = debug::DebugViewMode::Wireframe;
                g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "Set Debug view mode to: " + pair.second);
            }
            else if (Common::ToUpper(pair.second) == "LIGHTING")
            {
                g_theGame->m_debugViewMode = debug::DebugViewMode::Lighting;
                g_theDevConsole->AddLine(DevConsole::COLOR_INPUT_NORMAL, "Set Debug view mode to: " + pair.second);
            }
            else
            {
                g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Invalid Debug view" + pair.second);
            }
            FrameConstants frameConstants{game->m_clock->GetTotalSeconds(), (int)game->m_shaderDebugType, 0.f, (int)game->m_debugViewMode};
            game->m_frameConstants = frameConstants;
            g_theRenderer->SetFrameConstants(frameConstants);
            return true;
        }
    }
    g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, "Invalid Debug Subcommand");
    return false;
}
