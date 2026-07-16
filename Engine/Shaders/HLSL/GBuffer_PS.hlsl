#include "Common.hlsl"

cbuffer MaterialParameters : register(b2)
{
    float4 BaseColor;
    float4 EmissiveColor;
    float Roughness;
    float Metallic;
    float AmbientOcclusion;
    float Opacity;
    float3 DummyMaterial;
    float TessFactor;
}

struct PSOutput
{
    float4 Albedo : SV_Target0;
    float4 Normal : SV_Target1;
    float4 RoughnessMetallicAO : SV_Target2;
    float4 Emissive : SV_Target3;
};

PSOutput PSMain(VSOutput input)
{
    PSOutput output;

    float4 baseColor = BaseColor;

    float3 N = normalize(input.WorldNormal);

    float roughness = Roughness;
    float metallic = Metallic;
    float ao = AmbientOcclusion;
    float opacity = Opacity;

    if (opacity < 0.01) discard;

    output.Albedo = float4(baseColor.rgb, opacity);
    output.Normal = float4(N * 0.5 + 0.5, 1.0);
    output.RoughnessMetallicAO = float4(roughness, metallic, ao, 1.0);
    output.Emissive = float4(EmissiveColor.rgb * EmissiveColor.a, 1.0);

    return output;
}
