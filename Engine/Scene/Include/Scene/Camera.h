#pragma once
#include <Core/Types.h>
#include <Core/Math.h>
#include <Scene/SceneNode.h>

namespace grom {

class Camera : public SceneNode {
public:
    Camera();
    explicit Camera(const GString& name);

    void SetPerspective(f32 fovDegrees, f32 aspectRatio, f32 nearPlane, f32 farPlane);
    void SetOrthographic(f32 width, f32 height, f32 nearPlane, f32 farPlane);

    void SetFOV(f32 fovDegrees);
    void SetAspectRatio(f32 aspect);
    void SetNearPlane(f32 nearPlane);
    void SetFarPlane(f32 farPlane);

    f32 GetFOV() const { return m_FOV; }
    f32 GetAspectRatio() const { return m_AspectRatio; }
    f32 GetNearPlane() const { return m_NearPlane; }
    f32 GetFarPlane() const { return m_FarPlane; }

    GMatrix4x4 GetViewMatrix() const;
    GMatrix4x4 GetProjectionMatrix() const;
    GMatrix4x4 GetViewProjectionMatrix() const;

    bool IsOrthographic() const { return m_Orthographic; }

    struct FrustumPlanes {
        GVec4 Planes[6];
    };
    FrustumPlanes GetFrustumPlanes() const;
    bool IsBoxVisible(const GVec3& center, const GVec3& extents) const;

    struct Ray {
        GVec3 Origin;
        GVec3 Direction;
    };
    Ray ScreenToRay(f32 screenX, f32 screenY) const;

    void SetViewportOffset(f32 x, f32 y) { m_ViewportOffsetX = x; m_ViewportOffsetY = y; }
    void SetViewportSize(f32 w, f32 h) { m_ViewportWidth = w; m_ViewportHeight = h; }

private:
    void RebuildProjection();

    f32 m_FOV = 60.0f;
    f32 m_AspectRatio = 16.0f / 9.0f;
    f32 m_NearPlane = 0.1f;
    f32 m_FarPlane = 1000.0f;
    f32 m_OrthoWidth = 10.0f;
    f32 m_OrthoHeight = 10.0f;
    f32 m_ViewportOffsetX = 0.0f;
    f32 m_ViewportOffsetY = 0.0f;
    f32 m_ViewportWidth = 1920.0f;
    f32 m_ViewportHeight = 1080.0f;
    bool m_Orthographic = false;
    bool m_ProjectionDirty = true;
    GMatrix4x4 m_ProjectionMatrix;
};

}
