#pragma once

#include <Core/Types.h>
#include <Core/Memory.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <new>

namespace grom
{

template<typename T>
class TArray
{
public:
    TArray()
        : m_data(nullptr)
        , m_size(0)
        , m_capacity(0)
    {
    }

    explicit TArray(usize count)
        : m_data(nullptr)
        , m_size(0)
        , m_capacity(0)
    {
        Reserve(count);
    }

    TArray(const T* items, usize count)
        : m_data(nullptr)
        , m_size(0)
        , m_capacity(0)
    {
        Reserve(count);
        for (usize i = 0; i < count; ++i)
        {
            new(m_data + i) T(items[i]);
        }
        m_size = count;
    }

    TArray(const TArray& other)
        : m_data(nullptr)
        , m_size(0)
        , m_capacity(0)
    {
        Reserve(other.m_size);
        for (usize i = 0; i < other.m_size; ++i)
        {
            new(m_data + i) T(other.m_data[i]);
        }
        m_size = other.m_size;
    }

    TArray(TArray&& other) noexcept
        : m_data(other.m_data)
        , m_size(other.m_size)
        , m_capacity(other.m_capacity)
    {
        other.m_data = nullptr;
        other.m_size = 0;
        other.m_capacity = 0;
    }

    ~TArray()
    {
        Clear();
        if (m_data)
        {
            free(m_data);
        }
    }

    TArray& operator=(const TArray& other)
    {
        if (this != &other)
        {
            Clear();
            Reserve(other.m_size);
            for (usize i = 0; i < other.m_size; ++i)
            {
                new(m_data + i) T(other.m_data[i]);
            }
            m_size = other.m_size;
        }
        return *this;
    }

    TArray& operator=(TArray&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            if (m_data)
            {
                free(m_data);
            }
            m_data = other.m_data;
            m_size = other.m_size;
            m_capacity = other.m_capacity;
            other.m_data = nullptr;
            other.m_size = 0;
            other.m_capacity = 0;
        }
        return *this;
    }

    T& operator[](usize index) { return m_data[index]; }
    const T& operator[](usize index) const { return m_data[index]; }

    T* Data() { return m_data; }
    const T* Data() const { return m_data; }
    usize Size() const { return m_size; }
    usize Capacity() const { return m_capacity; }
    bool IsEmpty() const { return m_size == 0; }

    void Reserve(usize newCapacity)
    {
        if (newCapacity <= m_capacity) return;

        usize allocSize = newCapacity * sizeof(T);
        T* newData = static_cast<T*>(malloc(allocSize));
        std::memset(newData, 0, allocSize);

        for (usize i = 0; i < m_size; ++i)
        {
            new(newData + i) T(static_cast<T&&>(m_data[i]));
            m_data[i].~T();
        }

        if (m_data)
        {
            free(m_data);
        }

        m_data = newData;
        m_capacity = newCapacity;
    }

