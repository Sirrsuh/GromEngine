#include <Core/Container.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cstdlib>

namespace grom
{

GString::GString()
    : m_length(0)
    , m_capacity(SSO_SIZE)
    , m_isLong(false)
{
    m_ssoBuffer[0] = '\0';
}

GString::GString(const char* str)
    : m_length(0)
    , m_capacity(SSO_SIZE)
    , m_isLong(false)
{
    m_ssoBuffer[0] = '\0';
    if (str)
    {
        usize len = std::strlen(str);
        SetFromCStr(str, len);
    }
}

GString::GString(const char* str, usize len)
    : m_length(0)
    , m_capacity(SSO_SIZE)
    , m_isLong(false)
{
    m_ssoBuffer[0] = '\0';
    if (str && len > 0)
    {
        SetFromCStr(str, len);
    }
}

GString::GString(const GString& other)
    : m_length(0)
    , m_capacity(SSO_SIZE)
    , m_isLong(false)
{
    m_ssoBuffer[0] = '\0';
    if (other.m_length > 0)
    {
        SetFromCStr(other.c_str(), other.m_length);
    }
}

GString::GString(GString&& other) noexcept
    : m_length(other.m_length)
    , m_capacity(other.m_capacity)
    , m_isLong(other.m_isLong)
{
    if (m_isLong)
    {
        m_longData = other.m_longData;
        other.m_longData = nullptr;
    }
    else
    {
        std::memcpy(m_ssoBuffer, other.m_ssoBuffer, SSO_SIZE);
    }

    other.m_length = 0;
    other.m_capacity = SSO_SIZE;
    other.m_isLong = false;
    other.m_ssoBuffer[0] = '\0';
}

GString::~GString()
{
    if (m_isLong && m_longData)
    {
        free(m_longData);
    }
}

GString& GString::operator=(const char* str)
{
    Clear();
    if (str)
    {
        usize len = std::strlen(str);
        SetFromCStr(str, len);
    }
    return *this;
}

GString& GString::operator=(const GString& other)
{
    if (this != &other)
    {
        Clear();
        if (other.m_length > 0)
        {
            SetFromCStr(other.c_str(), other.m_length);
        }
    }
    return *this;
}

GString& GString::operator=(GString&& other) noexcept
{
    if (this != &other)
    {
        if (m_isLong && m_longData)
        {
            free(m_longData);
        }

        m_length = other.m_length;
        m_capacity = other.m_capacity;
        m_isLong = other.m_isLong;

        if (m_isLong)
        {
            m_longData = other.m_longData;
            other.m_longData = nullptr;
        }
        else
        {
            std::memcpy(m_ssoBuffer, other.m_ssoBuffer, SSO_SIZE);
        }

        other.m_length = 0;
        other.m_capacity = SSO_SIZE;
        other.m_isLong = false;
        other.m_ssoBuffer[0] = '\0';
    }
    return *this;
}

bool GString::operator==(const GString& other) const
{
    if (m_length != other.m_length) return false;
    return std::memcmp(c_str(), other.c_str(), m_length) == 0;
}

bool GString::operator==(const char* str) const
{
    if (!str) return m_length == 0;
    usize len = std::strlen(str);
    if (m_length != len) return false;
    return std::memcmp(c_str(), str, m_length) == 0;
}

bool GString::operator!=(const GString& other) const
{
    return !(*this == other);
}

bool GString::operator!=(const char* str) const
{
    return !(*this == str);
}

GString GString::operator+(const GString& other) const
{
    GString result(*this);
    result.Append(other);
    return result;
}

GString GString::operator+(const char* str) const
{
    GString result(*this);
    result.Append(str);
    return result;
}

GString& GString::operator+=(const GString& other)
{
    Append(other);
    return *this;
}

GString& GString::operator+=(const char* str)
{
    Append(str);
    return *this;
}

const char* GString::c_str() const
{
    return m_isLong ? m_longData : m_ssoBuffer;
}

usize GString::Len() const
{
    return m_length;
}

GString& GString::Append(const char* str)
{
    if (!str) return *this;
    usize appendLen = std::strlen(str);
    if (appendLen == 0) return *this;

    usize required = m_length + appendLen + 1;
    if (required > m_capacity)
    {
        Grow(required);
    }

    char* buffer = const_cast<char*>(c_str());
    std::memcpy(buffer + m_length, str, appendLen);
    m_length += appendLen;
    buffer[m_length] = '\0';
    return *this;
}

GString& GString::Append(const GString& str)
{
    return Append(str.c_str());
}

GString& GString::Append(char c)
{
    usize required = m_length + 2;
    if (required > m_capacity)
    {
        Grow(required);
    }

    char* buffer = const_cast<char*>(c_str());
    buffer[m_length] = c;
    m_length += 1;
    buffer[m_length] = '\0';
    return *this;
}

GString GString::Format(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    GString result = VFormat(fmt, args);
    va_end(args);
    return result;
}

GString GString::VFormat(const char* fmt, va_list args)
{
    va_list argsCopy;
    va_copy(argsCopy, args);
    int len = std::vsnprintf(nullptr, 0, fmt, args);
    va_end(argsCopy);

    if (len <= 0) return GString();

    GString result;
    usize required = static_cast<usize>(len) + 1;
    if (required > result.m_capacity)
    {
        result.Grow(required);
    }

    std::vsnprintf(const_cast<char*>(result.c_str()), required, fmt, args);
    result.m_length = static_cast<usize>(len);
    return result;
}

TArray<GString> GString::Split(char delimiter) const
{
    TArray<GString> result;
    const char* str = c_str();
    usize start = 0;

    for (usize i = 0; i <= m_length; ++i)
    {
        if (i == m_length || str[i] == delimiter)
        {
            usize count = i - start;
            if (count > 0)
            {
                GString part(str + start, count);
                result.Add(static_cast<GString&&>(part));
            }
            start = i + 1;
        }
    }

    return result;
}

GString GString::SubStr(usize offset, usize count) const
{
    if (offset >= m_length) return GString();
    if (offset + count > m_length) count = m_length - offset;
    return GString(c_str() + offset, count);
}

i32 GString::Find(const char* str, usize startPos) const
{
    if (!str || !str[0] || startPos >= m_length) return -1;

    const char* src = c_str();
    usize searchLen = std::strlen(str);

    for (usize i = startPos; i <= m_length - searchLen; ++i)
    {
        if (std::memcmp(src + i, str, searchLen) == 0)
        {
            return static_cast<i32>(i);
        }
    }
    return -1;
}

GString GString::Replace(const char* from, const char* to) const
{
    if (!from || !from[0]) return *this;

    GString result;
    const char* src = c_str();
    usize fromLen = std::strlen(from);
    usize toLen = to ? std::strlen(to) : 0;
    GROM_UNUSED(toLen);

    usize lastPos = 0;
    i32 pos = Find(from, 0);

    while (pos >= 0)
    {
        result.Append(GString(src + lastPos, static_cast<usize>(pos) - lastPos));
        if (to) result.Append(to);
        lastPos = static_cast<usize>(pos) + fromLen;
        pos = Find(from, lastPos);
    }

    result.Append(GString(src + lastPos, m_length - lastPos));
    return result;
}

void GString::Clear()
{
    if (m_isLong && m_longData)
    {
        free(m_longData);
        m_longData = nullptr;
    }
    m_length = 0;
    m_capacity = SSO_SIZE;
    m_isLong = false;
    m_ssoBuffer[0] = '\0';
}

bool GString::IsEmpty() const
{
    return m_length == 0;
}

void GString::Grow(usize required)
{
    usize newCapacity = m_capacity;
    while (newCapacity < required)
    {
        newCapacity *= 2;
    }

    char* newBuffer = static_cast<char*>(malloc(newCapacity));
    std::memcpy(newBuffer, c_str(), m_length + 1);

    if (m_isLong && m_longData)
    {
        free(m_longData);
    }

    m_longData = newBuffer;
    m_capacity = newCapacity;
    m_isLong = true;
}

void GString::SetFromCStr(const char* str, usize len)
{
    if (len == 0) return;

    usize required = len + 1;
    if (required > m_capacity)
    {
        Grow(required);
    }

    char* buffer = const_cast<char*>(c_str());
    std::memcpy(buffer, str, len);
    m_length = len;
    buffer[m_length] = '\0';
}

}
