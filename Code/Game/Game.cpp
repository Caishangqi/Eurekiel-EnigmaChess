#include <Game/Game.hpp>

#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Input/XboxController.hpp"
#include "Engine/Input/InputSystem.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/DebugRenderSystem.h"
#include "Engine/Renderer/Renderer.hpp"

#include "Core/LoggerSubsystem.hpp"
#include "Core/Render/BakedModel.hpp"
#include "Module/Definition/ChessPieceDefinition.hpp"
#include "Module/Gameplay/ChessMatch.hpp"
#include "Module/Gameplay/ChessPlayer.hpp"

#include "App.hpp"
#include "GameCommon.hpp"
#include "Player.hpp"
#include "Prop.hpp"
#include "Core/WidgetSubsystem.hpp"
#include "Core/Render/RenderSubsystem.hpp"
#include "Module/Debug/WidgetDebugPanel.hpp"
#include "Module/Lib/DebugCommon.hpp"


Game::Game()
{
    g_theLoggerSubsystem->SetMinVerbosity(ELogVerbosity::Info);
    g_theLoggerSubsystem->EnableCategory(ELogCategory::LogGame);
    g_theLoggerSubsystem->EnableCategory(ELogCategory::LogResource);

    /// Event Callback
    g_theEventSystem->SubscribeEventCallbackFunction("CharInput", Event_DebugCharInput);

    /// Resource
    g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
    g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Caizii.png");
    BakedModel::RegisterModels();
    ChessPieceDefinition::LoadDefinitions("Data/Definitions/ChessPieceDefinition.xml");

    /// Config
    ISerializable::Create(m_chessMatchConfig, "Data/ChessMatchConfig.xml");

    /// Command Registration
    g_theDevConsole->RegisterCommand("ChessMove", "None", ChessMatchCommon::Command_ChessMove);
    g_theDevConsole->RegisterCommand("ChessMatch", "None", ChessMatchCommon::Command_ChessMatch);
    g_theDevConsole->RegisterCommand("Debug", "None", DebugCommon::Command_Debug);

    /// Rasterize
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);

    /// Spaces
    m_screenSpace.m_mins = Vec2::ZERO;
    m_screenSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("screenSizeX", 1600.f), g_gameConfigBlackboard.GetValue("screenSizeY", 800.f));

    m_worldSpace.m_mins = Vec2::ZERO;
    m_worldSpace.m_maxs = Vec2(g_gameConfigBlackboard.GetValue("worldSizeX", 200.f), g_gameConfigBlackboard.GetValue("worldSizeY", 100.f));

    /// Cameras
    m_screenCamera         = new Camera();
    m_screenCamera->m_mode = eMode_Orthographic;
    m_screenCamera->SetOrthographicView(Vec2::ZERO, m_screenSpace.m_maxs);
    m_spectatorCamera         = new Camera();
    m_spectatorCamera->m_mode = eMode_Perspective;
    m_spectatorCamera->SetOrthographicView(Vec2(-1, -1), Vec2(1, 1));
    ///

    /// Clock
    m_clock = new Clock(Clock::GetSystemClock());
    ///

    /// Player
    m_player             = new Player(this);
    m_player->m_position = Vec3(-2, 0, 1);
    /// 

    /// Cube
    m_cube   = new Prop(this);
    m_cube_1 = new Prop(this);

    AddVertsForCube3D(m_cube->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));
    AddVertsForCube3D(m_cube_1->m_vertexes, Rgba8(255, 0, 0), Rgba8(0, 255, 255), Rgba8(0, 255, 0), Rgba8(255, 0, 255), Rgba8(0, 0, 255), Rgba8(255, 255, 0));

    m_cube->m_position   = Vec3(2, 2, 0);
    m_cube_1->m_position = Vec3(-2, -2, 0);
    ///

    /// Test Prop
    m_testProp             = new Prop(this);
    m_testProp->m_position = Vec3(0, 0, 0);
    //AddVertsForCylinder3D(m_testProp->m_vertexes,Vec3(0,2,0),Vec3(0,0,0),1);
    //AddVertsForCone3D(m_testProp->m_vertexes,Vec3(0,2,0),Vec3(0,0,0),1);
    AddVertsForArrow3D(m_testProp->m_vertexes, Vec3(0, 2, 0), Vec3(0, 0, 0), 0.1f, 0.4f);
    /// 

    /// Ball
    m_ball             = new Prop(this);
    m_ball->m_position = Vec3(10, -5, 1);
    m_ball->m_texture  = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
    //m_ball->m_texture = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Caizii.png");
    AddVertsForSphere3D(m_ball->m_vertexes, Vec3(0, 0, 0), 2, Rgba8::WHITE, AABB2::ZERO_TO_ONE, 64, 32);
    /// 

    /// Grid
    m_grid_x = new Prop(this);
    AddVertsForCube3D(m_grid_x->m_vertexes, Rgba8::RED);
    m_grid_x->m_scale = Vec3(GRID_SIZE * 2.f, 0.1f, 0.1f);
    m_grid_y          = new Prop(this);

    AddVertsForCube3D(m_grid_y->m_vertexes, Rgba8::GREEN);
    m_grid_y->m_scale = Vec3(0.1f, GRID_SIZE * 2.f, 0.1f);

    m_grid_x_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_x_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_5[i]->m_vertexes, Rgba8(191, 0, 0));
        m_grid_x_unit_5[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.06f, 0.06f);
        m_grid_x_unit_5[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0);
    }

    m_grid_x_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_x_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_x_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_x_unit_1[i]->m_scale    = Vec3(GRID_SIZE * 2.f, 0.03f, 0.03f);
        m_grid_x_unit_1[i]->m_position = Vec3(0, GRID_SIZE * 1.f - static_cast<float>(i), 0);
    }

    m_grid_y_unit_5.resize(GRID_SIZE * 2 / GRID_UNIT_SIZE + 1);
    for (int i = 0; i < GRID_SIZE * 2 / GRID_UNIT_SIZE + 1; i++)
    {
        m_grid_y_unit_5[i] = new Prop(this);
        if (i == ((GRID_SIZE * 2 / GRID_UNIT_SIZE) / 2))
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_5[i]->m_vertexes, Rgba8(0, 191, 0));
        m_grid_y_unit_5[i]->m_scale    = Vec3(0.06f, GRID_SIZE * 2.f, 0.06f);
        m_grid_y_unit_5[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i) * 5.f, 0, 0);
    }

    m_grid_y_unit_1.resize(GRID_SIZE * 2 + 1);
    for (int i = 0; i < GRID_SIZE * 2 + 1; i++)
    {
        m_grid_y_unit_1[i] = new Prop(this);
        if ((i % GRID_UNIT_SIZE) == 0)
        {
            continue;
        }
        AddVertsForCube3D(m_grid_y_unit_1[i]->m_vertexes, Rgba8(127, 127, 127));
        m_grid_y_unit_1[i]->m_scale    = Vec3(0.03f, GRID_SIZE * 2.f, 0.03f);
        m_grid_y_unit_1[i]->m_position = Vec3(GRID_SIZE * 1.f - static_cast<float>(i), 0, 0);
    }
    ///

    /// Debug Drawing

    // Arrows
    DebugAddWorldArrow(Vec3(1, 0, 0), Vec3(0, 0, 0), 0.03f, -1, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 1, 0), Vec3(0, 0, 0), 0.03f, -1, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    DebugAddWorldArrow(Vec3(0, 0, 1), Vec3(0, 0, 0), 0.03f, -1, Rgba8::BLUE, Rgba8::GREEN, DebugRenderMode::USE_DEPTH);
    /// 

    // Text for y axis
    Mat44 transformY = Mat44::MakeTranslation3D(Vec3(0, 1.25f, 0.25f));
    transformY.AppendZRotation(180.f);
    DebugAddWorldText("y - left", transformY, 1.f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for x axis
    Mat44 transformX = Mat44::MakeTranslation3D(Vec3(1.6f, 0, 0.25f));
    transformX.AppendZRotation(90.f);
    DebugAddWorldText("x - forward", transformX, 1.f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);
    // Text for z axis
    Mat44 transformZ = Mat44::MakeTranslation3D(Vec3(0, -0.25f, .9f));
    transformZ.AppendXRotation(-90.f);
    transformZ.AppendZRotation(180.f);
    DebugAddWorldText("z - up", transformZ, 1.f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USE_DEPTH, Vec2(0.5, 0.5), -1);

    /// Game State
    g_theInput->SetCursorMode(CursorMode::POINTER);

    /// Frame Constant
    FrameConstants frameConstants{m_clock->GetTotalSeconds(), (int)m_shaderDebugType, 0.f, (int)m_debugViewMode};
    m_frameConstants = frameConstants;
    g_theRenderer->SetFrameConstants(frameConstants);

    /// Debug Pannel
    WidgetDebugPanel* debugPanel = new WidgetDebugPanel();
    g_theWidgetSubsystem->AddToViewport(debugPanel);
}

