#pragma once
#include "GameCommon.hpp"
#include "Core/Actor/Actor.hpp"
#include "Core/Render/RenderContext.hpp"
#include "Engine/Core/EventSystem.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Module/Gameplay/CameraState.h"
#include "Module/Gameplay/GameState.hpp"

class ChessMatch;
class Player;
class Clock;
class Prop;

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

    // Grid
    void RenderGrids() const;
    void RenderProps() const;

    /// Chess
    ChessMatch*  match       = nullptr;
    EGameState   gameState   = EGameState::ATTRACT;
    ECameraState cameraState = ECameraState::PER_PLAYER;
    ECameraMode  cameraMode  = ECameraMode::AUTO;

    bool m_isInMainMenu = true;
    bool m_isGameStart  = false;

    // Camera
    Camera* m_spectatorCamera = nullptr; // Default World Camera
    Camera* m_playerCamera    = nullptr; // Player Camera
    Camera* m_screenCamera    = nullptr;

    // Space for both world and screen, camera needs them
    AABB2 m_screenSpace;
    AABB2 m_worldSpace;

    /// Clock
    Clock* m_clock = nullptr;
    /// 

    /// Player
    Player* m_player = nullptr;
    /// 

    /// Cube
    Prop* m_cube   = nullptr;
    Prop* m_cube_1 = nullptr;
    ///

    /// Test Obj
    Prop* m_testProp = nullptr;
    /// 

    /// Balls
    Prop* m_ball = nullptr;
    /// 

    /// Light Constants
    LightingConstants m_lightingConstants = {Vec3(3, 1, -2), 0.55f, 0.35f};

    /// Grid
    Prop*              m_grid_x = nullptr;
    Prop*              m_grid_y = nullptr;
    std::vector<Prop*> m_grid_x_unit_5;
    std::vector<Prop*> m_grid_x_unit_1;
    std::vector<Prop*> m_grid_y_unit_5;
    std::vector<Prop*> m_grid_y_unit_1;
    /// 

    /// Configs
    XmlDocument m_chessMatchConfig;

    /// Debugs
    debug::ShaderDebugType m_shaderDebugType = debug::ShaderDebugType::Lit;
    debug::DebugViewMode   m_debugViewMode   = debug::DebugViewMode::Lighting;
    FrameConstants         m_frameConstants;

    /// Display Only
private:
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
