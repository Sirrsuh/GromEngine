#include "Common.hlsl"

VSOutput VSMain(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0), WorldMatrix);
    output.WorldPosition = worldPos.xyz;
    output.Position = mul(worldPos, ViewProjectionMatrix);
    output.PrevPosition = mul(float4(input.Position, 1.0), mul(PrevWorldMatrix, ViewProjectionMatrix));

    output.WorldNormal = normalize(mul(float4(input.Normal, 0.0), NormalMatrix).xyz);

    float3 tangent = normalize(mul((float3x3)WorldMatrix, input.Tangent.xyz));
    output.WorldTangent = tangent;
    output.WorldBitangent = cross(output.WorldNormal, tangent) * input.Tangent.w;

    output.UV0 = input.UV0;
    output.UV1 = input.UV1;
    output.Color = input.Color;

    return output;
}
