#pragma once
#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>

namespace grom {

class D3D12Texture : public Texture
{
public:
    D3D12Texture() = default;
    ~D3D12Texture() override;

    void* GetHandle() override;
    TextureDesc& GetDesc() override;
    void Upload(void* data, u32 mipLevel = 0) override;

    Microsoft::WRL::ComPtr<ID3D12Resource> GetResource() const { return m_Resource; }
    Microsoft::WRL::ComPtr<ID3D12Resource> GetUploadBuffer() const { return m_UploadBuffer; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetRTV() const { return m_RTV; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetDSV() const { return m_DSV; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetSRV() const { return m_SRV; }
    CD3DX12_CPU_DESCRIPTOR_HANDLE GetUAV() const { return m_UAV; }

    static D3D12Texture* Create(TextureDesc& desc, ID3D12Device* device);

private:
    Microsoft::WRL::ComPtr<ID3D12Resource> m_Resource;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_UploadBuffer;
    TextureDesc m_Desc;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_RTV;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_DSV;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_SRV;
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_UAV;
    bool m_HasRTV = false;
    bool m_HasDSV = false;
    bool m_HasSRV = false;
    bool m_HasUAV = false;
};

} // namespace grom