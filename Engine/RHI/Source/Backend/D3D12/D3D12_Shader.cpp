#include "RHI/Backend/D3D12/D3D12_Shader.h"
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

using Microsoft::WRL::ComPtr;

namespace grom {

D3D12Shader::~D3D12Shader()
{
}

void* D3D12Shader::GetHandle() { return m_Bytecode.Get(); }
ShaderDesc& D3D12Shader::GetDesc() { return m_Desc; }

D3D12Shader* D3D12Shader::Create(ShaderDesc& desc)
{
    if (desc.Source.IsEmpty())
        return nullptr;

    UINT compileFlags = 0;
#ifdef _DEBUG
    compileFlags |= D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    const char* target = "";
    switch (desc.Type) {
    case EShaderType::Vertex:   target = "vs_5_1"; break;
    case EShaderType::Pixel:    target = "ps_5_1"; break;
    case EShaderType::Compute:  target = "cs_5_1"; break;
    default: return nullptr;
    }

    ComPtr<ID3DBlob> errorBlob;
    ComPtr<ID3DBlob> bytecode;
    HRESULT hr = D3DCompile(desc.Source.c_str(), desc.Source.Len(), nullptr, nullptr, nullptr,
        desc.EntryPoint.c_str(), target, compileFlags, 0, &bytecode, &errorBlob);

    if (FAILED(hr)) {
        if (errorBlob) {
            OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        return nullptr;
    }

    D3D12Shader* shader = new D3D12Shader();
    shader->m_Bytecode = bytecode;
    shader->m_Desc = desc;
    return shader;
}

} // namespace grom