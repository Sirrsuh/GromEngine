#include <Scene/Camera.h>

namespace grom {

Camera::Camera() : SceneNode("Camera")
{
    SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
}

Camera::Camera(const GString& name) : SceneNode(name)
{
    SetPerspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
}

void Camera::SetPerspective(f32 fovDegrees, f32 aspectRatio, f32 nearPlane, f32 farPlane)
{
    m_FOV = fovDegrees;
    m_AspectRatio = aspectRatio;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_Orthographic = false;
    m_ProjectionDirty = true;
}

void Camera::SetOrthographic(f32 width, f32 height, f32 nearPlane, f32 farPlane)
{
    m_OrthoWidth = width;
    m_OrthoHeight = height;
    m_NearPlane = nearPlane;
    m_FarPlane = farPlane;
    m_Orthographic = true;
    m_ProjectionDirty = true;
}

void Camera::SetFOV(f32 fovDegrees)
{
    m_FOV = fovDegrees;
    m_ProjectionDirty = true;
}

void Camera::SetAspectRatio(f32 aspect)
{
    m_AspectRatio = aspect;
    m_ProjectionDirty = true;
}

void Camera::SetNearPlane(f32 nearPlane)
{
    m_NearPlane = nearPlane;
    m_ProjectionDirty = true;
}

void Camera::SetFarPlane(f32 farPlane)
{
    m_FarPlane = farPlane;
    m_ProjectionDirty = true;
}

void Camera::RebuildProjection()
{
    if (m_Orthographic)
    {
        m_ProjectionMatrix = GMatrix4x4::Orthographic(m_OrthoWidth, m_OrthoHeight, m_NearPlane, m_FarPlane);
    }
    else
    {
        m_ProjectionMatrix = GMatrix4x4::Perspective(m_FOV, m_AspectRatio, m_NearPlane, m_FarPlane);
    }
    m_ProjectionDirty = false;
}

GMatrix4x4 Camera::GetViewMatrix() const
{
    GMatrix4x4 world = GetWorldMatrix();
    return world.Inverse();
}

GMatrix4x4 Camera::GetProjectionMatrix() const
{
    if (m_ProjectionDirty)
    {
        const_cast<Camera*>(this)->RebuildProjection();
    }
    return m_ProjectionMatrix;
}

GMatrix4x4 Camera::GetViewProjectionMatrix() const
{
    GMatrix4x4 view = GetViewMatrix();
    GMatrix4x4 proj = GetProjectionMatrix();
    return view * proj;
}

Camera::FrustumPlanes Camera::GetFrustumPlanes() const
{
    FrustumPlanes fp;
    GMatrix4x4 vp = GetViewProjectionMatrix();
    GMatrix4x4 t = vp.Transpose();

    fp.Planes[0] = t.GetRow(3) + t.GetRow(0);
    fp.Planes[1] = t.GetRow(3) - t.GetRow(0);
    fp.Planes[2] = t.GetRow(3) + t.GetRow(1);
    fp.Planes[3] = t.GetRow(3) - t.GetRow(1);
    fp.Planes[4] = t.GetRow(3) + t.GetRow(2);
    fp.Planes[5] = t.GetRow(3) - t.GetRow(2);

    for (u32 i = 0; i < 6; ++i)
    {
        f32 len = GVec3(fp.Planes[i].x, fp.Planes[i].y, fp.Planes[i].z).Length();
        if (len > 0.0f)
        {
            fp.Planes[i] = fp.Planes[i] * (1.0f / len);
        }
    }
    return fp;
}

bool Camera::IsBoxVisible(const GVec3& center, const GVec3& extents) const
{
    FrustumPlanes fp = GetFrustumPlanes();
    for (u32 i = 0; i < 6; ++i)
    {
        const GVec4& p = fp.Planes[i];
        f32 d = p.x * center.x + p.y * center.y + p.z * center.z + p.w;
        f32 r = extents.x * (p.x > 0.0f ? p.x : -p.x) +
                extents.y * (p.y > 0.0f ? p.y : -p.y) +
                extents.z * (p.z > 0.0f ? p.z : -p.z);
        if (d + r < 0.0f) return false;
    }
    return true;
}

Camera::Ray Camera::ScreenToRay(f32 screenX, f32 screenY) const
{
    Ray ray;

    f32 ndcX = (2.0f * (screenX - m_ViewportOffsetX) / m_ViewportWidth) - 1.0f;
    f32 ndcY = 1.0f - (2.0f * (screenY - m_ViewportOffsetY) / m_ViewportHeight);

    GMatrix4x4 invVP = GetViewProjectionMatrix().Inverse();

    GVec4 nearPoint(ndcX, ndcY, 0.0f, 1.0f);
    GVec4 farPoint(ndcX, ndcY, 1.0f, 1.0f);

    GVec4 nearWorld = invVP * nearPoint;
    GVec4 farWorld = invVP * farPoint;

    ray.Origin = GVec3(nearWorld.x / nearWorld.w, nearWorld.y / nearWorld.w, nearWorld.z / nearWorld.w);
    GVec3 farPos = GVec3(farWorld.x / farWorld.w, farWorld.y / farWorld.w, farWorld.z / farWorld.w);
    ray.Direction = (farPos - ray.Origin).Normalized();

    return ray;
}

}
