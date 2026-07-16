#pragma once
#include <Core/Types.h>
#include <Core/Math.h>
#include <Scene/SceneNode.h>

namespace grom {

enum class ELightType {
    Directional,
    Point,
    Spot,
    Rectangular,
    Sky
};

class Light : public SceneNode {
public:
    Light();
    explicit Light(const GString& name);

    void SetLightType(ELightType type) { m_LightType = type; }
    ELightType GetLightType() const { return m_LightType; }

    void SetColor(const GColor& color) { m_Color = color; }
    GColor GetColor() const { return m_Color; }

    void SetIntensity(f32 intensity) { m_Intensity = intensity; }
    f32 GetIntensity() const { return m_Intensity; }

    void SetRange(f32 range) { m_Range = range; }
    f32 GetRange() const { return m_Range; }

    void SetInnerConeAngle(f32 angle) { m_InnerConeAngle = angle; }
    f32 GetInnerConeAngle() const { return m_InnerConeAngle; }

    void SetOuterConeAngle(f32 angle) { m_OuterConeAngle = angle; }
    f32 GetOuterConeAngle() const { return m_OuterConeAngle; }

    void SetCastShadows(bool cast) { m_CastShadows = cast; }
    bool GetCastShadows() const { return m_CastShadows; }

    void SetShadowMapSize(u32 size) { m_ShadowMapSize = size; }
    u32 GetShadowMapSize() const { return m_ShadowMapSize; }

    void SetShadowBias(f32 bias) { m_ShadowBias = bias; }
    f32 GetShadowBias() const { return m_ShadowBias; }

    void SetVolumetricScattering(f32 scattering) { m_VolumetricScattering = scattering; }
    f32 GetVolumetricScattering() const { return m_VolumetricScattering; }

    f32 GetAttenuationRadius() const { return m_Range; }
    f32 GetAttenuation(f32 distance) const;

    struct GPULight {
        GVec3 Position;
        f32 Range;
        GVec3 Direction;
        f32 Intensity;
        GColor Color;
        f32 InnerConeCos;
        f32 OuterConeCos;
        u32 Type;
        u32 CastShadows;
        u32 ShadowMapIndex;
        f32 Padding;
    };
    GPULight GetGPULight() const;

private:
    ELightType m_LightType = ELightType::Directional;
    GColor m_Color = GColor::White();
    f32 m_Intensity = 1.0f;
    f32 m_Range = 100.0f;
    f32 m_InnerConeAngle = 0.0f;
    f32 m_OuterConeAngle = 45.0f;
    bool m_CastShadows = true;
    u32 m_ShadowMapSize = 1024;
    f32 m_ShadowBias = 0.005f;
    f32 m_VolumetricScattering = 0.0f;
};

}
