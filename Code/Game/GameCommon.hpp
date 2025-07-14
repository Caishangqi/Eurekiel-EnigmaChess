#pragma once
#include <string>
#include <vector>

#include "Engine/Core/EventSystem.hpp"

/// Whether or not enable cosmic circle (developer)
#define COSMIC


struct Vertex_PCU;
struct Rgba8;
struct Vec2;
class Camera;
class App;
class RandomNumberGenerator;
class IRenderer;
class InputSystem;
class AudioSystem;
class Game;
class RenderSubsystem;
class LoggerSubsystem;
class WidgetSubsystem;
class NetworkSubsystem;

extern RandomNumberGenerator* g_rng;
extern App*                   g_theApp;
extern IRenderer*             g_theRenderer;
extern InputSystem*           g_theInput;
extern AudioSystem*           g_theAudio;
extern Game*                  g_theGame;
extern RenderSubsystem*       g_theRenderSubsystem;
extern LoggerSubsystem*       g_theLoggerSubsystem;
extern WidgetSubsystem*       g_theWidgetSubsystem;
extern NetworkSubsystem*      g_theNetworkSubsystem;

constexpr float WORLD_SIZE_X   = 200.f;
constexpr float WORLD_SIZE_Y   = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
// Math
constexpr float PI = 3.14159265359f;
// Entity Data
constexpr int MAX_ENTITY_PER_TYPE = 64;

/// Grid
constexpr int GRID_SIZE      = 50; // Half
constexpr int GRID_UNIT_SIZE = 5;
/// 


void DebugDrawRing(const Vec2& center, float radius, float thickness, const Rgba8& color);
void DebugDrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba8& color);
void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& color);
void AddVertsForCube3D(std::vector<Vertex_PCU>& verts, const Rgba8& colorX, const Rgba8& colorNX, const Rgba8& colorY, const Rgba8& colorNY, const Rgba8& colorZ, const Rgba8& colorNZ);

namespace debug
{
    enum class DebugViewMode
    {
        Default = 0,
        Lit = 1,
        Unlit = 2,
        Wireframe = 3,
        Lighting = 4
    };

    inline const char* to_string(DebugViewMode e)
    {
        switch (e)
        {
        case DebugViewMode::Default: return "Default";
        case DebugViewMode::Lit: return "Lit";
        case DebugViewMode::Unlit: return "Unlit";
        case DebugViewMode::Wireframe: return "Wireframe";
        case DebugViewMode::Lighting: return "Lighting";
        default: return "unknown";
        }
    }

    enum class ShaderDebugType
    {
        Lit = 0,
        DiffuseColor = 1,
        SurfaceColor,
        UVCoords,
        SurfaceTangentModelSpace,
        SurfaceBitangentModelSpace,
        SurfaceNormalModelSpace,
        NormalColor,
        PixelNormalTBNSpace,
        PixelNormalWorldSpace,
        Lighting,
        SurfaceTangentWorldSpace,
        SurfaceBitangentWorldSpace,
        SurfaceNormalWorldSpace,
        ModelIBasisWorld,
        ModelJBasisWorld,
        ModelKBasisWorld
    };

    inline const char* to_string(ShaderDebugType e)
    {
        switch (e)
        {
        case ShaderDebugType::Lit: return "Lit";
        case ShaderDebugType::DiffuseColor: return "DiffuseColor";
        case ShaderDebugType::SurfaceColor: return "SurfaceColor";
        case ShaderDebugType::UVCoords: return "UVCoords";
        case ShaderDebugType::SurfaceTangentModelSpace: return "SurfaceTangentModelSpace";
        case ShaderDebugType::SurfaceBitangentModelSpace: return "SurfaceBitangentModelSpace";
        case ShaderDebugType::SurfaceNormalModelSpace: return "SurfaceNormalModelSpace";
        case ShaderDebugType::NormalColor: return "NormalColor";
        case ShaderDebugType::PixelNormalTBNSpace: return "PixelNormalTBNSpace";
        case ShaderDebugType::PixelNormalWorldSpace: return "PixelNormalWorldSpace";
        case ShaderDebugType::Lighting: return "Lighting";
        case ShaderDebugType::SurfaceTangentWorldSpace: return "SurfaceTangentWorldSpace";
        case ShaderDebugType::SurfaceBitangentWorldSpace: return "SurfaceBitangentWorldSpace";
        case ShaderDebugType::SurfaceNormalWorldSpace: return "SurfaceNormalWorldSpace";
        case ShaderDebugType::ModelIBasisWorld: return "ModelIBasisWorld";
        case ShaderDebugType::ModelJBasisWorld: return "ModelJBasisWorld";
        case ShaderDebugType::ModelKBasisWorld: return "ModelKBasisWorld";
        default: return "unknown";
        }
    }
}

namespace Common
{
    [[nodiscard]]
    std::string ToLower(const std::string& input);
    [[nodiscard]]
    std::string ToUpper(const std::string& input);
}