Game::~Game()
{
    POINTER_SAFE_DELETE(m_grid_x)
    POINTER_SAFE_DELETE(m_grid_y)
    for (Prop* grid_x_unit_5 : m_grid_x_unit_5)
    {
        POINTER_SAFE_DELETE(grid_x_unit_5)
    }
    for (Prop* grid_x_unit_1 : m_grid_x_unit_1)
    {
        POINTER_SAFE_DELETE(grid_x_unit_1)
    }
    for (Prop* grid_y_unit_5 : m_grid_y_unit_5)
    {
        POINTER_SAFE_DELETE(grid_y_unit_5)
    }
    for (Prop* grid_y_unit_1 : m_grid_y_unit_1)
    {
        POINTER_SAFE_DELETE(grid_y_unit_1)
    }
    POINTER_SAFE_DELETE(m_ball)
    POINTER_SAFE_DELETE(m_cube_1)
    POINTER_SAFE_DELETE(m_cube)
    POINTER_SAFE_DELETE(m_player)
    POINTER_SAFE_DELETE(m_screenCamera)
    POINTER_SAFE_DELETE(m_spectatorCamera)
    POINTER_SAFE_DELETE(m_spectatorCamera)
    BakedModel::ReleaseResources();
    ChessPieceDefinition::ReleaseResources();
    g_theRenderSubsystem->Shutdown();
}


