#pragma once

#include <Core/Types.h>
#include <cmath>

namespace grom
{

struct GVec2
{
    f32 x = 0.0f;
    f32 y = 0.0f;

    GVec2() = default;
    GVec2(f32 x, f32 y) : x(x), y(y) {}

    GVec2 operator+(const GVec2& v) const { return { x + v.x, y + v.y }; }
    GVec2 operator-(const GVec2& v) const { return { x - v.x, y - v.y }; }
    GVec2 operator*(f32 s) const { return { x * s, y * s }; }
    GVec2 operator/(f32 s) const { return { x / s, y / s }; }
    GVec2 operator-() const { return { -x, -y }; }

    GVec2& operator+=(const GVec2& v) { x += v.x; y += v.y; return *this; }
    GVec2& operator-=(const GVec2& v) { x -= v.x; y -= v.y; return *this; }
    GVec2& operator*=(f32 s) { x *= s; y *= s; return *this; }
    GVec2& operator/=(f32 s) { x /= s; y /= s; return *this; }

    bool operator==(const GVec2& v) const { return x == v.x && y == v.y; }
    bool operator!=(const GVec2& v) const { return !(*this == v); }

    f32 Length() const { return std::sqrt(x * x + y * y); }
    f32 LengthSq() const { return x * x + y * y; }

    GVec2 Normalized() const
    {
        f32 len = Length();
        if (len > 0.0f) return { x / len, y / len };
        return { 0.0f, 0.0f };
    }

    f32 Dot(const GVec2& v) const { return x * v.x + y * v.y; }
};

struct GVec3
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;

    GVec3() = default;
    GVec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}
    GVec3(const GVec2& v, f32 z) : x(v.x), y(v.y), z(z) {}

    GVec3 operator+(const GVec3& v) const { return { x + v.x, y + v.y, z + v.z }; }
    GVec3 operator-(const GVec3& v) const { return { x - v.x, y - v.y, z - v.z }; }
    GVec3 operator*(f32 s) const { return { x * s, y * s, z * s }; }
    GVec3 operator/(f32 s) const { return { x / s, y / s, z / s }; }
    GVec3 operator-() const { return { -x, -y, -z }; }

    GVec3& operator+=(const GVec3& v) { x += v.x; y += v.y; z += v.z; return *this; }
    GVec3& operator-=(const GVec3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    GVec3& operator*=(f32 s) { x *= s; y *= s; z *= s; return *this; }
    GVec3& operator/=(f32 s) { x /= s; y /= s; z /= s; return *this; }

    bool operator==(const GVec3& v) const { return x == v.x && y == v.y && z == v.z; }
    bool operator!=(const GVec3& v) const { return !(*this == v); }

    f32 Length() const { return std::sqrt(x * x + y * y + z * z); }
    f32 LengthSq() const { return x * x + y * y + z * z; }

    GVec3 Normalized() const
    {
        f32 len = Length();
        if (len > 0.0f) return { x / len, y / len, z / len };
        return { 0.0f, 0.0f, 0.0f };
    }

    f32 Dot(const GVec3& v) const { return x * v.x + y * v.y + z * v.z; }

    GVec3 Cross(const GVec3& v) const
    {
        return {
            y * v.z - z * v.y,
            z * v.x - x * v.z,
            x * v.y - y * v.x
        };
    }

    GVec2 XZ() const { return { x, z }; }
    GVec2 XY() const { return { x, y }; }
};

