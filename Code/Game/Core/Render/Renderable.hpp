#pragma once

struct RenderContext;

class IRenderable
{
public:
    virtual      ~IRenderable() = default;
    virtual void Render(const RenderContext& ctx) = 0;
};
