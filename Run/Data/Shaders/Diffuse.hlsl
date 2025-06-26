//#include "ShaderMath.hlsl"
/// Shader math Common


//namespace shader::math
//{
float RangeMapClamped(float inValue, float inStart, float inEnd, float outStart, float outEnd)
{
    float fraction = saturate((inValue - inStart) / (inEnd - inStart));
    float outValue = outStart + fraction * (outEnd - outStart);
    return outValue;
}

/// Used standard normal color encoding, mapping rgb in [0,1] to xyz in [-1,1]
/// @param color The normal map color in
/// @return the TBN space normal vector
float3 DecodeRGBToXYZ(float3 color)
{
    return (color * 2.0) - 1.0;
}

/// Used standard normal color encoding, mapping rgb in [0,1] to xyz in [-1,1]
/// @param xyz the vector that between [-1,1]
/// @return the color between [0,1]
float3 EncodeXYZToRGB(float3 xyz)
{
    float3 normalizedXYZ = normalize(xyz);
    return (normalizedXYZ + 1.0) * 0.5;
}

//}


/// In order to compile shader from the include file, user need to fill the
/// D3DCompile() func parameter ID3DInclude* pInclude which will need the user
/// to implement the interfaces.
/// @link https://learn.microsoft.com/en-us/previous-versions/windows/desktop/legacy/bb173775(v=vs.85)
/// However, the FXC compiler could automatically handle the include and compile to .cso (complied shader object).
/// User need to loaded tha .cso file and still need to loaded shader
/// @link https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-part1


//------------------------------------------------------------------------------------------------
struct vs_input_t
{
    float3 localPosition : POSITION;
    float4 color : COLOR;
    float2 uv : TEXCOORD;
    float3 localTangent : TANGENT; // Model space
    float3 localBitangent : BITANGENT;
    float3 localNormal : NORMAL;
};

//------------------------------------------------------------------------------------------------
struct VertexOutPixelIn
{
    float4 position : SV_Position; // Clip position (On screen)
    float4 color : COLOR;
    float2 uv : SURFACE_UVTEXCOORDS;
    // World
    float3 worldPosition: WORLD_POSITION;
    float3 worldTangent : WORLD_TANGENT;
    float3 worldBitangent : WORLD_BITANGENT;
    float3 worldNormal : WORLD_NORMAL;
    // Model
    float3 modelTangent : MODEL_TANGENT;
    float3 modelBitangent : MODEL_BITANGENT;
    float3 modelNormal : MODEL_NORMAL;
};

struct Light
{
    float4 worldPosition;
    float4 lightDirection;
    float4 color;
    float  innerPenumbra;
    float  outerPenumbra;
    float  innerRadius;
    float  outerRadius;
};

//------------------------------------------------------------------------------------------------
#define MAX_LIGHT 8

cbuffer LightConstants : register(b4)
{
    float3 SunDirection;
    float  SunIntensity;
    float  AmbientIntensity;
    int    NumLights;
    float  pad0;
    float  pad1;
    Light  lights[MAX_LIGHT];
};

//------------------------------------------------------------------------------------------------
cbuffer CameraConstants : register(b2)
{
    float4x4 WorldToCameraTransform;
    float4x4 CameraToRenderTransform;
    float4x4 RenderToClipTransform;
    float4x4 CameraToWorldTransform; // Camera position, use for calculate Spectral
};

//------------------------------------------------------------------------------------------------
cbuffer ModelConstants : register(b3)
{
    float4x4 ModelToWorldMatrix;
    float4   ModelColor;
};

cbuffer PerFrameConstants : register(b1)
{
    float    c_time;
    int      c_debugInt;
    int      c_debugViewMode;
    float    c_debugFloat;
    float4x4 EMPTY_PADDING;
};

//------------------------------------------------------------------------------------------------
Texture2D<float4> t_diffuseTexture : register(t0);
Texture2D<float4> t_normalTexture : register(t1);
Texture2D<float4> t_sgeTexture : register(t2); // Texture bound in texture constant slot #2 (t2)

//------------------------------------------------------------------------------------------------
SamplerState s_diffuseSampler : register(s0);
SamplerState s_normalSampler : register(s1);
SamplerState s_sgeSampler : register(s2); // Sampler is bound in sampler constant slot #2 (s2)

