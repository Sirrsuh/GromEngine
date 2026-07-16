#include "RHI/Backend/D3D11/D3D11_Shader.h"

#include <d3d11.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3dcompiler.lib")

namespace grom
{

D3D11Shader::~D3D11Shader()
{
	if (m_VertexShader)  m_VertexShader->Release();
	if (m_PixelShader)   m_PixelShader->Release();
	if (m_GeometryShader) m_GeometryShader->Release();
	if (m_ComputeShader) m_ComputeShader->Release();
	if (m_HullShader)    m_HullShader->Release();
	if (m_DomainShader)  m_DomainShader->Release();
	if (m_Blob)          m_Blob->Release();
}

void* D3D11Shader::GetHandle()
{
	switch (m_Desc.Type)
	{
	case EShaderType::Vertex:   return m_VertexShader;
	case EShaderType::Pixel:    return m_PixelShader;
	case EShaderType::Geometry: return m_GeometryShader;
	case EShaderType::Compute:  return m_ComputeShader;
	case EShaderType::Hull:     return m_HullShader;
	case EShaderType::Domain:   return m_DomainShader;
	default:                    return nullptr;
	}
}

ShaderDesc& D3D11Shader::GetDesc()
{
	return m_Desc;
}

static HRESULT CompileShaderSource(const char* source, usize sourceSize, const char* entryPoint, const char* target, ID3DBlob** outBlob)
{
	UINT compileFlags = 0;
#ifdef _DEBUG
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompile(
		source,
		sourceSize,
		nullptr,
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		target,
		compileFlags,
		0,
		outBlob,
		&errorBlob
	);

	if (errorBlob)
		errorBlob->Release();

	return hr;
}

D3D11Shader* D3D11Shader::Create(ShaderDesc& desc, ID3D11Device* device)
{
	D3D11Shader* shader = new D3D11Shader();
	shader->m_Desc = desc;

	const char* profile = nullptr;
	switch (desc.Type)
	{
	case EShaderType::Vertex:   profile = "vs_5_0"; break;
	case EShaderType::Pixel:    profile = "ps_5_0"; break;
	case EShaderType::Geometry: profile = "gs_5_0"; break;
	case EShaderType::Compute:  profile = "cs_5_0"; break;
	case EShaderType::Hull:     profile = "hs_5_0"; break;
	case EShaderType::Domain:   profile = "ds_5_0"; break;
	default:
		delete shader;
		return nullptr;
	}

	const char* src = desc.Source.c_str();
	usize srcLen = desc.Source.Len();

	ID3DBlob* blob = nullptr;
	HRESULT hr = CompileShaderSource(src, srcLen, desc.EntryPoint.c_str(), profile, &blob);
	if (FAILED(hr))
	{
		delete shader;
		return nullptr;
	}

	shader->m_Blob = blob;

	void* data = blob->GetBufferPointer();
	SIZE_T dataSize = blob->GetBufferSize();

	switch (desc.Type)
	{
	case EShaderType::Vertex:
		hr = device->CreateVertexShader(data, dataSize, nullptr, &shader->m_VertexShader);
		break;
	case EShaderType::Pixel:
		hr = device->CreatePixelShader(data, dataSize, nullptr, &shader->m_PixelShader);
		break;
	case EShaderType::Geometry:
		hr = device->CreateGeometryShader(data, dataSize, nullptr, &shader->m_GeometryShader);
		break;
	case EShaderType::Compute:
		hr = device->CreateComputeShader(data, dataSize, nullptr, &shader->m_ComputeShader);
		break;
	case EShaderType::Hull:
		hr = device->CreateHullShader(data, dataSize, nullptr, &shader->m_HullShader);
		break;
	case EShaderType::Domain:
		hr = device->CreateDomainShader(data, dataSize, nullptr, &shader->m_DomainShader);
		break;
	default:
		break;
	}

	if (FAILED(hr))
	{
		delete shader;
		return nullptr;
	}

	return shader;
}

}
