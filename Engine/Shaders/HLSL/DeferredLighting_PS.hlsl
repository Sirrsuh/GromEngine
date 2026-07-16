#include "Common.hlsl"

Texture2D AlbedoTexture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D RMATexture : register(t2);
Texture2D EmissiveTexture : register(t3);
Texture2D DepthTexture : register(t4);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};

float3 WorldPosFromDepth(float2 uv, float depth)
{
    float4 clipPos = float4(uv * 2.0 - 1.0, depth, 1.0);
    float4 worldPos = mul(clipPos, InvViewProjectionMatrix);
    return worldPos.xyz / worldPos.w;
}

float4 PSMain(VSOutput input) : SV_Target0
{
    float depth = DepthTexture.Sample(SamplerPoint, input.UV).r;
    if (depth >= 1.0)
        discard;

    float4 albedo = AlbedoTexture.Sample(SamplerPoint, input.UV);
    float3 N = NormalTexture.Sample(SamplerPoint, input.UV).xyz * 2.0 - 1.0;
    float roughness = RMATexture.Sample(SamplerPoint, input.UV).r;
    float metallic = RMATexture.Sample(SamplerPoint, input.UV).g;
    float ao = RMATexture.Sample(SamplerPoint, input.UV).b;
    float3 emissive = EmissiveTexture.Sample(SamplerPoint, input.UV).rgb;

    float3 worldPos = WorldPosFromDepth(input.UV, depth);

    float3 V = normalize(CameraPosition.xyz - worldPos);
    float3 L = -SunDirection.xyz;
    float3 H = normalize(V + L);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.rgb, metallic);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F = FresnelSchlickRoughness(HdotV, F0, roughness);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float3 kD = lerp(float3(1.0, 1.0, 1.0) - F, float3(0.0, 0.0, 0.0), metallic);
    kD *= 1.0 - metallic;

    float3 specularBRDF = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
    float3 diffuseBRDF = kD * albedo.rgb / 3.14159;

    float3 directLighting = (diffuseBRDF + specularBRDF) * SunColor.rgb * SunColor.a * NdotL;
    float3 ambientLighting = AmbientColor.rgb * AmbientColor.a * ao * albedo.rgb;

    float3 finalColor = directLighting + ambientLighting + emissive;

    return float4(finalColor, 1.0);
}
