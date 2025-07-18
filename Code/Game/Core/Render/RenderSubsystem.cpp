#include "RenderSubsystem.hpp"

#include <algorithm>

#include "Renderable.hpp"
#include "RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Component/Component.hpp"
#include "Game/Core/PostProcess/PostProcessEffect.hpp"

RenderSubsystem::RenderSubsystem(IRenderer& renderer) : m_renderer(renderer)
{
}

void RenderSubsystem::Register(IRenderable* r)
{
    m_renderables.push_back(r);
}

void RenderSubsystem::Unregister(IRenderable* r)
{
    for (int i = 0; i < static_cast<int>(m_renderables.size()); i++)
    {
        if (m_renderables[i] == r)
        {
            m_renderables[i] = nullptr;
        }
    }
    //m_renderables.erase(std::find(m_renderables.begin(), m_renderables.end(), r));
}

void RenderSubsystem::RenderWorld(const Camera& camera, LightingConstants& lightConstants, FrameConstants& frameConstants)
{
    m_renderer.BeginCamera(camera);
    RenderContext ctx{m_renderer, camera, lightConstants, frameConstants};

    for (IRenderable* r : m_renderables)
    {
        // whether or not is a component
        auto comp = dynamic_cast<IComponent*>(r);
        if (comp)
        {
            if (comp->GetEnable())
                r->Render(ctx); // component renderer
        }
        else if (r != nullptr)
        {
            r->Render(ctx); // Non-component renderer, like widget, particle etc.
        }
    }
    ctx.renderer.SetLightConstants(lightConstants);
    ctx.renderer.SetFrameConstants(frameConstants); // Not in mesh component because this is global
    m_renderer.BindShader(nullptr);
    m_renderer.BindTexture(nullptr);
    m_renderer.EndCamera(camera);
}

void RenderSubsystem::AddPostProcessEffect(std::unique_ptr<PostProcessEffect> effect)
{
    effect->Initialize(m_renderer);
    m_postProcessEffects.push_back(std::move(effect));
    m_postProcessDirty = true;
}

void RenderSubsystem::RemovePostProcessEffect(const std::string& effectName)
{
    auto it = std::find_if(m_postProcessEffects.begin(), m_postProcessEffects.end(),
                           [&effectName](const std::unique_ptr<PostProcessEffect>& effect)
                           {
                               return effect->GetName() == effectName;
                           });

    if (it != m_postProcessEffects.end())
    {
        (*it)->Shutdown();
        m_postProcessEffects.erase(it);
    }
}

PostProcessEffect* RenderSubsystem::GetPostProcessEffect(const std::string& effectName)
{
    auto it = std::find_if(m_postProcessEffects.begin(), m_postProcessEffects.end(),
                           [&effectName](const std::unique_ptr<PostProcessEffect>& effect)
                           {
                               return effect->GetName() == effectName;
                           });

    return (it != m_postProcessEffects.end()) ? it->get() : nullptr;
}


void RenderSubsystem::Startup()
{
    g_theLoggerSubsystem->EnableCategory(ELogCategory::LogRender);
    LOG(LogRender, Info, "Start up Render subsystem...");
}

void RenderSubsystem::Shutdown()
{
    for (IRenderable* r : m_renderables)
    {
        POINTER_SAFE_DELETE(r)
    }
}

/**
 * Renders the scene to a specified render target using the provided camera,
 * lighting constants, and frame constants. This function is responsible for
 * preparing and executing the rendering pipeline for the target.
 *
 * @param camera The camera object defining the viewpoint and perspective for rendering the scene.
 * @param lightConstants A reference to the lighting constants struct
 *        containing data such as sun direction, intensity, and other light information.
 * @param frameConstants A reference to the frame constants struct containing
 *        frame-specific data such as time and debugging parameters.
 * @param target A pointer to the render target where the scene will be rendered.
 */
void RenderSubsystem::RenderSceneToTarget(const Camera& camera, LightingConstants& lightConstants, FrameConstants& frameConstants, RenderTarget* target)
{
}

/**
 * Processes all active post-processing effects on a scene render target and writes the final result
 * to the specified output render target. Effects are applied in sequence, and intermediate results
 * are managed using a ping-pong mechanism between internal render targets.
 *
 * @param sceneRT A pointer to the render target containing the preprocessed scene to which
 *        post-processing effects will be applied.
 * @param outputRT A pointer to the output render target where the final processed scene
 *        will be rendered after all post-processing effects are applied.
 */
void RenderSubsystem::ProcessPostEffects(RenderTarget* sceneRT, RenderTarget* outputRT)
{
    // If needed, we first perform sorting
    if (m_postProcessDirty)
    {
        SortPostProcessEffects();
        m_postProcessDirty = false;
    }

    // Ping-pong render pipeline
    RenderTarget* currentInput  = sceneRT;
    RenderTarget* currentOutput = m_postProcessRT1;

    int enabledEffectCount = 0;
    for (std::unique_ptr<PostProcessEffect>& effect : m_postProcessEffects)
    {
        if (effect->GetEnable())
            enabledEffectCount++;
    }

    int processedCount = 0;
    for (std::unique_ptr<PostProcessEffect>& effect : m_postProcessEffects)
    {
        if (!effect->GetEnable())
            continue;
        processedCount++;

        // The last process directly sent to the output
        if (processedCount == enabledEffectCount)
            currentOutput = outputRT;

        // Process the effects
        effect->Process(currentInput, currentOutput);

        // Swap the input and output buffers RTV
        if (currentOutput != outputRT)
        {
            currentInput  = currentOutput;
            currentOutput = (currentOutput == m_postProcessRT1) ? m_postProcessRT2 : m_postProcessRT1;
        }
    }
}

/**
 * Sorts the post-processing effects in the internal list based on their
 * defined priority or order. This ensures that post-processing effects
 * are applied in the correct sequence during the rendering pipeline.
 *
 * This function should be called whenever the list of post-processing
 * effects changes, such as after adding or removing effects.
 */
void RenderSubsystem::SortPostProcessEffects()
{
    std::sort(m_postProcessEffects.begin(), m_postProcessEffects.end(),
              [](const std::unique_ptr<PostProcessEffect>& a, const std::unique_ptr<PostProcessEffect>& b)
              {
                  return a->GetPriority() < b->GetPriority();
              });
}
