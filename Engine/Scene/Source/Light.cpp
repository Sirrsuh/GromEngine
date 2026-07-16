#include <Scene/Light.h>

namespace grom {

Light::Light() : SceneNode("Light")
{
}

Light::Light(const GString& name) : SceneNode(name)
{
}

f32 Light::GetAttenuation(f32 distance) const
{
    f32 denom = distance / m_Range;
    f32 atten = 1.0f / (denom * denom);
    return atten > 1.0f ? 1.0f : atten;
}

Light::GPULight Light::GetGPULight() const
{
    GPULight gpu{};
    gpu.Position = GetWorldPosition();
    gpu.Range = m_Range;
    gpu.Direction = GetForward();
    gpu.Intensity = m_Intensity;
    gpu.Color = m_Color;
    f32 innerRad = m_InnerConeAngle * GROM_PI / 180.0f;
    f32 outerRad = m_OuterConeAngle * GROM_PI / 180.0f;
    gpu.InnerConeCos = cosf(innerRad);
    gpu.OuterConeCos = cosf(outerRad);
    gpu.Type = static_cast<u32>(m_LightType);
    gpu.CastShadows = m_CastShadows ? 1 : 0;
    gpu.ShadowMapIndex = 0;
    gpu.Padding = 0.0f;
    return gpu;
}

}
