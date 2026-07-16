#include "Common.hlsl"

Texture2D<float4> HDRTexture : register(t0);

float4 main(float4 Position : SV_POSITION,
            float2 UV : TEXCOORD) : SV_Target0
{
    float3 color = HDRTexture.Sample(SamplerLinear, UV).xyz;

    color *= Exposure;

    if (ToneMapMode == 1.0)
        color = ACESFilm(color);

    return float4(color, 1.0);
}
