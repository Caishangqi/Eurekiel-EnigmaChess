#pragma once
#include "PostProcessEffect.hpp"

class EffectBloom : public PostProcessEffect
{
public:
    EffectBloom(const std::string& name, int priority);

    void Initialize(IRenderer& renderer) override;
    void Shutdown() override;
    
    void Process(RenderTarget* input, RenderTarget* output) override;

protected:


protected:


private:
    
};
