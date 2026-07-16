#include <Core/Memory.h>
#include <Windows.h>
#include <cstdlib>
#include <cstring>

namespace grom
{

static Allocator* g_defaultAllocator = nullptr;

HeapAllocator::HeapAllocator()
{
}

HeapAllocator::~HeapAllocator()
{
}

void* HeapAllocator::Allocate(usize size, usize alignment)
{
    GROM_UNUSED(alignment);
    void* ptr = HeapAlloc(GetProcessHeap(), 0, size);
    return ptr;
}

void HeapAllocator::Free(void* ptr)
{
    if (ptr)
    {
        HeapFree(GetProcessHeap(), 0, ptr);
    }
}

usize HeapAllocator::GetAllocatedSize(void* ptr) const
{
    if (ptr)
    {
        return HeapSize(GetProcessHeap(), 0, ptr);
    }
    return 0;
}

LinearAllocator::LinearAllocator(usize totalSize)
    : m_memory(static_cast<u8*>(HeapAlloc(GetProcessHeap(), 0, totalSize)))
    , m_totalSize(totalSize)
    , m_offset(0)
{
}

LinearAllocator::~LinearAllocator()
{
    if (m_memory)
    {
        HeapFree(GetProcessHeap(), 0, m_memory);
    }
}

void* LinearAllocator::Allocate(usize size, usize alignment)
{
    usize alignmentPadding = (alignment - (m_offset % alignment)) % alignment;
    usize alignedOffset = m_offset + alignmentPadding;

    if (alignedOffset + size > m_totalSize)
    {
        return nullptr;
    }

    void* ptr = m_memory + alignedOffset;
    m_offset = alignedOffset + size;
    return ptr;
}

void LinearAllocator::Free(void* ptr)
{
    (void)ptr;
}

usize LinearAllocator::GetAllocatedSize(void* ptr) const
{
    (void)ptr;
    return m_offset;
}

void LinearAllocator::Reset()
{
    m_offset = 0;
}

PoolAllocator::PoolAllocator(usize blockSize, usize blockCount)
    : m_memory(static_cast<u8*>(HeapAlloc(GetProcessHeap(), 0, blockSize * blockCount)))
    , m_blockSize(blockSize < sizeof(void*) ? sizeof(void*) : blockSize)
    , m_blockCount(blockCount)
    , m_freeList(nullptr)
{
    m_freeList = m_memory;
    u8* current = static_cast<u8*>(m_freeList);

    for (usize i = 0; i < blockCount - 1; ++i)
    {
        u8* next = current + m_blockSize;
        std::memcpy(current, &next, sizeof(u8*));
        current = next;
    }

    std::memcpy(current, &m_freeList, sizeof(void*));
}

PoolAllocator::~PoolAllocator()
{
    if (m_memory)
    {
        HeapFree(GetProcessHeap(), 0, m_memory);
    }
}

void* PoolAllocator::Allocate(usize size, usize alignment)
{
    (void)size;
    (void)alignment;

    if (!m_freeList) return nullptr;

    void* block = m_freeList;
    void* next = nullptr;
    std::memcpy(&next, block, sizeof(void*));
    m_freeList = next;

    return block;
}

void PoolAllocator::Free(void* ptr)
{
    if (!ptr) return;

    std::memcpy(ptr, &m_freeList, sizeof(void*));
    m_freeList = ptr;
}

usize PoolAllocator::GetAllocatedSize(void* ptr) const
{
    (void)ptr;
    return m_blockSize;
}

Allocator* GDefaultAllocator()
{
    return g_defaultAllocator;
}

void InitGlobalAllocator()
{
    static HeapAllocator heapAlloc;
    g_defaultAllocator = &heapAlloc;
}

void ShutdownGlobalAllocator()
{
    g_defaultAllocator = nullptr;
}

}
