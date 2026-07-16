#pragma once

#include "../../RHI_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

namespace grom
{

class D3D11Shader : public Shader
{
public:
	D3D11Shader() = default;
	~D3D11Shader() override;

	void* GetHandle() override;
	ShaderDesc& GetDesc() override;

	ID3D11VertexShader* GetVertexShader() const { return m_VertexShader; }
	ID3D11PixelShader* GetPixelShader() const { return m_PixelShader; }
	ID3D11GeometryShader* GetGeometryShader() const { return m_GeometryShader; }
	ID3D11HullShader* GetHullShader() const { return m_HullShader; }
	ID3D11DomainShader* GetDomainShader() const { return m_DomainShader; }
	ID3D11ComputeShader* GetComputeShader() const { return m_ComputeShader; }
	ID3DBlob* GetBlob() const { return m_Blob; }

	static D3D11Shader* Create(ShaderDesc& desc, ID3D11Device* device);

private:
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;
	ID3D11GeometryShader* m_GeometryShader = nullptr;
	ID3D11HullShader* m_HullShader = nullptr;
	ID3D11DomainShader* m_DomainShader = nullptr;
	ID3D11ComputeShader* m_ComputeShader = nullptr;
	ID3DBlob* m_Blob = nullptr;
	ShaderDesc m_Desc;
};

} // namespace grom
