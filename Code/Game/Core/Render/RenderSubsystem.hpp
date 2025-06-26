#pragma once
#include <vector>

struct FrameConstants;
struct LightingConstants;
class Renderer;
class Camera;
class IRenderable;

class RenderSubsystem
{
public:
    RenderSubsystem(Renderer& renderer);

    void Register(IRenderable* r); // Called when the component OnAttach
    void Unregister(IRenderable* r); // Called when the component OnDetach

    void RenderWorld(const Camera& camera, LightingConstants& lightConstants, FrameConstants& frameConstants);

    void Startup();
    void Shutdown();

private:
    Renderer&                 m_renderer;
    std::vector<IRenderable*> m_renderables;
};
