#include "Common.hlsl"

Texture2D<float4> HDRTexture : register(t0);

float4 main(float4 Position : SV_POSITION, float2 UV : TEXCOORD) : SV_Target0
{
    float3 color = HDRTexture.Sample(SamplerLinear, UV).xyz;
    float luminance = dot(color, float3(0.2126, 0.7152, 0.0722));
    float threshold = 1.0;
    float knee = 0.5;
    float soft = luminance - threshold + knee;
    soft = clamp(soft, 0.0, 2.0 * knee);
    soft = soft * soft / (4.0 * knee + 0.0001);
    float contribution = max(soft, luminance - threshold);
    return float4(color * contribution, 1.0);
}