void Game::Render() const
{
    g_theRenderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_BACK);
    g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
    g_theRenderer->SetDepthMode(DepthMode::READ_WRITE_LESS_EQUAL);


    if (gameState == EGameState::MATCH || gameState == EGameState::SETTLEMENT)
    {
        m_player->Render();
        /// Grid
        //RenderGrids();
        /// Props
        //RenderProps();
        DebugRenderWorld(*g_theGame->m_player->m_camera);
        DebugRenderScreen(*g_theGame->m_screenCamera);
    }

    //======================================================================= End of World Render =======================================================================
    // Second render screen camera
    g_theRenderer->BeginCamera(*m_screenCamera);
    /// Display Only
#ifdef COSMIC
    if (gameState == EGameState::ATTRACT)
    {
        g_theRenderer->ClearScreen(g_theApp->m_backgroundColor);
        g_theRenderer->BindTexture(nullptr);
        DebugDrawRing(Vec2(800, 400), m_currentIconCircleThickness, m_currentIconCircleThickness / 10, Rgba8::WHITE);
    }
#endif
    // UI render
    g_theRenderer->EndCamera(*m_screenCamera);
    //======================================================================= End of Screen Render =======================================================================
    /// 
}


void Game::UpdateCameras(float deltaTime)
{
    m_screenCamera->Update(deltaTime);
}

void Game::EnterCameraState(ECameraState state)
{
    cameraState = state;
}

void Game::RenderGrids() const
{
    m_grid_x->Render();
    m_grid_y->Render();
    for (Prop* grid_x_unit : m_grid_x_unit_5)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_x_unit : m_grid_x_unit_1)
    {
        grid_x_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_5)
    {
        grid_y_unit->Render();
    }
    for (Prop* grid_y_unit : m_grid_y_unit_1)
    {
        grid_y_unit->Render();
    }
}

void Game::RenderProps() const
{
    g_theRenderer->BindTexture(nullptr);
    m_cube->Render();
    m_cube_1->Render();
    m_ball->Render();
    //m_testProp->Render();
}


