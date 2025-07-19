#include "EffectBloom.hpp"

#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Engine/Renderer/IRenderer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Window/Window.hpp"
#include "Game/App.hpp"

EffectBloom::EffectBloom(const std::string& name, int priority) : PostProcessEffect("Bloom", 0)
{
}

EffectBloom::~EffectBloom()
{
    EffectBloom::Shutdown();
}

void EffectBloom::Initialize(IRenderer& renderer)
{
    m_renderer = &renderer;

    CreateShaders(renderer);
    CreateBuffers(renderer);
    CreateRenderTargets(renderer);

    UpdateConstants();
}

void EffectBloom::Shutdown()
{
    POINTER_SAFE_DELETE(m_brightnessRT)
    for (BloomLevel& bloom_level : m_bloomMipChain)
    {
        POINTER_SAFE_DELETE(bloom_level.downsample)
        POINTER_SAFE_DELETE(bloom_level.blurTemp)
    }
    m_bloomMipChain.clear();

    POINTER_SAFE_DELETE(m_fullscreenQuadVB)
    POINTER_SAFE_DELETE(m_bloomConstantsCB)
    POINTER_SAFE_DELETE(m_blurConstantsCB)
}

void EffectBloom::Process(RenderTarget* input, RenderTarget* output)
{
}

void EffectBloom::SetThreshold(float threshold)
{
    m_bloomConstants.threshold = threshold;
    UpdateConstants();
}

void EffectBloom::SetIntensity(float intensity)
{
    m_bloomConstants.intensity = intensity;
    UpdateConstants();
}

void EffectBloom::SetBlurSigma(float sigma)
{
    m_bloomConstants.blurSigma = sigma;
    UpdateConstants();
}

void EffectBloom::SetBloomLevels(int levels)
{
    m_bloomLevels = (int)GetClamped((float)levels, 1, MAX_BLOOM_LEVELS);

    // If it has already been initialized, need to recreate the render target
    if (m_renderer && !m_bloomMipChain.empty())
    {
        // Clean up the old ones
        for (auto& level : m_bloomMipChain)
        {
            delete level.downsample;
            delete level.blurTemp;
        }
        m_bloomMipChain.clear();

        // Create a new one
        CreateRenderTargets(*m_renderer);
    }
}

void EffectBloom::CreateShaders(IRenderer& renderer)
{
    // Compile different entry points in Bloom.hlsl
    m_brightnessExtractShader = renderer.CreateShader("BrightnessExtract",
                                                      "Data/Shaders/Bloom.hlsl", "PostProcessVS", "BrightnessExtractPS");

    m_downsampleShader = renderer.CreateShader("Downsample",
                                               "Data/Shaders/Bloom.hlsl", "PostProcessVS", "DownsamplePS");

    m_gaussianBlurShader = renderer.CreateShader("GaussianBlur",
                                                 "Data/Shaders/Bloom.hlsl", "PostProcessVS", "GaussianBlurPS");

    m_upsampleShader = renderer.CreateShader("Upsample",
                                             "Data/Shaders/Bloom.hlsl", "PostProcessVS", "UpsamplePS");

    m_compositeShader = renderer.CreateShader("Composite",
                                              "Data/Shaders/Bloom.hlsl", "PostProcessVS", "CompositePS");
}

void EffectBloom::CreateBuffers(IRenderer& renderer)
{
    // Create a full-screen quadrilateral
    std::vector<Vertex_PCU> vertices;
    vertices.reserve(6);

    // Two triangles make up a full-screen quadrilateral
    // NDC coordinate system: lower left (-1,-1) to upper right (1,1)
    vertices.push_back(Vertex_PCU(Vec3(-1.f, -1.f, 0.f), Rgba8::WHITE, Vec2(0.f, 1.f)));
    vertices.push_back(Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8::WHITE, Vec2(1.f, 1.f)));
    vertices.push_back(Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f)));

    vertices.push_back(Vertex_PCU(Vec3(-1.f, 1.f, 0.f), Rgba8::WHITE, Vec2(0.f, 0.f)));
    vertices.push_back(Vertex_PCU(Vec3(1.f, -1.f, 0.f), Rgba8::WHITE, Vec2(1.f, 1.f)));
    vertices.push_back(Vertex_PCU(Vec3(1.f, 1.f, 0.f), Rgba8::WHITE, Vec2(1.f, 0.f)));

    m_fullscreenQuadVB = renderer.CreateVertexBuffer(vertices.size() * sizeof(Vertex_PCU), 0);
    renderer.CopyCPUToGPU(vertices.data(), vertices.size() * sizeof(Vertex_PCU), m_fullscreenQuadVB);

    // Create a constant buffer
    m_bloomConstantsCB = renderer.CreateConstantBuffer(sizeof(BloomConstants));
    m_blurConstantsCB  = renderer.CreateConstantBuffer(sizeof(BlurConstants));
}

