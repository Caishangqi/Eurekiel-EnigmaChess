#include "PostProcessEffect.hpp"

#include "Engine/Renderer/IRenderer.hpp"

PostProcessEffect::PostProcessEffect(const std::string& name, int priority) : m_name(name), m_priority(priority), m_enabled(true)
{
}

/**
 * @brief Processes a post-processing effect on the given input render target and outputs the result to the specified output render target.
 *
 * This method should be implemented by subclasses to define the specific functionality of the post-processing effect.
 *
 * @param input The input render target on which the effect will be applied.
 * @param output The output render target where the processed result will be stored.
 */
void PostProcessEffect::Process(RenderTarget* input, RenderTarget* output)
{
}

/**
 * @brief Configures the renderer state for post-processing effects.
 *
 * This method sets the required state for 2D post-processing operations,
 * including disabling depth testing, configuring rasterization, setting the
 * sampling mode, and applying appropriate blending modes. It ensures that
 * the renderer is in the correct configuration to handle a full-screen
 * post-processing operation.
 *
 */
void PostProcessEffect::SetState()
{
    // Standard state configuration for post-processing
    m_renderer->SetDepthMode(DepthMode::DISABLED); // 2D processing does not require depth
    m_renderer->SetRasterizerMode(RasterizerMode::SOLID_CULL_NONE); // full screen quadrilateral
    m_renderer->SetSamplerMode(SamplerMode::BILINEAR_WRAP, 0); // Linear sampling
    m_renderer->SetBlendMode(BlendMode::OPAQUE); // Initial unmixed
};
