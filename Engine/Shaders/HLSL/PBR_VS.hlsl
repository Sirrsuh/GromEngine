#include "Common.hlsl"

VSOutput main(VSInput input)
{
    VSOutput output;

    float4 worldPos = mul(float4(input.Position, 1.0), WorldMatrix);
    float4 viewPos = mul(worldPos, ViewMatrix);
    float4 projPos = mul(viewPos, ProjectionMatrix);

    output.Position = projPos;
    output.WorldPosition = worldPos.xyz;
    output.WorldNormal = normalize(mul(float4(input.Normal, 0.0), NormalMatrix).xyz);

    float3 tangent = normalize(mul(float4(input.Tangent.xyz, 0.0), NormalMatrix).xyz);
    output.WorldTangent = tangent;
    float3 bitangent = cross(output.WorldNormal, tangent) * input.Tangent.w;
    output.WorldBitangent = bitangent;

    output.UV0 = input.UV0;
    output.UV1 = input.UV1;
    output.Color = input.Color;

    float4 prevWorldPos = mul(float4(input.Position, 1.0), PrevWorldMatrix);
    output.PrevPosition = mul(prevWorldPos, ViewProjectionMatrix);

    return output;
}