struct GVec4
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 0.0f;

    GVec4() = default;
    GVec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    GVec4(const GVec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}

    GVec4 operator+(const GVec4& v) const { return { x + v.x, y + v.y, z + v.z, w + v.w }; }
    GVec4 operator-(const GVec4& v) const { return { x - v.x, y - v.y, z - v.z, w - v.w }; }
    GVec4 operator*(f32 s) const { return { x * s, y * s, z * s, w * s }; }

    GVec4& operator+=(const GVec4& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
    GVec4& operator-=(const GVec4& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
    GVec4& operator*=(f32 s) { x *= s; y *= s; z *= s; w *= s; return *this; }

    bool operator==(const GVec4& v) const { return x == v.x && y == v.y && z == v.z && w == v.w; }
    bool operator!=(const GVec4& v) const { return !(*this == v); }

    GVec3 XYZ() const { return { x, y, z }; }
};

struct GMatrix4x4
{
    f32 m[4][4];

    GMatrix4x4()
    {
        std::memset(m, 0, sizeof(m));
    }

    explicit GMatrix4x4(f32 diag)
    {
        std::memset(m, 0, sizeof(m));
        m[0][0] = diag;
        m[1][1] = diag;
        m[2][2] = diag;
        m[3][3] = diag;
    }

    f32* operator[](u32 row) { return m[row]; }
    const f32* operator[](u32 row) const { return m[row]; }

    f32& At(u32 row, u32 col) { return m[row][col]; }
    f32 At(u32 row, u32 col) const { return m[row][col]; }

    static GMatrix4x4 Identity()
    {
        return GMatrix4x4(1.0f);
    }

    static GMatrix4x4 Translation(f32 x, f32 y, f32 z)
    {
        GMatrix4x4 mat = Identity();
        mat.m[3][0] = x;
        mat.m[3][1] = y;
        mat.m[3][2] = z;
        return mat;
    }

    static GMatrix4x4 Translation(const GVec3& t)
    {
        return Translation(t.x, t.y, t.z);
    }

    static GMatrix4x4 RotationX(f32 angle)
    {
        f32 c = std::cos(angle);
        f32 s = std::sin(angle);
        GMatrix4x4 mat = Identity();
        mat.m[1][1] = c;
        mat.m[1][2] = s;
        mat.m[2][1] = -s;
        mat.m[2][2] = c;
        return mat;
    }

    static GMatrix4x4 RotationY(f32 angle)
    {
        f32 c = std::cos(angle);
        f32 s = std::sin(angle);
        GMatrix4x4 mat = Identity();
        mat.m[0][0] = c;
        mat.m[0][2] = -s;
        mat.m[2][0] = s;
        mat.m[2][2] = c;
        return mat;
    }

    static GMatrix4x4 RotationZ(f32 angle)
    {
        f32 c = std::cos(angle);
        f32 s = std::sin(angle);
        GMatrix4x4 mat = Identity();
        mat.m[0][0] = c;
        mat.m[0][1] = s;
        mat.m[1][0] = -s;
        mat.m[1][1] = c;
        return mat;
    }

    static GMatrix4x4 Scale(f32 x, f32 y, f32 z)
    {
        GMatrix4x4 mat = Identity();
        mat.m[0][0] = x;
        mat.m[1][1] = y;
        mat.m[2][2] = z;
        return mat;
    }

    static GMatrix4x4 Scale(const GVec3& s)
    {
        return Scale(s.x, s.y, s.z);
    }

    static GMatrix4x4 Perspective(f32 fovY, f32 aspect, f32 zNear, f32 zFar)
    {
        GMatrix4x4 mat = {};
        f32 tanHalfFov = std::tan(fovY / 2.0f);
        mat.m[0][0] = 1.0f / (aspect * tanHalfFov);
        mat.m[1][1] = 1.0f / tanHalfFov;
        mat.m[2][2] = -(zFar + zNear) / (zFar - zNear);
        mat.m[2][3] = -1.0f;
        mat.m[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
        return mat;
    }

    static GMatrix4x4 LookAt(const GVec3& eye, const GVec3& target, const GVec3& up)
    {
        GVec3 f = (target - eye).Normalized();
        GVec3 r = f.Cross(up).Normalized();
        GVec3 u = r.Cross(f);

        GMatrix4x4 mat = Identity();
        mat.m[0][0] = r.x;
        mat.m[1][0] = r.y;
        mat.m[2][0] = r.z;
        mat.m[0][1] = u.x;
        mat.m[1][1] = u.y;
        mat.m[2][1] = u.z;
        mat.m[0][2] = -f.x;
        mat.m[1][2] = -f.y;
        mat.m[2][2] = -f.z;
        mat.m[3][0] = -r.Dot(eye);
        mat.m[3][1] = -u.Dot(eye);
        mat.m[3][2] = f.Dot(eye);
        return mat;
    }

    GMatrix4x4 Multiply(const GMatrix4x4& other) const
    {
        GMatrix4x4 result = {};
        for (u32 row = 0; row < 4; ++row)
        {
            for (u32 col = 0; col < 4; ++col)
            {
                f32 sum = 0.0f;
                for (u32 k = 0; k < 4; ++k)
                {
                    sum += m[row][k] * other.m[k][col];
                }
                result.m[row][col] = sum;
            }
        }
        return result;
    }

    GMatrix4x4 Transpose() const
    {
        GMatrix4x4 result = {};
        for (u32 row = 0; row < 4; ++row)
        {
            for (u32 col = 0; col < 4; ++col)
            {
                result.m[row][col] = m[col][row];
            }
        }
        return result;
    }

    GMatrix4x4 Inverse() const;

    GMatrix4x4 operator*(const GMatrix4x4& other) const
    {
        return Multiply(other);
    }

    GVec4 operator*(const GVec4& v) const
    {
        return {
            m[0][0] * v.x + m[0][1] * v.y + m[0][2] * v.z + m[0][3] * v.w,
            m[1][0] * v.x + m[1][1] * v.y + m[1][2] * v.z + m[1][3] * v.w,
            m[2][0] * v.x + m[2][1] * v.y + m[2][2] * v.z + m[2][3] * v.w,
            m[3][0] * v.x + m[3][1] * v.y + m[3][2] * v.z + m[3][3] * v.w
        };
    }

    GVec3 TransformPoint(const GVec3& p) const
    {
        f32 w = m[3][0] * p.x + m[3][1] * p.y + m[3][2] * p.z + m[3][3];
        return {
            (m[0][0] * p.x + m[0][1] * p.y + m[0][2] * p.z + m[0][3]) / w,
            (m[1][0] * p.x + m[1][1] * p.y + m[1][2] * p.z + m[1][3]) / w,
            (m[2][0] * p.x + m[2][1] * p.y + m[2][2] * p.z + m[2][3]) / w
        };
    }

    GVec3 TransformDirection(const GVec3& d) const
    {
        return {
            m[0][0] * d.x + m[0][1] * d.y + m[0][2] * d.z,
            m[1][0] * d.x + m[1][1] * d.y + m[1][2] * d.z,
            m[2][0] * d.x + m[2][1] * d.y + m[2][2] * d.z
        };
    }
};

struct GQuat
{
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 z = 0.0f;
    f32 w = 1.0f;

    GQuat() = default;
    GQuat(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}

    static GQuat Identity() { return { 0.0f, 0.0f, 0.0f, 1.0f }; }

    static GQuat FromAxisAngle(const GVec3& axis, f32 angle)
    {
        f32 halfAngle = angle * 0.5f;
        f32 s = std::sin(halfAngle);
        GVec3 n = axis.Normalized();
        return { n.x * s, n.y * s, n.z * s, std::cos(halfAngle) };
    }

    static GQuat FromEuler(f32 pitch, f32 yaw, f32 roll)
    {
        f32 cp = std::cos(pitch * 0.5f);
        f32 sp = std::sin(pitch * 0.5f);
        f32 cy = std::cos(yaw * 0.5f);
        f32 sy = std::sin(yaw * 0.5f);
        f32 cr = std::cos(roll * 0.5f);
        f32 sr = std::sin(roll * 0.5f);

        return {
            sr * cp * cy - cr * sp * sy,
            cr * sp * cy + sr * cp * sy,
            cr * cp * sy - sr * sp * cy,
            cr * cp * cy + sr * sp * sy
        };
    }

    GQuat operator*(const GQuat& q) const
    {
        return {
            w * q.x + x * q.w + y * q.z - z * q.y,
            w * q.y - x * q.z + y * q.w + z * q.x,
            w * q.z + x * q.y - y * q.x + z * q.w,
            w * q.w - x * q.x - y * q.y - z * q.z
        };
    }

    GQuat& operator*=(const GQuat& q)
    {
        *this = *this * q;
        return *this;
    }

    GQuat Conjugate() const { return { -x, -y, -z, w }; }

    GQuat Inverse() const
    {
        f32 lenSq = x * x + y * y + z * z + w * w;
        if (lenSq > 0.0f)
        {
            f32 invLen = 1.0f / lenSq;
            return { -x * invLen, -y * invLen, -z * invLen, w * invLen };
        }
        return Identity();
    }

    f32 Length() const { return std::sqrt(x * x + y * y + z * z + w * w); }

    GQuat Normalized() const
    {
        f32 len = Length();
        if (len > 0.0f)
        {
            f32 invLen = 1.0f / len;
            return { x * invLen, y * invLen, z * invLen, w * invLen };
        }
        return Identity();
    }

    GVec3 RotateVector(const GVec3& v) const
    {
        GQuat vQuat = { v.x, v.y, v.z, 0.0f };
        GQuat result = (*this) * vQuat * Conjugate();
        return { result.x, result.y, result.z };
    }

    f32 Dot(const GQuat& q) const { return x * q.x + y * q.y + z * q.z + w * q.w; }

    static GQuat Slerp(const GQuat& a, const GQuat& b, f32 t)
    {
        f32 dot = a.Dot(b);

        GQuat end = b;
        if (dot < 0.0f)
        {
            dot = -dot;
            end = { -b.x, -b.y, -b.z, -b.w };
        }

        if (dot > 0.9995f)
        {
            GQuat result = {
                a.x + t * (end.x - a.x),
                a.y + t * (end.y - a.y),
                a.z + t * (end.z - a.z),
                a.w + t * (end.w - a.w)
            };
            return result.Normalized();
        }

        f32 theta0 = std::acos(dot);
        f32 theta = theta0 * t;
        f32 sinTheta = std::sin(theta);
        f32 sinTheta0 = std::sin(theta0);

        f32 s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
        f32 s1 = sinTheta / sinTheta0;

        return {
            s0 * a.x + s1 * end.x,
            s0 * a.y + s1 * end.y,
            s0 * a.z + s1 * end.z,
            s0 * a.w + s1 * end.w
        };
    }

    GMatrix4x4 ToMatrix() const
    {
        GMatrix4x4 mat = GMatrix4x4::Identity();

        f32 xx = x * x;
        f32 yy = y * y;
        f32 zz = z * z;
        f32 xy = x * y;
        f32 xz = x * z;
        f32 yz = y * z;
        f32 wx = w * x;
        f32 wy = w * y;
        f32 wz = w * z;

        mat.m[0][0] = 1.0f - 2.0f * (yy + zz);
        mat.m[0][1] = 2.0f * (xy + wz);
        mat.m[0][2] = 2.0f * (xz - wy);
        mat.m[1][0] = 2.0f * (xy - wz);
        mat.m[1][1] = 1.0f - 2.0f * (xx + zz);
        mat.m[1][2] = 2.0f * (yz + wx);
        mat.m[2][0] = 2.0f * (xz + wy);
        mat.m[2][1] = 2.0f * (yz - wx);
        mat.m[2][2] = 1.0f - 2.0f * (xx + yy);

        return mat;
    }
};

struct GTransform
{
    GVec3 Translation = { 0.0f, 0.0f, 0.0f };
    GQuat Rotation = GQuat::Identity();
    GVec3 Scale = { 1.0f, 1.0f, 1.0f };

    GTransform() = default;
    GTransform(const GVec3& t, const GQuat& r, const GVec3& s)
        : Translation(t), Rotation(r), Scale(s) {}

    GMatrix4x4 ToMatrix() const
    {
        GMatrix4x4 rotMat = Rotation.ToMatrix();
        GMatrix4x4 scaleMat = GMatrix4x4::Scale(Scale);
        GMatrix4x4 transMat = GMatrix4x4::Translation(Translation);
        return scaleMat * rotMat * transMat;
    }
};

struct GColor
{
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    GColor() = default;
    GColor(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}

    static GColor FromFloats(f32 r, f32 g, f32 b, f32 a = 1.0f)
    {
        return {
            static_cast<u8>(r * 255.0f),
            static_cast<u8>(g * 255.0f),
            static_cast<u8>(b * 255.0f),
            static_cast<u8>(a * 255.0f)
        };
    }

    u32 ToU32() const
    {
        return (static_cast<u32>(a) << 24) |
               (static_cast<u32>(b) << 16) |
               (static_cast<u32>(g) << 8) |
               static_cast<u32>(r);
    }

    bool operator==(const GColor& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
    bool operator!=(const GColor& c) const { return !(*this == c); }

    static GColor FromU32(u32 color)
    {
        return {
            static_cast<u8>(color & 0xFF),
            static_cast<u8>((color >> 8) & 0xFF),
            static_cast<u8>((color >> 16) & 0xFF),
            static_cast<u8>((color >> 24) & 0xFF)
        };
    }

    static GColor Red() { return { 255, 0, 0, 255 }; }
    static GColor Green() { return { 0, 255, 0, 255 }; }
    static GColor Blue() { return { 0, 0, 255, 255 }; }
    static GColor White() { return { 255, 255, 255, 255 }; }
    static GColor Black() { return { 0, 0, 0, 255 }; }
    static GColor Transparent() { return { 0, 0, 0, 0 }; }
};

}