void Game::Update()
{
    if (gameState == EGameState::ATTRACT)
    {
        g_theInput->SetCursorMode(CursorMode::POINTER);
    }
    UpdateMatch();

    /// Player
    m_player->Update(Clock::GetSystemClock().GetDeltaSeconds());
    ///

    /// Cube
    float brightnessFactor = CycleValue(m_clock->GetTotalSeconds(), 1.f);
    auto  color            = Rgba8(
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        static_cast<unsigned char>(brightnessFactor * 255),
        255);
    m_cube_1->m_color = color;
    m_cube->m_orientation.m_rollDegrees += m_clock->GetDeltaSeconds() * 30;
    m_cube->m_orientation.m_pitchDegrees += m_clock->GetDeltaSeconds() * 30;
    /// 

    /// Sphere
    m_ball->m_orientation.m_yawDegrees += m_clock->GetDeltaSeconds() * 45;
    /// 

    g_theWidgetSubsystem->Update();

    /// Debug Only
    std::string debugGameState = Stringf("Time: %.2f FPS: %.1f Scale: %.2f",
                                         m_clock->GetTotalSeconds(),
                                         m_clock->GetFrameRate(),
                                         m_clock->GetTimeScale()
    );
    DebugAddScreenText(debugGameState, m_screenSpace, 14, 0);
    DebugAddMessage(Stringf("Camera position: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z), 0);
    DebugAddMessage(Stringf("Camera orientation: %.2f, %.2f, %.2f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), 0);
    DebugAddMessage(Stringf("CameraMode (F4): %s | GameState: %s", to_string(cameraMode), to_string(gameState)), 0, Rgba8::ORANGE, Rgba8::ORANGE);
    /// Display Only
#ifdef COSMIC
    m_counter++;
    m_currentIconCircleThickness = FluctuateValue(m_iconCircleRadius, 50.f, 0.02f, static_cast<float>(m_counter));
#endif
    float deltaTime = m_clock->GetDeltaSeconds();
    UpdateCameras(deltaTime);

    HandleMouseEvent(deltaTime);
    HandleKeyBoardEvent(deltaTime);
}

void Game::UpdateMatch()
{
    if (gameState == EGameState::MATCH || gameState == EGameState::SETTLEMENT)
        match->Update();
}

bool Game::Event_DebugCharInput(EventArgs& args)
{
    if (g_theDevConsole && g_theDevConsole->IsOpen())
    {
        return false;
    }
    char inputChar = '0';
    inputChar      = (char)args.GetValue("KeyCode", inputChar);
    int numEnum    = inputChar - '0';
    if (numEnum < 0 || numEnum > 10)
    {
        numEnum = 0;
        return false;
    }
    Game* game              = g_theGame;
    game->m_shaderDebugType = static_cast<debug::ShaderDebugType>(numEnum);
    FrameConstants frameConstants{game->m_clock->GetTotalSeconds(), (int)game->m_shaderDebugType, 0.f, (int)game->m_debugViewMode};
    game->m_frameConstants = frameConstants;
    g_theRenderer->SetFrameConstants(frameConstants);
    return false;
}


void Game::HandleKeyBoardEvent(float deltaTime)
{
    UNUSED(deltaTime)
    const XboxController& controller = g_theInput->GetController(0);
    if (gameState == EGameState::ATTRACT)
    {
        bool spaceBarPressed = g_theInput->WasKeyJustPressed(32);
        bool NKeyPressed     = g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(XBOX_BUTTON_A) || controller.WasButtonJustPressed(XBOX_BUTTON_START);
        if (spaceBarPressed || NKeyPressed)
        {
            EnterState(EGameState::MATCH);
        }
    }

    if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XBOX_BUTTON_BACK))
    {
        if (gameState == EGameState::MATCH)
        {
            EnterState(EGameState::ATTRACT);
        }
        else
        {
            g_theEventSystem->FireEvent("WindowCloseEvent");
        }
    }

    if (g_theInput->WasKeyJustPressed('H') || controller.WasButtonJustPressed(XBOX_BUTTON_START))
    {
        if (m_isGameStart)
        {
            m_player->m_position    = Vec3(-2, 0, 1);
            m_player->m_orientation = EulerAngles();
        }
    }

    if (gameState != EGameState::ATTRACT && gameState != EGameState::LOBBY && gameState != EGameState::LOAD_RESOURCE)
    {
        // 1
        /*if (g_theInput->WasKeyJustPressed(0x31))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            DebugAddWorldCylinder(m_player->m_position, m_player->m_position + forward * 20, 0.0625f, 10.f, Rgba8::YELLOW, Rgba8::YELLOW, DebugRenderMode::X_RAY);
        }
        if (g_theInput->IsKeyDown(0x32))
        {
            DebugAddWorldSphere(Vec3(m_player->m_position.x, m_player->m_position.y, 0.f), 0.25f, 10.f, Rgba8(150, 75, 0), Rgba8(150, 75, 0));
        }
        // 3
        if (g_theInput->WasKeyJustPressed(0x33))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            // Push the wire ball 2 unit forward away
            DebugAddWorldWireSphere(m_player->m_position + forward * 2, 1, 5.f, Rgba8::GREEN, Rgba8::RED);
        }
        // 4
        if (g_theInput->WasKeyJustPressed(0x34))
        {
            Mat44 transform = Mat44::MakeTranslation3D(m_player->m_position);
            transform.Append(m_player->m_orientation.GetAsMatrix_IFwd_JLeft_KUp());
            DebugAddWorldBasis(transform, 20.f);
        }

        // 5
        if (g_theInput->WasKeyJustPressed(0x35))
        {
            Vec3 forward, left, up;
            m_player->m_orientation.GetAsVectors_IFwd_JLeft_KUp(forward, left, up);
            DebugAddWorldBillboardText(
                Stringf("Position: %.2f, %.2f, %.2f Orientation: %.2f, %.2f, %.2f", m_player->m_position.x, m_player->m_position.y, m_player->m_position.z, m_player->m_orientation.m_yawDegrees,
                        m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), m_player->m_position + forward * 2, 0.125f, Rgba8::WHITE, Rgba8::RED,
                DebugRenderMode::USE_DEPTH,
                Vec2(0.5, 0.5), 10.f);
        }

        // 6
        if (g_theInput->WasKeyJustPressed(0x36))
        {
            DebugAddWorldCylinder(m_player->m_position + Vec3(0, 0, 1), m_player->m_position, 0.5f, 10, Rgba8::WHITE, Rgba8::RED);
        }

        // 7
        if (g_theInput->WasKeyJustPressed(0x37))
        {
            DebugAddMessage(Stringf("Camera orientation: %.2f, %.2f, %.2f", m_player->m_orientation.m_yawDegrees, m_player->m_orientation.m_pitchDegrees, m_player->m_orientation.m_rollDegrees), 5);
        }*/
    }
}

void Game::HandleMouseEvent(float deltaTime)
{
    UNUSED(deltaTime)
}

void Game::EnterState(EGameState state)
{
    LOG(LogGame, Info, Stringf("Entering State %s",to_string(state)).c_str());
    switch (state)
    {
    case EGameState::NONE:
        break;
    case EGameState::LOAD_RESOURCE:
        break;
    case EGameState::ATTRACT:
        EnterAttractState();
        break;
    case EGameState::LOBBY:
        break;
    case EGameState::MATCH:
        EnterMatchState();
        break;
    case EGameState::SETTLEMENT:
        EnterSettlementState();
        break;
    }
}

void Game::EnterAttractState()
{
    POINTER_SAFE_DELETE(match)
    g_theInput->SetCursorMode(CursorMode::POINTER);
    gameState = EGameState::ATTRACT;
}

void Game::EnterMatchState()
{
    gameState = EGameState::MATCH;
    match     = new ChessMatch(this);
}

void Game::EnterSettlementState()
{
    gameState = EGameState::SETTLEMENT;
    EnterCameraState(ECameraState::CONFIGURED);
    ChessMatchCommon::GetCameraTransform(g_theGame->cameraState, g_theGame->m_player->m_position, g_theGame->m_player->m_orientation, match, "above");
    g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("[ %s ] win the game", match->GetCurrentTurnPlayer()->m_faction.m_displayName.c_str()));
    g_theDevConsole->AddLine(DevConsole::COLOR_WARNING, Stringf("Enter ChessMatch reset to reset the match", match->GetCurrentTurnPlayer()->m_faction.m_displayName.c_str()));
}

void Game::ChessMatchReset()
{
    delete match;
    match = nullptr;
    EnterState(EGameState::MATCH);
}
