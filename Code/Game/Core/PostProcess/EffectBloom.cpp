#include "EffectBloom.hpp"

#include "Engine/Renderer/IRenderer.hpp"

EffectBloom::EffectBloom(const std::string& name, int priority) : PostProcessEffect(name, priority)
{
}

void EffectBloom::Initialize(IRenderer& renderer)
{
}

void EffectBloom::Shutdown()
{
}

void EffectBloom::Process(RenderTarget* input, RenderTarget* output)
{
}