//------------------------------------------------------------------------------------------------
VertexOutPixelIn VertexMain(vs_input_t input)
{
    float4 worldPosition  = mul(ModelToWorldMatrix, float4(input.localPosition, 1));
    float4 cameraPosition = mul(WorldToCameraTransform, worldPosition);
    float4 renderPosition = mul(CameraToRenderTransform, cameraPosition);
    float4 clipPosition   = mul(RenderToClipTransform, renderPosition);

    VertexOutPixelIn v2p;
    v2p.position = clipPosition; // put the clip
    v2p.color    = input.color;
    v2p.uv       = input.uv;

    // Model space
    float4 modelTangent   = float4(input.localTangent, 0);
    float4 modelBitangent = float4(input.localBitangent, 0);
    float4 modelNormal    = float4(input.localNormal, 0);
    // TBN space to World space
    v2p.worldTangent   = mul(ModelToWorldMatrix, modelTangent).xyz;
    v2p.worldBitangent = mul(ModelToWorldMatrix, modelBitangent).xyz;
    v2p.worldNormal    = mul(ModelToWorldMatrix, modelNormal).xyz;

    // Assign and fill values
    v2p.modelTangent   = modelTangent.xyz;
    v2p.modelBitangent = modelBitangent.xyz;
    v2p.modelNormal    = modelNormal.xyz;

    v2p.worldPosition = worldPosition.xyz; // put the model to world position (world position)

    return v2p;
}

