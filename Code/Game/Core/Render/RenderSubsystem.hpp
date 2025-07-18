#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Game/Core/PostProcess/PostProcessEffect.hpp"

struct RenderTarget;
struct FrameConstants;
struct LightingConstants;
class IRenderer;
class Camera;
class IRenderable;

class RenderSubsystem
{
public:
    RenderSubsystem(IRenderer& renderer);

    void Register(IRenderable* r); // Called when the component OnAttach
    void Unregister(IRenderable* r); // Called when the component OnDetach

    void RenderWorld(const Camera& camera, LightingConstants& lightConstants, FrameConstants& frameConstants);

    /// PostProcessing
    void               AddPostProcessEffect(std::unique_ptr<PostProcessEffect> effect);
    void               RemovePostProcessEffect(const std::string& effectName);
    PostProcessEffect* GetPostProcessEffect(const std::string& effectName);
    void               SetPostProcessEnabled(bool enabled) { m_postProcessEnabled = enabled; }
    bool               GetPostProcessEnabled() const { return m_postProcessEnabled; }


    void Startup();
    void Shutdown();

private:
    // Performing Scene Rendering (to Render Target)
    void RenderSceneToTarget(const Camera& camera, LightingConstants& lightConstants, FrameConstants& frameConstants, RenderTarget* target);
    void ProcessPostEffects(RenderTarget* sceneRT, RenderTarget* outputRT); // Execute a post-processing chain
    void SortPostProcessEffects(); // Sort post-processing effects
private:
    std::vector<std::unique_ptr<PostProcessEffect>> m_postProcessEffects;
    bool                                            m_postProcessEnabled = true;
    bool                                            m_postProcessDirty   = false; // It needs to be reordered

private:
    IRenderer&                m_renderer;
    std::vector<IRenderable*> m_renderables;

    /// Render targets
    RenderTarget* m_sceneRenderTarget = nullptr;
    RenderTarget* m_postProcessRT1    = nullptr; // Ping-pong buffer 1
    RenderTarget* m_postProcessRT2    = nullptr; // Ping-pong buffer 2
};
