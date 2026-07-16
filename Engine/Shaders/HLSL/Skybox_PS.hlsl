#include "Common.hlsl"

TextureCube<float4> SkyboxTexture : register(t0);

float4 main(float4 Position : SV_POSITION,
            float3 WorldPos : TEXCOORD) : SV_Target0
{
    float3 dir = normalize(WorldPos - CameraPosition.xyz);
    float4 color = SkyboxTexture.Sample(SamplerLinear, dir);
    float3 result = color.xyz * Exposure;

    if (ToneMapMode == 1.0)
        result = ACESFilm(result);

    return float4(result, 1.0);
}