void EffectBloom::CreateRenderTargets(IRenderer& renderer)
{
    IntVec2 screenSize = g_theWindow->GetClientDimensions();

    DXGI_FORMAT rtvFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
    // Create Luminance Extraction RT (Full Resolution)
    // 32-bit HDR format for Bloom
    // m_brightnessRT = renderer.CreateRenderTarget(screenSize, DXGI_FORMAT_R11G11B10_FLOAT);
    // Uses standard RGBA8 format (simple but does not support HDR)
    m_brightnessRT = renderer.CreateRenderTarget(screenSize, rtvFormat);

    // Create Mip chain
    m_bloomMipChain.resize(m_bloomLevels);
    IntVec2 currentSize = screenSize;
    for (int i = 0; i < m_bloomLevels; ++i)
    {
        currentSize.x = max(1, currentSize.x / 2);
        currentSize.y = max(1, currentSize.y / 2);

        auto& level = m_bloomMipChain[i];
        level.size  = currentSize;

        // All Bloom-related RTs use the same format
        level.downsample = renderer.CreateRenderTarget(currentSize, rtvFormat);
        level.blurTemp   = renderer.CreateRenderTarget(currentSize, rtvFormat);
    }
}

void EffectBloom::ExtractBrightness(IRenderer& renderer, RenderTarget* source, RenderTarget* dest)
{
    renderer.SetRenderTarget(dest);
    renderer.SetViewport(dest->dimensions);
    renderer.ClearRenderTarget(dest, Rgba8::BLACK);

    renderer.BindShader(m_brightnessExtractShader);
    renderer.BindTexture(source->srv, 0);
    renderer.BindConstantBuffer(0, m_bloomConstantsCB);

    DrawFullscreenQuad(renderer);
}

void EffectBloom::DownsampleBlur(IRenderer& renderer, int level)
{
    BloomLevel& currentLevel = m_bloomMipChain[level];

    // Source texture
    RenderTarget* source = (level == 0) ? m_brightnessRT : m_bloomMipChain[level - 1].blurTemp;

    // Down sampling
    renderer.SetRenderTarget(currentLevel.downsample);
    renderer.SetViewport(currentLevel.size);
    renderer.ClearRenderTarget(currentLevel.downsample, Rgba8::BLACK);

    renderer.BindShader(m_downsampleShader);
    renderer.BindTexture(source->srv, 0);

    // Update texel size
    m_blurConstants.texelSize = Vec2(1.0f / (float)currentLevel.size.x, 1.0f / (float)currentLevel.size.y);
    renderer.CopyCPUToGPU(&m_blurConstants, sizeof(BlurConstants), m_blurConstantsCB);
    renderer.BindConstantBuffer(1, m_blurConstantsCB);

    DrawFullscreenQuad(renderer);

    // Horizontal Blur
    renderer.SetRenderTarget(currentLevel.blurTemp);
    renderer.ClearRenderTarget(currentLevel.blurTemp, Rgba8::BLACK);

    renderer.BindShader(m_gaussianBlurShader);
    renderer.BindTexture(currentLevel.downsample->srv, 0);

    SetBlurDirection(renderer, true); // horizontal
    DrawFullscreenQuad(renderer);

    // Vertical Blur
    renderer.SetRenderTarget(currentLevel.downsample);
    renderer.BindTexture(currentLevel.blurTemp->srv, 0);

    SetBlurDirection(renderer, false); // vertically
    DrawFullscreenQuad(renderer);

    // The end result is in blurTemp
    std::swap(currentLevel.downsample, currentLevel.blurTemp);
}

void EffectBloom::UpsampleCombine(IRenderer& renderer, int level)
{
    auto& currentLevel = m_bloomMipChain[level];
    auto& higherLevel  = m_bloomMipChain[level + 1];

    renderer.SetRenderTarget(currentLevel.downsample);
    renderer.SetViewport(currentLevel.size);

    renderer.BindShader(m_upsampleShader);

    // Bind two textures
    renderer.BindTexture(higherLevel.blurTemp->srv, 0); // Last level
    renderer.BindTexture(currentLevel.blurTemp->srv, 1); // Current level

    // Additive mixing
    renderer.SetBlendMode(BlendMode::ADDITIVE);

    DrawFullscreenQuad(renderer);

    // Swap buffers
    std::swap(currentLevel.downsample, currentLevel.blurTemp);
}

void EffectBloom::FinalComposite(IRenderer& renderer, RenderTarget* scene, RenderTarget* bloom, RenderTarget* output)
{
    renderer.SetRenderTarget(output);
    renderer.SetViewport(output->dimensions);
    renderer.SetBlendMode(BlendMode::OPAQUE);

    renderer.BindShader(m_compositeShader);
    renderer.BindTexture(scene->srv, 0);
    renderer.BindTexture(bloom->srv, 1);
    renderer.BindConstantBuffer(0, m_bloomConstantsCB);

    DrawFullscreenQuad(renderer);
}

void EffectBloom::DrawFullscreenQuad(IRenderer& renderer)
{
    renderer.BindVertexBuffer(m_fullscreenQuadVB);
    renderer.DrawVertexBuffer(m_fullscreenQuadVB, 6);
}

void EffectBloom::UpdateConstants()
{
    if (m_renderer && m_bloomConstantsCB)
    {
        m_renderer->CopyCPUToGPU(&m_bloomConstants, sizeof(BloomConstants), m_bloomConstantsCB);
    }
}

void EffectBloom::SetBlurDirection(IRenderer& renderer, bool horizontal)
{
    m_blurConstants.blurDirection = horizontal ? 0.0f : 1.0f;
    renderer.CopyCPUToGPU(&m_blurConstants, sizeof(BlurConstants), m_blurConstantsCB);
    renderer.BindConstantBuffer(1, m_blurConstantsCB);
}
