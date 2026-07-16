#include "Common.hlsl"

Texture2D<float4> AlbedoTexture   : register(t0);
Texture2D<float4> NormalTexture   : register(t1);
Texture2D<float4> RMATexture      : register(t2);
Texture2D<float4> EmissiveTexture : register(t3);

struct PSOutput
{
    float4 Albedo   : SV_Target0;
    float4 Normal   : SV_Target1;
    float4 RMA      : SV_Target2;
    float4 Emissive : SV_Target3;
};

PSOutput main(VSOutput input)
{
    PSOutput output;

    float4 albedoSample = AlbedoTexture.Sample(SamplerLinear, input.UV0);
    float4 rmaSample = RMATexture.Sample(SamplerLinear, input.UV0);
    float4 emissiveSample = EmissiveTexture.Sample(SamplerLinear, input.UV0);
    float4 normalSample = NormalTexture.Sample(SamplerLinear, input.UV0);

    float roughness = rmaSample.g;
    float metallic = rmaSample.r;
    float ao = rmaSample.b;

    output.Albedo = albedoSample * ObjectColor;
    output.Normal = float4(normalize(input.WorldNormal), 1.0);
    output.RMA = float4(roughness, metallic, ao, 1.0);
    output.Emissive = emissiveSample;

    return output;
}
