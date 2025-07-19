#pragma once
#include <vector>

#include "PostProcessEffect.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec2.hpp"

class VertexBuffer;
class ConstantBuffer;
class Shader;

/// Contants
struct BloomConstants
{
    float threshold = 1.0f; // Brightness threshold
    float intensity = 1.0f; // Bloom intensity
    float blurSigma = 2.0f; // Blur intensity
    float padding   = 0.0f; // Aligned to 16 bytes
};

struct BlurConstants
{
    Vec2  texelSize; // Texture Size
    float blurDirection; // 0=horizontal, 1=vertical
    float padding;
};


class EffectBloom : public PostProcessEffect
{
public:
    EffectBloom(const std::string& name, int priority);
    ~EffectBloom() override;

    void Initialize(IRenderer& renderer) override;
    void Shutdown() override;

    void Process(RenderTarget* input, RenderTarget* output) override;

    /// Parameter Setting
    void SetThreshold(float threshold);
    void SetIntensity(float intensity);
    void SetBlurSigma(float sigma);

    float GetThreshold() const { return m_bloomConstants.threshold; }
    float GetIntensity() const { return m_bloomConstants.intensity; }
    float GetBlurSigma() const { return m_bloomConstants.blurSigma; }

    // Sets the number of blur levels
    void SetBloomLevels(int levels);
    int  GetBloomLevels() const { return m_bloomLevels; }

protected:
    // Create a resource
    void CreateShaders(IRenderer& renderer);
    void CreateBuffers(IRenderer& renderer);
    void CreateRenderTargets(IRenderer& renderer);

    // Rendering steps
    void ExtractBrightness(IRenderer& renderer, RenderTarget* source, RenderTarget* dest);
    void DownsampleBlur(IRenderer& renderer, int level);
    void UpsampleCombine(IRenderer& renderer, int level);
    void FinalComposite(IRenderer& renderer, RenderTarget* scene, RenderTarget* bloom, RenderTarget* output);

    // Helper functions
    void DrawFullscreenQuad(IRenderer& renderer);
    void UpdateConstants();
    void SetBlurDirection(IRenderer& renderer, bool horizontal);

private:
    /// Shaders
    Shader* m_brightnessExtractShader = nullptr;
    Shader* m_downsampleShader        = nullptr;
    Shader* m_gaussianBlurShader      = nullptr;
    Shader* m_upsampleShader          = nullptr;
    Shader* m_compositeShader         = nullptr;

    // Vertex buffers
    VertexBuffer* m_fullscreenQuadVB = nullptr;

    /// Constant and Buffer
    ConstantBuffer* m_bloomConstantsCB = nullptr;
    ConstantBuffer* m_blurConstantsCB  = nullptr;
    BloomConstants  m_bloomConstants;
    BlurConstants   m_blurConstants;

    // Multi-level blurring
    static constexpr int MAX_BLOOM_LEVELS = 6;
    int                  m_bloomLevels    = 5;

    struct BloomLevel
    {
        RenderTarget* downsample = nullptr; // Downsampling results
        RenderTarget* blurTemp   = nullptr; // Blur temporary buffering
        IntVec2       size;
    };

    std::vector<BloomLevel> m_bloomMipChain;
    RenderTarget*           m_brightnessRT = nullptr; // Brightness extraction results
};
