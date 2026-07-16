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
    float4 Color : SV_Target0;
    float4 Normal : SV_Target1;
    float4 RoughnessMetallicAO : SV_Target2;
    float4 Emissive : SV_Target3;
};

PSOutput PSMain(VSOutput input)
{
    PSOutput output;

    float4 baseColor = BaseColor;
#ifdef FEATURE_VERTEX_COLORS
    baseColor *= input.Color;
#endif

    float3 N = normalize(input.WorldNormal);
#ifdef FEATURE_NORMAL_MAP
    float3 T = normalize(input.WorldTangent);
    float3 B = normalize(input.WorldBitangent);
    float3x3 TBN = float3x3(T, B, N);
    float3 sampledNormal = tex2D(SamplerLinear, input.UV0).xyz * 2.0 - 1.0;
    N = normalize(mul(sampledNormal, TBN));
#endif

    float roughness = Roughness;
#ifdef FEATURE_ROUGHNESS_MAP
    roughness *= tex2D(SamplerLinear, input.UV0).g;
#endif

    float metallic = Metallic;
#ifdef FEATURE_METALLIC_MAP
    metallic *= tex2D(SamplerLinear, input.UV0).b;
#endif

    float ao = AmbientOcclusion;
#ifdef FEATURE_AO_MAP
    ao *= tex2D(SamplerLinear, input.UV0).r;
#endif

    float opacity = Opacity;
#ifdef FEATURE_OPACITY_MAP
    opacity *= tex2D(SamplerLinear, input.UV0).a;
#endif
    if (opacity < 0.01) discard;

    float3 V = normalize(CameraPosition.xyz - input.WorldPosition);
    float3 L = -SunDirection.xyz;
    float3 H = normalize(V + L);

    float3 F0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, metallic);

    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float HdotV = max(dot(H, V), 0.0);

    float3 F = FresnelSchlickRoughness(HdotV, F0, roughness);
    float D = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);

    float3 kD = lerp(float3(1.0, 1.0, 1.0) - F, float3(0.0, 0.0, 0.0), metallic);
    kD *= 1.0 - metallic;

    float3 specularBRDF = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);
    float3 diffuseBRDF = kD * baseColor.rgb / 3.14159;

    float3 directLighting = (diffuseBRDF + specularBRDF) * SunColor.rgb * SunColor.a * NdotL;
    float3 ambientLighting = AmbientColor.rgb * AmbientColor.a * ao * baseColor.rgb;

    float3 emissive = EmissiveColor.rgb * EmissiveColor.a;

    float3 finalColor = directLighting + ambientLighting + emissive;

    output.Color = float4(finalColor, opacity);
    output.Normal = float4(N * 0.5 + 0.5, 1.0);
    output.RoughnessMetallicAO = float4(roughness, metallic, ao, 1.0);
    output.Emissive = float4(emissive, 1.0);

    return output;
}

float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(float3(1.0 - roughness, 1.0 - roughness, 1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    return a2 / (3.14159 * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float3 ImportanceSampleGGX(float2 Xi, float3 N, float roughness)
{
    float a = roughness * roughness;
    float phi = 2.0 * 3.14159 * Xi.x;
    float cosTheta = sqrt((1.0 - Xi.y) / (1.0 + (a*a - 1.0) * Xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    float3 H;
    H.x = sinTheta * cos(phi);
    H.y = sinTheta * sin(phi);
    H.z = cosTheta;

    float3 up = abs(N.z) < 0.999 ? float3(0.0, 0.0, 1.0) : float3(1.0, 0.0, 0.0);
    float3 tangentX = normalize(cross(up, N));
    float3 tangentY = cross(N, tangentX);

    return tangentX * H.x + tangentY * H.y + N * H.z;
}
