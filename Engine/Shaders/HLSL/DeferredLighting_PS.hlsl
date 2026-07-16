#include "Common.hlsl"

Texture2D<float4> AlbedoRT   : register(t0);
Texture2D<float4> NormalRT   : register(t1);
Texture2D<float4> RMART      : register(t2);
Texture2D<float4> EmissiveRT : register(t3);
Texture2D<float>  DepthRT    : register(t4);
Texture2D<float>  ShadowMap  : register(t5);
TextureCube<float4> SkyboxTexture : register(t6);

float4 main(float4 Position : SV_POSITION,
            float2 UV : TEXCOORD) : SV_Target0
{
    float depth = DepthRT.Sample(SamplerPoint, UV).r;
    float4 albedo = AlbedoRT.Sample(SamplerPoint, UV);
    float4 normalPack = NormalRT.Sample(SamplerPoint, UV);
    float4 rma = RMART.Sample(SamplerPoint, UV);
    float4 emissive = EmissiveRT.Sample(SamplerPoint, UV);

    float roughness = rma.r;
    float metallic = rma.g;
    float ao = rma.b;

    float3 N = normalize(normalPack.xyz);
    float3 V = normalize(CameraPosition.xyz - ReconstructWorldPos(UV, depth));

    float3 Lo = float3(0.0, 0.0, 0.0);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo.xyz, metallic);

    float3 L = normalize(-SunDirection.xyz);
    float3 H = normalize(V + L);

    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);

    float shadow = 1.0;
    if (NdotL > 0.0)
    {
        float4 shadowProj = mul(float4(ReconstructWorldPos(UV, depth), 1.0), ShadowMatrix);
        float2 shadowUV = shadowProj.xy / shadowProj.w * float2(0.5, -0.5) + 0.5;
        float shadowZ = shadowProj.z / shadowProj.w;
        shadow = ShadowMap.SampleCmpLevelZero(SamplerShadow, shadowUV, shadowZ - ShadowBias);
        shadow = lerp(1.0, shadow, ShadowStrength);
    }

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 kS = F;
    float3 kD = (1.0 - kS) * (1.0 - metallic);

    float3 numerator = NDF * G * F;
    float denominator = 4.0 * NdotV * NdotL + 0.0001;
    float3 specular = numerator / denominator;

    Lo += (kD * albedo.xyz / PI + specular) * SunColor.xyz * NdotL * shadow;

    Lo += AmbientColor.xyz * albedo.xyz * ao;

    Lo += emissive.xyz;

    float3 color = Lo * Exposure;

    if (ToneMapMode == 1.0)
        color = ACESFilm(color);

    return float4(color, 1.0);
}
