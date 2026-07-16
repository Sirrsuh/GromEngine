#include "Common.hlsl"

struct VSOutput
{
    float4 Position : SV_POSITION;
    float3 UVW : TEXCOORD0;
};

VSOutput VSMain(VSInput input)
{
    VSOutput output;
    float4 pos = mul(float4(input.Position, 1.0), WorldViewProjection);
    output.Position = pos.xyww;
    output.UVW = input.Position;
    return output;
}