    T& Add(const T& value)
    {
        if (m_size >= m_capacity)
        {
            Reserve(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        new(m_data + m_size) T(value);
        return m_data[m_size++];
    }

    T& Add(T&& value)
    {
        if (m_size >= m_capacity)
        {
            Reserve(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        new(m_data + m_size) T(static_cast<T&&>(value));
        return m_data[m_size++];
    }

    template<typename... Args>
    T& Emplace(Args&&... args)
    {
        if (m_size >= m_capacity)
        {
            Reserve(m_capacity == 0 ? 1 : m_capacity * 2);
        }
        new(m_data + m_size) T(static_cast<Args&&>(args)...);
        return m_data[m_size++];
    }

    void RemoveAt(usize index)
    {
        if (index >= m_size) return;
        m_data[index].~T();
        for (usize i = index; i < m_size - 1; ++i)
        {
            new(m_data + i) T(static_cast<T&&>(m_data[i + 1]));
            m_data[i + 1].~T();
        }
        --m_size;
    }

    void RemoveSwap(usize index)
    {
        if (index >= m_size) return;
        m_data[index].~T();
        if (index < m_size - 1)
        {
            new(m_data + index) T(static_cast<T&&>(m_data[m_size - 1]));
            m_data[m_size - 1].~T();
        }
        --m_size;
    }

    void Clear()
    {
        for (usize i = 0; i < m_size; ++i)
        {
            m_data[i].~T();
        }
        m_size = 0;
    }

    T* Begin() { return m_data; }
    T* End() { return m_data + m_size; }
    const T* Begin() const { return m_data; }
    const T* End() const { return m_data + m_size; }

    T& Last() { return m_data[m_size - 1]; }
    const T& Last() const { return m_data[m_size - 1]; }

    bool Contains(const T& value) const
    {
        for (usize i = 0; i < m_size; ++i)
        {
            if (m_data[i] == value) return true;
        }
        return false;
    }

    i32 Find(const T& value) const
    {
        for (usize i = 0; i < m_size; ++i)
        {
            if (m_data[i] == value) return static_cast<i32>(i);
        }
        return -1;
    }

    void SwapRemove(usize index)
    {
        RemoveSwap(index);
    }

private:
    T* m_data;
    usize m_size;
    usize m_capacity;
};

template<typename T, u32 N>
class TStaticArray
{
public:
    TStaticArray()
    {
    }

    T& operator[](usize index) { return m_data[index]; }
    const T& operator[](usize index) const { return m_data[index]; }

    T* Data() { return m_data; }
    const T* Data() const { return m_data; }
    constexpr usize Size() const { return N; }
    bool IsEmpty() const { return false; }

    T* Begin() { return m_data; }
    T* End() { return m_data + N; }
    const T* Begin() const { return m_data; }
    const T* End() const { return m_data + N; }

    T& First() { return m_data[0]; }
    T& Last() { return m_data[N - 1]; }
    const T& First() const { return m_data[0]; }
    const T& Last() const { return m_data[N - 1]; }

    T m_data[N];
};

template<typename T>
class TSpan
{
public:
    TSpan() : m_data(nullptr), m_size(0) {}
    TSpan(T* data, usize size) : m_data(data), m_size(size) {}

    template<usize N>
    TSpan(T (&arr)[N]) : m_data(arr), m_size(N) {}

    template<usize N>
    TSpan(TStaticArray<T, N>& arr) : m_data(arr.Data()), m_size(N) {}

    TSpan(const TSpan& other) = default;
    TSpan& operator=(const TSpan& other) = default;

    T& operator[](usize index) { return m_data[index]; }
    const T& operator[](usize index) const { return m_data[index]; }

    T* Data() const { return m_data; }
    usize Size() const { return m_size; }
    bool IsEmpty() const { return m_size == 0; }

    T* Begin() const { return m_data; }
    T* End() const { return m_data + m_size; }

    TSpan SubSpan(usize offset, usize count) const
    {
        return TSpan(m_data + offset, count);
    }

    T& First() { return m_data[0]; }
    T& Last() { return m_data[m_size - 1]; }
    const T& First() const { return m_data[0]; }
    const T& Last() const { return m_data[m_size - 1]; }

private:
    T* m_data;
    usize m_size;
};

class GString
{
public:
    GString();
    GString(const char* str);
    GString(const char* str, usize len);
    GString(const GString& other);
    GString(GString&& other) noexcept;
    ~GString();

    GString& operator=(const char* str);
    GString& operator=(const GString& other);
    GString& operator=(GString&& other) noexcept;

    bool operator==(const GString& other) const;
    bool operator==(const char* str) const;
    bool operator!=(const GString& other) const;
    bool operator!=(const char* str) const;

    GString operator+(const GString& other) const;
    GString operator+(const char* str) const;
    GString& operator+=(const GString& other);
    GString& operator+=(const char* str);

    const char* c_str() const;
    usize Len() const;

    GString& Append(const char* str);
    GString& Append(const GString& str);
    GString& Append(char c);

    static GString Format(const char* fmt, ...);
    static GString VFormat(const char* fmt, va_list args);

    TArray<GString> Split(char delimiter) const;
    GString SubStr(usize offset, usize count) const;
    i32 Find(const char* str, usize startPos = 0) const;
    GString Replace(const char* from, const char* to) const;

    void Clear();
    bool IsEmpty() const;

private:
    void Grow(usize required);
    void SetFromCStr(const char* str, usize len);

    static constexpr usize SSO_SIZE = 32;

    union
    {
        char m_ssoBuffer[SSO_SIZE];
        char* m_longData;
    };
    usize m_length;
    usize m_capacity;
    bool m_isLong;
};

}
