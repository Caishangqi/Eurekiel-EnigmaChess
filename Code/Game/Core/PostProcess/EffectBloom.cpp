#include "EffectBloom.hpp"

#include "Engine/Core/EngineCommon.hpp"
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
    // Create Luminance Extraction RT (Full Resolution)
    m_brightnessRT = renderer.CreateRenderTarget(screenSize);
}

void EffectBloom::ExtractBrightness(IRenderer& renderer, RenderTarget* source, RenderTarget* dest)
{
}

void EffectBloom::DownsampleBlur(IRenderer& renderer, int level)
{
}

void EffectBloom::UpsampleCombine(IRenderer& renderer, int level)
{
}

void EffectBloom::FinalComposite(IRenderer& renderer, RenderTarget* scene, RenderTarget* bloom, RenderTarget* output)
{
}

void EffectBloom::DrawFullscreenQuad(IRenderer& renderer)
{
}

void EffectBloom::UpdateConstants()
{
}

void EffectBloom::SetBlurDirection(IRenderer& renderer, bool horizontal)
{
}
