#pragma once
#include "../../RHI_Buffer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace grom {

class D3D12Buffer : public Buffer
{
public:
    D3D12Buffer() = default;
    ~D3D12Buffer() override;

    void* GetHandle() override;
    BufferDesc& GetDesc() override;
    void Update(void* data, u32 size, u32 offset) override;
    void* Map() override;
    void Unmap() override;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_Resource; }
    D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return m_Resource->GetGPUVirtualAddress(); }

    static D3D12Buffer* Create(BufferDesc& desc, ID3D12Device* device);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
    BufferDesc m_Desc;
    void* m_MappedData = nullptr;
    u32 m_AlignedSize = 0;
};

} // namespace grom