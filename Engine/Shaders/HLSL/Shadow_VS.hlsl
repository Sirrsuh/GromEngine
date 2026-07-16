#include "Common.hlsl"

struct ShadowVSOut
{
    float4 Position : SV_POSITION;
};

ShadowVSOut main(VSInput input)
{
    ShadowVSOut output;
    float4 worldPos = mul(float4(input.Position, 1.0), WorldMatrix);
    output.Position = mul(worldPos, ShadowMatrix);
    return output;
}
