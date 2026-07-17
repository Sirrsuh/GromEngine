#pragma once

#include "../../RHI_Pipeline.h"
#include "D3D11_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>

namespace grom
{

class D3D11Pipeline : public Pipeline
{
public:
	D3D11Pipeline() = default;
	~D3D11Pipeline() override;

	void* GetHandle() override;
	PipelineDesc& GetDesc() override;

	ID3D11RasterizerState* GetRasterizerState() const { return m_RasterizerState; }
	ID3D11DepthStencilState* GetDepthStencilState() const { return m_DepthStencilState; }
	ID3D11BlendState* GetBlendState() const { return m_BlendState; }
	ID3D11InputLayout* GetInputLayout() const { return m_InputLayout; }
	D3D11Shader* GetVS() const { return m_VS; }
	D3D11Shader* GetPS() const { return m_PS; }
	D3D11Shader* GetGS() const { return m_GS; }
	D3D11Shader* GetHS() const { return m_HS; }
	D3D11Shader* GetDS() const { return m_DS; }
	D3D11Shader* GetCS() const { return m_CS; }

	static D3D11Pipeline* Create(PipelineDesc& desc, ID3D11Device* device);

private:
	static D3D11_CULL_MODE ConvertCullMode(ECullMode mode);
	static D3D11_COMPARISON_FUNC ConvertCompareOp(ECompareOp op);
	static D3D11_BLEND ConvertBlendFactor(EBlendFactor factor);
	static D3D11_BLEND_OP ConvertBlendOp(EBlendOp op);
	static D3D11_PRIMITIVE_TOPOLOGY ConvertTopology(EPrimitiveTopology topology);
	static DXGI_FORMAT ConvertFormat(EFormat format);

	ID3D11RasterizerState* m_RasterizerState = nullptr;
	ID3D11DepthStencilState* m_DepthStencilState = nullptr;
	ID3D11BlendState* m_BlendState = nullptr;
	ID3D11InputLayout* m_InputLayout = nullptr;
	D3D11Shader* m_VS = nullptr;
	D3D11Shader* m_PS = nullptr;
	D3D11Shader* m_GS = nullptr;
	D3D11Shader* m_HS = nullptr;
	D3D11Shader* m_DS = nullptr;
	D3D11Shader* m_CS = nullptr;
	PipelineDesc m_Desc;
};

} // namespace grom
