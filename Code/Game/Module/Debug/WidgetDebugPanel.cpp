#include "WidgetDebugPanel.hpp"

#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Math/LineSegment2.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Game/Game.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Module/Definition/ChessPieceDefinition.hpp"
#include "Game/Module/Gameplay/ChessMatch.hpp"
#include "Game/Module/Gameplay/ChessPiece.hpp"
#include "Game/Module/Gameplay/ChessPlayer.hpp"

WidgetDebugPanel::WidgetDebugPanel()
{
    m_name = "WidgetDebugPanel";

    m_cursorVertex.reserve(1024);
}

void WidgetDebugPanel::OnInit()
{
    Widget::OnInit();
}

void WidgetDebugPanel::Draw() const
{
    Widget::Draw();
    g_theRenderer->DrawVertexArray(m_cursorVertex);
}

void WidgetDebugPanel::Update()
{
    Widget::Update();

    if (g_theGame->match)
    {
        m_cursorVertex.clear();
        ChessMatch* match       = g_theGame->match;
        Rgba8       cursorColor = match->GetCurrentTurnPlayer()->m_faction.m_color;

        AABB2 horizontal;
        horizontal.SetDimensions(Vec2(50, 3));
        horizontal.SetCenter(Vec2(g_theGame->m_screenSpace.GetDimensions().x / 2.0f, g_theGame->m_screenSpace.GetDimensions().y / 2.0f));

        AABB2 vertical;
        vertical.SetDimensions(Vec2(3, 50));
        vertical.SetCenter(Vec2(g_theGame->m_screenSpace.GetDimensions().x / 2.0f, g_theGame->m_screenSpace.GetDimensions().y / 2.0f));

        AddVertsForAABB2D(m_cursorVertex, horizontal, cursorColor);
        AddVertsForAABB2D(m_cursorVertex, vertical, cursorColor);
    }


    /// Pannel
    AABB2 panel;
    panel.m_mins = Vec2(20, 500);
    panel.m_maxs = Vec2(800, 540);
    DebugAddScreenText("[ Control Panel ]", panel, 16.f, 0.f, Rgba8::WHITE, Rgba8::WHITE, Vec2(0.f, 1.0f));
    panel.m_mins.y -= 30.f;
    panel.m_maxs.y -= 30.f;
    DebugAddScreenText(">> Debug View Modes", panel, 10.f, 0.f, Rgba8::WHITE, Rgba8::WHITE, Vec2(0.f, 1.0f));
    panel.m_mins.y -= 20.f;
    panel.m_maxs.y -= 20.f;
    DebugAddScreenText(Stringf(" > Render Mode: (%d) %s", g_theGame->m_shaderDebugType, to_string(g_theGame->m_shaderDebugType)), panel, 10.f, 0.f, Rgba8::DEBUG_GREEN, Rgba8::DEBUG_GREEN,
                       Vec2(0.f, 1.0f));
    panel.m_mins.y -= 20.f;
    panel.m_maxs.y -= 20.f;
    DebugAddScreenText(Stringf(" > View Mode: (%d) %s", g_theGame->m_debugViewMode, to_string(g_theGame->m_debugViewMode)), panel, 10.f, 0.f, Rgba8::DEBUG_GREEN, Rgba8::DEBUG_GREEN,
                       Vec2(0.f, 1.0f));
    panel.m_mins.y -= 20.f;
    panel.m_maxs.y -= 20.f;
    DebugAddScreenText(Stringf(">> Match Control Scheme"), panel, 10.f, 0.f, Rgba8::WHITE, Rgba8::WHITE, Vec2(0.f, 1.0f));
    panel.m_mins.y -= 20.f;
    panel.m_maxs.y -= 20.f;
    if (g_theGame->match)
    {
        if (g_theGame->match->GetCurrentTurnPlayer()->GetEnableTeleportCheat())
        {
            DebugAddScreenText(Stringf(" > LCTRL / RCTRL Toggle Teleport Cheat", g_theGame->match->m_impactSquare.x, g_theGame->match->m_impactSquare.y), panel, 10.f, 0.f, Rgba8::DEBUG_GREEN,
                               Rgba8::DEBUG_GREEN,
                               Vec2(0.f, 1.0f));
        }
        else
        {
            DebugAddScreenText(Stringf(" > LCTRL / RCTRL Toggle Teleport Cheat", g_theGame->match->m_impactSquare.x, g_theGame->match->m_impactSquare.y), panel, 10.f, 0.f, Rgba8::GRAY,
                               Rgba8::GRAY,
                               Vec2(0.f, 1.0f));
        }
        panel.m_mins.y -= 20.f;
        panel.m_maxs.y -= 20.f;
    }


    DebugAddScreenText(Stringf(">> Debug Piece Selection"), panel, 10.f, 0.f, Rgba8::WHITE, Rgba8::WHITE, Vec2(0.f, 1.0f));
    panel.m_mins.y -= 20.f;
    panel.m_maxs.y -= 20.f;
    if (g_theGame->match)
    {
        if (g_theGame->match->m_impactSquare == IntVec2::INVALID)
            DebugAddScreenText(Stringf(" > Impact Square: INVALID"), panel, 10.f, 0.f, Rgba8::ORANGE, Rgba8::ORANGE,
                               Vec2(0.f, 1.0f));
        else
        {
            DebugAddScreenText(Stringf(" > Impact Square: (%d, %d)", g_theGame->match->m_impactSquare.x, g_theGame->match->m_impactSquare.y), panel, 10.f, 0.f, Rgba8::DEBUG_GREEN, Rgba8::DEBUG_GREEN,
                               Vec2(0.f, 1.0f));
        }
        panel.m_mins.y -= 20.f;
        panel.m_maxs.y -= 20.f;
        if (g_theGame->match->m_highLightedSquare == IntVec2::INVALID)
            DebugAddScreenText(Stringf(" > Highlighted Square: INVALID"), panel, 10.f, 0.f, Rgba8::ORANGE, Rgba8::ORANGE,
                               Vec2(0.f, 1.0f));
        else
        {
            DebugAddScreenText(Stringf(" > Highlighted Square: (%d, %d)", g_theGame->match->m_highLightedSquare.x, g_theGame->match->m_highLightedSquare.y), panel, 10.f, 0.f, Rgba8::DEBUG_GREEN,
                               Rgba8::DEBUG_GREEN,
                               Vec2(0.f, 1.0f));
        }
        panel.m_mins.y -= 20.f;
        panel.m_maxs.y -= 20.f;
        if (g_theGame->match->m_selectedPiece == nullptr)
        {
            DebugAddScreenText(Stringf(" > Selected Pieces: INVALID"), panel, 10.f, 0.f, Rgba8::ORANGE, Rgba8::ORANGE,
                               Vec2(0.f, 1.0f));
        }
        else
        {
            DebugAddScreenText(Stringf(" > Selected Pieces: %s", g_theGame->match->m_selectedPiece->GetDefinition()->m_name.c_str()), panel, 10.f, 0.f, Rgba8::ORANGE, Rgba8::ORANGE,
                               Vec2(0.f, 1.0f));
        }
    }
}
