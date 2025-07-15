#pragma once
#include "GameCommon.hpp"
#include "Core/Actor/Actor.hpp"
#include "Core/Render/RenderContext.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Module/Gameplay/CameraState.h"
#include "Module/Gameplay/GameState.hpp"
#include "Module/Lib/ChessMatchCommon.hpp"


class NetworkDispatcher;
class ChessMatch;
class Player;
class Clock;

class Game
{
public:
    Game();
    ~Game();
    void Render() const;
    void Update();

    void UpdateMatch();

    // Event
    static bool Event_DebugCharInput(EventArgs& args);

    void HandleKeyBoardEvent(float deltaTime);
    void HandleMouseEvent(float deltaTime);

    // Game play logic
    void EnterState(EGameState state);
    void EnterAttractState();
    void EnterMatchState();
    void EnterSettlementState();
    void ChessMatchReset();

    // Camera
    void UpdateCameras(float deltaTime);
    void EnterCameraState(ECameraState state);

    // Networking
    void                        InitializeNetworking();
    ChessMatchCommon::EGameMode GetGameMode() const { return m_gameMode; }
    void                        SetGameMode(ChessMatchCommon::EGameMode mode) { m_gameMode = mode; }

    bool IsMultiplayerMode() const
    {
        return m_gameMode == ChessMatchCommon::EGameMode::MULTIPLAYER_HOST ||
            m_gameMode == ChessMatchCommon::EGameMode::MULTIPLAYER_CLIENT;
    }

    bool IsLocalPlayerHost() const
    {
        return m_gameMode == ChessMatchCommon::EGameMode::MULTIPLAYER_HOST;
    }

    bool IsLocalPlayerClient() const
    {
        return m_gameMode == ChessMatchCommon::EGameMode::MULTIPLAYER_CLIENT;
    }

    /// Chess
    ChessMatch*        match          = nullptr;
    NetworkDispatcher* m_dispatcher   = nullptr;
    EGameState         gameState      = EGameState::ATTRACT;
    ECameraState       cameraState    = ECameraState::PER_PLAYER;
    ECameraMode        cameraMode     = ECameraMode::AUTO;
    bool               m_isInMainMenu = true;
    bool               m_isGameStart  = false;
    // Camera
    Camera* m_spectatorCamera = nullptr; // Default World Camera
    Camera* m_playerCamera    = nullptr; // Player Camera
    Camera* m_screenCamera    = nullptr;
    // Space for both world and screen, camera needs them
    AABB2 m_screenSpace;
    AABB2 m_worldSpace;
    // Clock
    Clock* m_clock = nullptr;
    // Player
    Player* m_player = nullptr;
    // Light Constants
    LightingConstants m_lightingConstants    = {Vec3(3, 1, -2), 0.55f, 0.35f};
    int               m_localPlayerFactionId = 0;

    /// Configs
    XmlDocument m_chessMatchConfig;

    /// Debugs
    debug::ShaderDebugType m_shaderDebugType = debug::ShaderDebugType::Lit;
    debug::DebugViewMode   m_debugViewMode   = debug::DebugViewMode::Lighting;
    FrameConstants         m_frameConstants;

    /// Display Only
private:
    ChessMatchCommon::EGameMode m_gameMode = ChessMatchCommon::EGameMode::SINGLE_PLAYER;

#ifdef COSMIC
    float FluctuateValue(float value, float amplitude, float frequency, float deltaTime)
    {
        return value + amplitude * sinf(frequency * deltaTime);
    }

    float m_iconCircleRadius           = 200;
    float m_currentIconCircleThickness = 0.f;
    int   m_counter                    = 0;
#endif
};
