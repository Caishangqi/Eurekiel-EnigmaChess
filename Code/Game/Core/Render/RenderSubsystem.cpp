#include "RenderSubsystem.hpp"

#include "Renderable.hpp"
#include "RenderContext.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Core/LoggerSubsystem.hpp"
#include "Game/Core/Component/Component.hpp"

RenderSubsystem::RenderSubsystem(IRenderer& renderer): m_renderer(renderer)
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
        IComponent* comp = dynamic_cast<IComponent*>(r);
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
