#pragma once
#include "Engine/Renderer/Renderer.hpp"

struct Rgba8;
struct Mat44;
class Camera;


struct RenderContext
{
    IRenderer&         renderer; //Globally unique Renderer
    const Camera&     camera; // The Camera currently being rendered
    LightingConstants lightingConstants{};
    FrameConstants    frameConstants{};

    void SetModel(const Mat44& model, const Rgba8& tint = Rgba8::WHITE) const
    {
        renderer.SetModelConstants(model, tint);
    }
};