//------------------------------------------------------------------------------------------------
float4 PixelMain(VertexOutPixelIn input) : SV_Target0
{
    float ambient = AmbientIntensity;

    /// Sampler
    float2 uvCoords     = input.uv;
    float4 diffuseTexel = t_diffuseTexture.Sample(s_diffuseSampler, uvCoords);
    float4 normalTexel  = t_normalTexture.Sample(s_normalSampler, uvCoords);

    /// Smaple Glossy, Emissive, and Spectral
    float spectralTexel = t_sgeTexture.Sample(s_sgeSampler, uvCoords).r;
    float glossyTexel   = t_sgeTexture.Sample(s_sgeSampler, uvCoords).g;
    float emissiveTexel = t_sgeTexture.Sample(s_sgeSampler, uvCoords).b;

    float4 surfaceColor = input.color;
    float4 modelColor   = ModelColor;

    float3 pixelNormalTBNSpace = normalize(DecodeRGBToXYZ(normalTexel.rgb));
    // Tint diffuse color based on overall model tinting (including alpha translucency)
    float4 diffuseColor = diffuseTexel * surfaceColor * modelColor;

    float3 lightDirection = normalize(SunDirection);

    // TBN basis vector
    float3 surfaceTangentWorldSpace   = normalize(input.worldTangent);
    float3 surfaceBitangentWorldSpace = normalize(input.worldBitangent);
    float3 surfaceNormalWorldSpace    = normalize(input.worldNormal);

    float3 surfaceTangentModelSpace   = normalize(input.modelTangent);
    float3 surfaceBitangentModelSpace = normalize(input.modelBitangent);
    float3 surfaceNormalModelSpace    = normalize(input.modelNormal);

    // TBN to world
    float3x3 tbnToWorld            = float3x3(surfaceTangentWorldSpace, surfaceBitangentWorldSpace, surfaceNormalWorldSpace);
    float3   pixelNormalWorldSpace = mul(pixelNormalTBNSpace, tbnToWorld); // V*M order because this matrix is component-major (not basis-major!)


    // Blinn-Phong Specular (Half angle)
    float3 pixelToCamera    = normalize(transpose(CameraToWorldTransform)[3].xyz - input.worldPosition);
    float3 halfAngle        = normalize((0.5 * (pixelToCamera + (-lightDirection))));
    float  specularPower    = RangeMapClamped(glossyTexel, 0, 1, 1, 32);
    float  specularStrength = spectralTexel * pow(saturate(dot(halfAngle, pixelNormalWorldSpace)), specularPower);
    float3 specularLight    = specularStrength * float3(1, 1, 1);

    // Blinn-Phong Emissive (Original diffuse color)
    float3 emissiveLight = emissiveTexel * diffuseColor.rgb;

    float diffuseLightDot = dot(-lightDirection, pixelNormalWorldSpace);
    if (c_debugInt == 10 || c_debugInt == 12)
    {
        diffuseLightDot = dot(-lightDirection, surfaceNormalWorldSpace);
    }

    float lightStrength = saturate(RangeMapClamped(diffuseLightDot, -1.0, 1.0, -ambient, 1.0));

    float4 totalDiffuse  = float4(diffuseColor.rgb * lightStrength, diffuseColor.a) * SunIntensity;
    float4 totalSpecular = float4(specularLight, diffuseColor.a) * SunIntensity;
    float4 totalEmissive = float4(emissiveLight, diffuseColor.a);

    // Local light accumulation part in PixelMain
    float4 localLightAccum     = float4(0, 0, 0, 1);
    float4 localPureLightAccum = float4(0, 0, 0, 1); // For Debug Lighting
    for (int i = 0; i < NumLights; i++)
    {
        Light  currentLight       = lights[i];
        float3 pixelWorldPosition = input.worldPosition;
        float3 viewDirection      = pixelToCamera;
        float3 materialBaseColor  = diffuseColor.rgb;

        // Light source direction and distance
        float3 lightVector     = normalize(currentLight.worldPosition.xyz - pixelWorldPosition);
        float  distanceToLight = length(currentLight.worldPosition.xyz - pixelWorldPosition);
        float  ndotl           = saturate(dot(pixelNormalWorldSpace, lightVector));

        // Use color.a to control intensity
        float lightStrength = currentLight.color.a;

        // Use innerRadius and outerRadius to control distance attenuation
        float distanceFalloff = 1.0;
        if (currentLight.innerRadius < currentLight.outerRadius)
        {
            if (distanceToLight <= currentLight.innerRadius)
            {
                distanceFalloff = 1.0;
            }
            else if (distanceToLight >= currentLight.outerRadius)
            {
                distanceFalloff = 0.0;
            }
            else
            {
                distanceFalloff = RangeMapClamped(distanceToLight, currentLight.innerRadius, currentLight.outerRadius, 1.0, 0.0);
            }
        }

        // Spot light angle control
        float spotFalloff = 1.0;
        if (currentLight.innerPenumbra > currentLight.outerPenumbra)
        {
            float cosineAngle = dot(normalize(currentLight.lightDirection.xyz), -lightVector);
            if (cosineAngle <= currentLight.outerPenumbra)
            {
                spotFalloff = 0.0;
            }
            else if (cosineAngle >= currentLight.innerPenumbra)
            {
                spotFalloff = 1.0;
            }
            else
            {
                spotFalloff = RangeMapClamped(cosineAngle, currentLight.innerPenumbra, currentLight.outerPenumbra, 1.0, 0.0);
            }
        }

        // Diffuse reflection calculation
        float3 diffuseContribution = materialBaseColor * currentLight.color.rgb * lightStrength * ndotl * distanceFalloff * spotFalloff;

        // Debug: Extract pure color light intensity, pay attention to distance and spot influence
        localPureLightAccum += float4(currentLight.color.rgb * lightStrength * distanceFalloff * spotFalloff, 0);

        // Specular processing (Blinn‑Phong)
        float3 halfVector           = normalize(lightVector + viewDirection); // Calculate the half angle between view and light source
        float  specPower            = RangeMapClamped(glossyTexel, 0, 1, 1, 32);
        float  ndoth                = saturate(dot(pixelNormalWorldSpace, halfVector));
        float  specularFactor       = pow(ndoth, specPower) * spectralTexel * lightStrength * distanceFalloff * spotFalloff;
        float3 specularContribution = specularFactor * currentLight.color.rgb;

        // Accumulate to final lighting
        localLightAccum += float4(diffuseContribution + specularContribution, 0);
    }


    float4 finalColor = totalDiffuse + totalSpecular + totalEmissive + localLightAccum;

    if (finalColor.a <= 0.001)
    {
        discard;
    }

    if (c_debugViewMode == 4)
    {
        finalColor.rgba = float4(0, 0, 0, 1);
        finalColor += localPureLightAccum;
    }

    if (c_debugInt == 1)
    {
        finalColor.rgba = diffuseTexel.rgba;
    }
    else if (c_debugInt == 2)
    {
        finalColor.rgba = surfaceColor.rgba;
    }
    else if (c_debugInt == 3)
    {
        finalColor.rgb = float3(uvCoords.x, uvCoords.y, 0.f);
    }
    else if (c_debugInt == 4)
    {
        finalColor.rgb = EncodeXYZToRGB(surfaceTangentModelSpace);
    }
    else if (c_debugInt == 5)
    {
        finalColor.rgb = EncodeXYZToRGB(surfaceBitangentModelSpace);
    }
    else if (c_debugInt == 6)
    {
        finalColor.rgb = EncodeXYZToRGB(surfaceNormalModelSpace);
    }
    else if (c_debugInt == 7)
    {
        finalColor.rgba = normalTexel.rgba;
    }
    else if (c_debugInt == 8)
    {
        finalColor.rgb = EncodeXYZToRGB(pixelNormalTBNSpace);
    }
    else if (c_debugInt == 9)
    {
        finalColor.rgb = EncodeXYZToRGB(pixelNormalWorldSpace);
    }

    return finalColor;
}
