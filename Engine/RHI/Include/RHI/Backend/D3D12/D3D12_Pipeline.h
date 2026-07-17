#pragma once
#include "../../RHI_Pipeline.h"
#include "D3D12_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

namespace grom {

class D3D12Device;

class D3D12Pipeline : public Pipeline
{
public:
    D3D12Pipeline() = default;
    ~D3D12Pipeline() override;

    void* GetHandle() override;
    PipelineDesc& GetDesc() override;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPSO() const { return m_PSO; }
    Microsoft::WRL::ComPtr<ID3D12RootSignature> GetRootSignature() const { return m_RootSignature; }

    static D3D12Pipeline* Create(PipelineDesc& desc, ID3D12Device* device, D3D12Device* d3d12Device);

private:
    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_PSO;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_RootSignature;
    PipelineDesc m_Desc;
    D3D12_PRIMITIVE_TOPOLOGY_TYPE m_PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

} // namespace grom