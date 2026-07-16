#ifndef __COMMON_HLSL__
#define __COMMON_HLSL__

cbuffer FrameConstants : register(b0)
{
    float4x4 ViewMatrix;
    float4x4 ProjectionMatrix;
    float4x4 ViewProjectionMatrix;
    float4x4 InvViewMatrix;
    float4x4 InvProjectionMatrix;
    float4x4 InvViewProjectionMatrix;
    float4 CameraPosition;
    float4 CameraDirection;
    float2 ViewportSize;
    float2 ViewportPixelSize;
    float NearPlane;
    float FarPlane;
    float DeltaTime;
    float TotalTime;
    uint FrameCount;
    uint bOrthographic;
    float2 DummyFrame;
}

cbuffer ObjectConstants : register(b1)
{
    float4x4 WorldMatrix;
    float4x4 WorldViewProjection;
    float4x4 PrevWorldMatrix;
    float4x4 NormalMatrix;
    float4 ObjectColor;
    uint ObjectID;
    uint bSelected;
    float2 DummyObject;
}

cbuffer LightConstants : register(b3)
{
    float4 AmbientColor;
    float4 SunDirection;
    float4 SunColor;
    float4 IBLSpecFactor;
    uint NumPointLights;
    uint NumSpotLights;
    uint NumDecals;
    float DummyLight;
}

cbuffer ShadowConstants : register(b4)
{
    float4x4 ShadowMatrix;
    float ShadowMapSize;
    float ShadowBias;
    float ShadowStrength;
    float DummyShadow;
}

cbuffer ToneMapConstants : register(b5)
{
    float Exposure;
    float ToneMapMode;
    float2 DummyToneMap;
}

SamplerState SamplerLinear       : register(s0);
SamplerState SamplerPoint        : register(s1);
SamplerState SamplerAniso        : register(s2);
SamplerComparisonState SamplerShadow : register(s3);

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float4 Tangent  : TANGENT;
    float2 UV0      : TEXCOORD0;
    float2 UV1      : TEXCOORD1;
    float4 Color    : COLOR;
};

struct VSOutput
{
    float4 Position      : SV_POSITION;
    float3 WorldPosition : TEXCOORD0;
    float3 WorldNormal   : TEXCOORD1;
    float3 WorldTangent  : TEXCOORD2;
    float3 WorldBitangent : TEXCOORD3;
    float2 UV0           : TEXCOORD4;
    float2 UV1           : TEXCOORD5;
    float4 Color         : TEXCOORD6;
    float4 PrevPosition  : TEXCOORD7;
};

float3 ReconstructWorldPos(float2 uv, float depth)
{
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    clipPos.y = -clipPos.y;
    float4 worldPos = mul(clipPos, InvViewProjectionMatrix);
    return worldPos.xyz / worldPos.w;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * PI * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a * a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    float3 H;
    H.x = cos(phi) * sinTheta;
    H.y = sin(phi) * sinTheta;
    H.z = cosTheta;
    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangent = normalize(cross(up, N));
    float3 bitangent = cross(N, tangent);
    return tangent * H.x + bitangent * H.y + N * H.z;
}

static const float PI = 3.14159265359;

float3 ACESFilm(float3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

#endif
