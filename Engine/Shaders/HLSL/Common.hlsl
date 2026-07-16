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

SamplerState SamplerLinear    : register(s0);
SamplerState SamplerPoint     : register(s1);
SamplerState SamplerAniso     : register(s2);
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

float3 FresnelSchlick(float cosTheta, float3 F0);
float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness);
float DistributionGGX(float3 N, float3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(float3 N, float3 V, float3 L, float roughness);
float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness);

#endif
