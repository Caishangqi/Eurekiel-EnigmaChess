//------------------------------------------------------------------------------------------------
// Bloom Post-Processing Shader
// Implements multi-pass bloom effect with brightness extraction and gaussian blur
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
// Constant Buffers
//------------------------------------------------------------------------------------------------
cbuffer BloomConstants : register(b0)
{
    float c_threshold; // Brightness threshold
    float c_intensity; // Bloom intensity multiplier
    float c_blurSigma; // Blur kernel sigma
    float c_padding; // Padding for 16-byte alignment
};

cbuffer BlurConstants : register(b1)
{
    float2 c_texelSize; // 1.0 / texture dimensions
    float  c_blurDirection; // 0 for horizontal, 1 for vertical
    float  c_padding2;
};

//------------------------------------------------------------------------------------------------
// Textures and Samplers
//------------------------------------------------------------------------------------------------
Texture2D    t_sourceTexture : register(t0);
Texture2D    t_bloomTexture : register(t1); // For final composite pass
SamplerState s_linearSampler : register(s0);

//------------------------------------------------------------------------------------------------
// Vertex Shader Input/Output Structures
//------------------------------------------------------------------------------------------------
struct vs_input_t
{
    float3 position : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
};

struct VertexOutPixelIn
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD;
};

//------------------------------------------------------------------------------------------------
// Vertex Shader - Simple pass-through for full-screen quad
//------------------------------------------------------------------------------------------------
VertexOutPixelIn PostProcessVS(vs_input_t input)
{
    VertexOutPixelIn output;
    output.position = float4(input.position, 1.0f);
    output.uv       = input.uv;
    return output;
}

//------------------------------------------------------------------------------------------------
// Helper Functions
//------------------------------------------------------------------------------------------------
float3 SampleBox(Texture2D tex, float2 uv, float2 texelSize)
{
    // 4-tap box filter for better quality downsampling
    float3 color = float3(0.0f, 0.0f, 0.0f);

    color += tex.Sample(s_linearSampler, uv + float2(-0.5f, -0.5f) * texelSize).rgb;
    color += tex.Sample(s_linearSampler, uv + float2(0.5f, -0.5f) * texelSize).rgb;
    color += tex.Sample(s_linearSampler, uv + float2(-0.5f, 0.5f) * texelSize).rgb;
    color += tex.Sample(s_linearSampler, uv + float2(0.5f, 0.5f) * texelSize).rgb;

    return color * 0.25f;
}

//------------------------------------------------------------------------------------------------
// Pixel Shader - Brightness Extraction Pass
//------------------------------------------------------------------------------------------------
float4 BrightnessExtractPS(VertexOutPixelIn input) : SV_Target0
{
    float3 color = t_sourceTexture.Sample(s_linearSampler, input.uv).rgb;

    // Calculate luminance using standard weights
    float luminance = dot(color, float3(0.299f, 0.587f, 0.114f));

    // Soft threshold with smooth transition
    float brightnessMask = smoothstep(c_threshold - 0.1f, c_threshold + 0.1f, luminance);

    // Extract bright areas while preserving color
    float3 brightColor = color * brightnessMask;

    return float4(brightColor, 1.0f);
}

//------------------------------------------------------------------------------------------------
// Pixel Shader - Downsample Pass (13-tap filter for high quality)
//------------------------------------------------------------------------------------------------
float4 DownsamplePS(VertexOutPixelIn input) : SV_Target0
{
    float2 texelSize = c_texelSize;
    float3 color     = float3(0.0f, 0.0f, 0.0f);

    // Center tap
    color += t_sourceTexture.Sample(s_linearSampler, input.uv).rgb * 0.125f;

    // Inner ring (4 samples)
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(-1, -1) * texelSize).rgb * 0.125f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(1, -1) * texelSize).rgb * 0.125f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(-1, 1) * texelSize).rgb * 0.125f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(1, 1) * texelSize).rgb * 0.125f;

    // Outer ring (8 samples)
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(-2, -2) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(0, -2) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(2, -2) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(-2, 0) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(2, 0) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(-2, 2) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(0, 2) * texelSize).rgb * 0.0625f;
    color += t_sourceTexture.Sample(s_linearSampler, input.uv + float2(2, 2) * texelSize).rgb * 0.0625f;

    return float4(color, 1.0f);
}

//------------------------------------------------------------------------------------------------
// Pixel Shader - Gaussian Blur Pass (Separable)
//------------------------------------------------------------------------------------------------
// Pre-calculated gaussian weights for 15-tap kernel (sigma = 2.0)
static const float GaussianWeights[15] =
{
    0.00239f, 0.00932f, 0.02745f, 0.06136f, 0.10407f, 0.13390f, 0.13084f, 0.09703f,
    0.05462f, 0.02332f, 0.00755f, 0.00186f, 0.00035f, 0.00005f, 0.00001f
};

float4 GaussianBlurPS(VertexOutPixelIn input) : SV_Target0
{
    float3 color  = float3(0.0f, 0.0f, 0.0f);
    float2 offset = float2(0.0f, 0.0f);

    // Determine blur direction
    if (c_blurDirection < 0.5f) // Horizontal
    {
        offset.x = c_texelSize.x;
    }
    else // Vertical
    {
        offset.y = c_texelSize.y;
    }

    // 15-tap gaussian blur
    [unroll]
    for (int i = -7; i <= 7; ++i)
    {
        float2 sampleUV    = input.uv + offset * float(i);
        float3 sampleColor = t_sourceTexture.Sample(s_linearSampler, sampleUV).rgb;
        color += sampleColor * GaussianWeights[abs(i) + 7];
    }

    return float4(color, 1.0f);
}

//------------------------------------------------------------------------------------------------
// Pixel Shader - Upsample and Combine Pass
//------------------------------------------------------------------------------------------------
float4 UpsamplePS(VertexOutPixelIn input) : SV_Target0
{
    // Sample lower resolution bloom
    float3 bloomLower = t_sourceTexture.Sample(s_linearSampler, input.uv).rgb;

    // Sample current resolution bloom  
    float3 bloomCurrent = t_bloomTexture.Sample(s_linearSampler, input.uv).rgb;

    // Combine with weight
    float3 combined = lerp(bloomCurrent, bloomLower, 0.5f);

    return float4(combined, 1.0f);
}

//------------------------------------------------------------------------------------------------
// Pixel Shader - Final Composite Pass
//------------------------------------------------------------------------------------------------
float4 CompositePS(VertexOutPixelIn input) : SV_Target0
{
    // Sample original scene
    float3 sceneColor = t_sourceTexture.Sample(s_linearSampler, input.uv).rgb;

    // Sample bloom
    float3 bloomColor = t_bloomTexture.Sample(s_linearSampler, input.uv).rgb;

    // Apply bloom intensity
    bloomColor *= c_intensity;

    // Additive blend
    float3 finalColor = sceneColor + bloomColor;

    // Simple tone mapping to prevent over-exposure
    // Using Reinhard tone mapping: color / (1 + color)
    finalColor = finalColor / (1.0f + finalColor);

    return float4(finalColor, 1.0f);
}
