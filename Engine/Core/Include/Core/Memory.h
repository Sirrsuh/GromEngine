#pragma once

#include <Core/Types.h>
#include <cstddef>
#include <new>

namespace grom
{

enum class EAllocationType
{
    Permanent,
    Frame,
    Scratch,
    GPU
};

class Allocator
{
public:
    virtual ~Allocator() = default;

    virtual void* Allocate(usize size, usize alignment = 16) = 0;
    virtual void Free(void* ptr) = 0;
    virtual usize GetAllocatedSize(void* ptr) const = 0;
};

class HeapAllocator : public Allocator
{
public:
    HeapAllocator();
    ~HeapAllocator() override;

    void* Allocate(usize size, usize alignment = 16) override;
    void Free(void* ptr) override;
    usize GetAllocatedSize(void* ptr) const override;
};

class LinearAllocator : public Allocator
{
public:
    explicit LinearAllocator(usize totalSize);
    ~LinearAllocator() override;

    void* Allocate(usize size, usize alignment = 16) override;
    void Free(void* ptr) override;
    usize GetAllocatedSize(void* ptr) const override;

    void Reset();

private:
    u8* m_memory;
    usize m_totalSize;
    usize m_offset;
};

class PoolAllocator : public Allocator
{
public:
    PoolAllocator(usize blockSize, usize blockCount);
    ~PoolAllocator() override;

    void* Allocate(usize size, usize alignment = 16) override;
    void Free(void* ptr) override;
    usize GetAllocatedSize(void* ptr) const override;

private:
    u8* m_memory;
    usize m_blockSize;
    usize m_blockCount;
    void* m_freeList;
};

Allocator* GDefaultAllocator();
void InitGlobalAllocator();
void ShutdownGlobalAllocator();

template<typename T, typename... Args>
T* AllocateObject(Allocator* allocator, Args&&... args)
{
    void* mem = allocator->Allocate(sizeof(T), alignof(T));
    return new(mem) T(static_cast<Args&&>(args)...);
}

template<typename T>
void FreeObject(Allocator* allocator, T* obj)
{
    if (obj)
    {
        obj->~T();
        allocator->Free(obj);
    }
}

}
