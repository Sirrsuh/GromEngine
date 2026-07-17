#pragma once
#include "../../RHI_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

namespace grom {

class D3D12Shader : public Shader
{
public:
    D3D12Shader() = default;
    ~D3D12Shader() override;

    void* GetHandle() override;
    ShaderDesc& GetDesc() override;

    Microsoft::WRL::ComPtr<ID3DBlob> GetBytecode() const { return m_Bytecode; }
    EShaderType GetShaderType() const { return m_Desc.Type; }

    static D3D12Shader* Create(ShaderDesc& desc);

private:
    Microsoft::WRL::ComPtr<ID3DBlob> m_Bytecode;
    ShaderDesc m_Desc;
};

} // namespace grom