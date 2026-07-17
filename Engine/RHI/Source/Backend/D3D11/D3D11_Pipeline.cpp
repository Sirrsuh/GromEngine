#include "RHI/Backend/D3D11/D3D11_Pipeline.h"
#include "D3D11_Conversions.h"

namespace grom
{

D3D11Pipeline::~D3D11Pipeline()
{
	if (m_RasterizerState)  m_RasterizerState->Release();
	if (m_DepthStencilState) m_DepthStencilState->Release();
	if (m_BlendState)       m_BlendState->Release();
	if (m_InputLayout)      m_InputLayout->Release();
}

void* D3D11Pipeline::GetHandle()
{
	return m_RasterizerState;
}

PipelineDesc& D3D11Pipeline::GetDesc()
{
	return m_Desc;
}

D3D11_CULL_MODE D3D11Pipeline::ConvertCullMode(ECullMode mode)
{
	return ToD3DCullMode(mode);
}

D3D11_COMPARISON_FUNC D3D11Pipeline::ConvertCompareOp(ECompareOp op)
{
	return ToD3DCompareOp(op);
}

D3D11_BLEND D3D11Pipeline::ConvertBlendFactor(EBlendFactor factor)
{
	return ToD3DBlend(factor);
}

D3D11_BLEND_OP D3D11Pipeline::ConvertBlendOp(EBlendOp op)
{
	return ToD3DBlendOp(op);
}

D3D11_PRIMITIVE_TOPOLOGY D3D11Pipeline::ConvertTopology(EPrimitiveTopology topology)
{
	return ToD3DTopology(topology);
}

DXGI_FORMAT D3D11Pipeline::ConvertFormat(EFormat format)
{
	return ToDXGIFormat(format);
}

D3D11Pipeline* D3D11Pipeline::Create(PipelineDesc& desc, ID3D11Device* device)
{
	D3D11Pipeline* pipeline = new D3D11Pipeline();
	pipeline->m_Desc = desc;

	if (desc.VS)
		pipeline->m_VS = static_cast<D3D11Shader*>(desc.VS);
	if (desc.PS)
		pipeline->m_PS = static_cast<D3D11Shader*>(desc.PS);
	if (desc.GS)
		pipeline->m_GS = static_cast<D3D11Shader*>(desc.GS);
	if (desc.HS)
		pipeline->m_HS = static_cast<D3D11Shader*>(desc.HS);
	if (desc.DS)
		pipeline->m_DS = static_cast<D3D11Shader*>(desc.DS);
	if (desc.CS)
		pipeline->m_CS = static_cast<D3D11Shader*>(desc.CS);

	BufferLayout& layout = desc.InputLayout;
	if (layout.Elements.Size() > 0 && pipeline->m_VS && pipeline->m_VS->GetBlob())
	{
		ID3DBlob* vsBlob = pipeline->m_VS->GetBlob();

		TArray<D3D11_INPUT_ELEMENT_DESC> elements;

		for (u32 i = 0; i < static_cast<u32>(layout.Elements.Size()); ++i)
		{
			BufferLayout::Element& elem = layout.Elements[i];

			D3D11_INPUT_ELEMENT_DESC d3dElem{};
			d3dElem.SemanticName = elem.Name.c_str();
			d3dElem.SemanticIndex = 0;
			d3dElem.Format = ToDXGIFormat(elem.Format);
			d3dElem.InputSlot = elem.Slot;
			d3dElem.AlignedByteOffset = elem.Offset;
			d3dElem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			d3dElem.InstanceDataStepRate = 0;

			elements.Add(d3dElem);
		}

		device->CreateInputLayout(
			elements.Data(),
			static_cast<UINT>(elements.Size()),
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&pipeline->m_InputLayout
		);
	}

	D3D11_RASTERIZER_DESC rsDesc{};
	rsDesc.FillMode = desc.Rasterizer.Wireframe ? D3D11_FILL_WIREFRAME : D3D11_FILL_SOLID;
	rsDesc.CullMode = ConvertCullMode(desc.Rasterizer.CullMode);
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = desc.Rasterizer.DepthBias;
	rsDesc.DepthBiasClamp = desc.Rasterizer.DepthBiasClamp;
	rsDesc.SlopeScaledDepthBias = desc.Rasterizer.SlopeScaledDepthBias;
	rsDesc.DepthClipEnable = desc.Rasterizer.DepthClip ? TRUE : FALSE;
	rsDesc.ScissorEnable = TRUE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.AntialiasedLineEnable = FALSE;

	HRESULT hr = device->CreateRasterizerState(&rsDesc, &pipeline->m_RasterizerState);
	if (FAILED(hr))
	{
		delete pipeline;
		return nullptr;
	}

	D3D11_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = desc.DepthStencil.DepthEnable ? TRUE : FALSE;
	dsDesc.DepthWriteMask = desc.DepthStencil.DepthWrite ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = ConvertCompareOp(desc.DepthStencil.DepthFunc);
	dsDesc.StencilEnable = desc.DepthStencil.StencilEnable ? TRUE : FALSE;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	hr = device->CreateDepthStencilState(&dsDesc, &pipeline->m_DepthStencilState);
	if (FAILED(hr))
	{
		delete pipeline;
		return nullptr;
	}

	D3D11_BLEND_DESC blendDesc{};
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = desc.Blend.Enable ? TRUE : FALSE;
	blendDesc.RenderTarget[0].SrcBlend = ConvertBlendFactor(desc.Blend.SrcFactor);
	blendDesc.RenderTarget[0].DestBlend = ConvertBlendFactor(desc.Blend.DstFactor);
	blendDesc.RenderTarget[0].BlendOp = ConvertBlendOp(desc.Blend.ColorOp);
	blendDesc.RenderTarget[0].SrcBlendAlpha = ConvertBlendFactor(desc.Blend.SrcFactorAlpha);
	blendDesc.RenderTarget[0].DestBlendAlpha = ConvertBlendFactor(desc.Blend.DstFactorAlpha);
	blendDesc.RenderTarget[0].BlendOpAlpha = ConvertBlendOp(desc.Blend.AlphaOp);
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	hr = device->CreateBlendState(&blendDesc, &pipeline->m_BlendState);
	if (FAILED(hr))
	{
		delete pipeline;
		return nullptr;
	}

	return pipeline;
}

}
