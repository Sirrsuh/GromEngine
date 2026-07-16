#include "Common.hlsl"

TextureCube SkyboxTexture : register(t0);

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 UVW : TEXCOORD0;
};

float4 PSMain(VSOutput input) : SV_Target0
{
    return SkyboxTexture.Sample(SamplerLinear, input.UVW);
}
