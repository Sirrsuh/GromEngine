#include <Core/Math.h>

namespace grom
{

GMatrix4x4 GMatrix4x4::Inverse() const
{
    f32 A2323 = m[2][2] * m[3][3] - m[2][3] * m[3][2];
    f32 A1323 = m[2][1] * m[3][3] - m[2][3] * m[3][1];
    f32 A1223 = m[2][1] * m[3][2] - m[2][2] * m[3][1];
    f32 A0323 = m[2][0] * m[3][3] - m[2][3] * m[3][0];
    f32 A0223 = m[2][0] * m[3][2] - m[2][2] * m[3][0];
    f32 A0123 = m[2][0] * m[3][1] - m[2][1] * m[3][0];
    f32 A2313 = m[1][2] * m[3][3] - m[1][3] * m[3][2];
    f32 A1313 = m[1][1] * m[3][3] - m[1][3] * m[3][1];
    f32 A1213 = m[1][1] * m[3][2] - m[1][2] * m[3][1];
    f32 A2312 = m[1][2] * m[2][3] - m[1][3] * m[2][2];
    f32 A1312 = m[1][1] * m[2][3] - m[1][3] * m[2][1];
    f32 A1212 = m[1][1] * m[2][2] - m[1][2] * m[2][1];
    f32 A0313 = m[1][0] * m[3][3] - m[1][3] * m[3][0];
    f32 A0213 = m[1][0] * m[3][2] - m[1][2] * m[3][0];
    f32 A0312 = m[1][0] * m[2][3] - m[1][3] * m[2][0];
    f32 A0212 = m[1][0] * m[2][2] - m[1][2] * m[2][0];
    f32 A0113 = m[1][0] * m[3][1] - m[1][1] * m[3][0];
    f32 A0112 = m[1][0] * m[2][1] - m[1][1] * m[2][0];

    f32 det = m[0][0] * (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223)
            - m[0][1] * (m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223)
            + m[0][2] * (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123)
            - m[0][3] * (m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123);

    if (std::abs(det) < 1e-10f)
    {
        return Identity();
    }

    f32 invDet = 1.0f / det;

    GMatrix4x4 result;
    result.m[0][0] = (m[1][1] * A2323 - m[1][2] * A1323 + m[1][3] * A1223) * invDet;
    result.m[0][1] = -(m[0][1] * A2323 - m[0][2] * A1323 + m[0][3] * A1223) * invDet;
    result.m[0][2] = (m[0][1] * A2313 - m[0][2] * A1313 + m[0][3] * A1213) * invDet;
    result.m[0][3] = -(m[0][1] * A2312 - m[0][2] * A1312 + m[0][3] * A1212) * invDet;

    result.m[1][0] = -(m[1][0] * A2323 - m[1][2] * A0323 + m[1][3] * A0223) * invDet;
    result.m[1][1] = (m[0][0] * A2323 - m[0][2] * A0323 + m[0][3] * A0223) * invDet;
    result.m[1][2] = -(m[0][0] * A2313 - m[0][2] * A0313 + m[0][3] * A0213) * invDet;
    result.m[1][3] = (m[0][0] * A2312 - m[0][2] * A0312 + m[0][3] * A0212) * invDet;

    result.m[2][0] = (m[1][0] * A1323 - m[1][1] * A0323 + m[1][3] * A0123) * invDet;
    result.m[2][1] = -(m[0][0] * A1323 - m[0][1] * A0323 + m[0][3] * A0123) * invDet;
    result.m[2][2] = (m[0][0] * A1313 - m[0][1] * A0313 + m[0][3] * A0113) * invDet;
    result.m[2][3] = -(m[0][0] * A1312 - m[0][1] * A0312 + m[0][3] * A0112) * invDet;

    result.m[3][0] = -(m[1][0] * A1223 - m[1][1] * A0223 + m[1][2] * A0123) * invDet;
    result.m[3][1] = (m[0][0] * A1223 - m[0][1] * A0223 + m[0][2] * A0123) * invDet;
    result.m[3][2] = -(m[0][0] * A1213 - m[0][1] * A0213 + m[0][2] * A0113) * invDet;
    result.m[3][3] = (m[0][0] * A1212 - m[0][1] * A0212 + m[0][2] * A0112) * invDet;

    return result;
}

void GMatrix4x4::Decompose(GVec3& pos, GQuat& rot, GVec3& scale) const
{
    pos.x = m[3][0];
    pos.y = m[3][1];
    pos.z = m[3][2];

    scale.x = GVec3(m[0][0], m[1][0], m[2][0]).Length();
    scale.y = GVec3(m[0][1], m[1][1], m[2][1]).Length();
    scale.z = GVec3(m[0][2], m[1][2], m[2][2]).Length();

    GMatrix4x4 rotMat;
    for (u32 col = 0; col < 3; ++col)
    {
        for (u32 row = 0; row < 3; ++row)
        {
            rotMat.m[row][col] = m[row][col] / (col == 0 ? scale.x : (col == 1 ? scale.y : scale.z));
        }
    }
    rotMat.m[0][3] = 0.0f;
    rotMat.m[1][3] = 0.0f;
    rotMat.m[2][3] = 0.0f;
    rotMat.m[3][0] = 0.0f;
    rotMat.m[3][1] = 0.0f;
    rotMat.m[3][2] = 0.0f;
    rotMat.m[3][3] = 1.0f;

    f32 tr = rotMat.m[0][0] + rotMat.m[1][1] + rotMat.m[2][2];
    if (tr > 0.0f)
    {
        f32 s = std::sqrt(tr + 1.0f) * 2.0f;
        rot.w = 0.25f * s;
        rot.x = (rotMat.m[1][2] - rotMat.m[2][1]) / s;
        rot.y = (rotMat.m[2][0] - rotMat.m[0][2]) / s;
        rot.z = (rotMat.m[0][1] - rotMat.m[1][0]) / s;
    }
    else if (rotMat.m[0][0] > rotMat.m[1][1] && rotMat.m[0][0] > rotMat.m[2][2])
    {
        f32 s = std::sqrt(1.0f + rotMat.m[0][0] - rotMat.m[1][1] - rotMat.m[2][2]) * 2.0f;
        rot.w = (rotMat.m[1][2] - rotMat.m[2][1]) / s;
        rot.x = 0.25f * s;
        rot.y = (rotMat.m[0][1] + rotMat.m[1][0]) / s;
        rot.z = (rotMat.m[0][2] + rotMat.m[2][0]) / s;
    }
    else if (rotMat.m[1][1] > rotMat.m[2][2])
    {
        f32 s = std::sqrt(1.0f + rotMat.m[1][1] - rotMat.m[0][0] - rotMat.m[2][2]) * 2.0f;
        rot.w = (rotMat.m[2][0] - rotMat.m[0][2]) / s;
        rot.x = (rotMat.m[0][1] + rotMat.m[1][0]) / s;
        rot.y = 0.25f * s;
        rot.z = (rotMat.m[1][2] + rotMat.m[2][1]) / s;
    }
    else
    {
        f32 s = std::sqrt(1.0f + rotMat.m[2][2] - rotMat.m[0][0] - rotMat.m[1][1]) * 2.0f;
        rot.w = (rotMat.m[0][1] - rotMat.m[1][0]) / s;
        rot.x = (rotMat.m[0][2] + rotMat.m[2][0]) / s;
        rot.y = (rotMat.m[1][2] + rotMat.m[2][1]) / s;
        rot.z = 0.25f * s;
    }
}

}
