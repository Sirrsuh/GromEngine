#include "RHI/Backend/D3D11/D3D11_Device.h"
#include "RHI/Backend/D3D11/D3D11_Texture.h"
#include "RHI/Backend/D3D11/D3D11_Shader.h"
#include "RHI/Backend/D3D11/D3D11_Buffer.h"
#include "RHI/Backend/D3D11/D3D11_Pipeline.h"
#include "D3D11_Conversions.h"

#include <dxgi1_2.h>
#include <array>

namespace grom
{

D3D11Device::~D3D11Device()
{
	if (m_BackBufferTexture)
		m_BackBufferTexture->Release();

	if (m_LinearSampler)
		m_LinearSampler->Release();
	if (m_PointSampler)
		m_PointSampler->Release();
	if (m_ComparisonSampler)
		m_ComparisonSampler->Release();

	ReleaseBackBufferViews();

	if (m_SwapChain)
		m_SwapChain->Release();

	if (m_Context)
	{
		m_Context->ClearState();
		m_Context->Release();
	}

	if (m_Device)
		m_Device->Release();
}

ERenderAPI D3D11Device::GetAPI()
{
	return ERenderAPI::D3D11;
}

void* D3D11Device::GetNativeDevice()
{
	return m_Device;
}

void* D3D11Device::GetNativeContext()
{
	return m_Context;
}

DeviceDesc& D3D11Device::GetDesc()
{
	return m_Desc;
}

D3D11Device* D3D11Device::Create(DeviceDesc& desc)
{
	D3D11Device* device = new D3D11Device();
	device->m_Desc = desc;

	UINT createFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	D3D_FEATURE_LEVEL obtainedLevel;

	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createFlags,
		featureLevels,
		static_cast<UINT>(std::size(featureLevels)),
		D3D11_SDK_VERSION,
		&device->m_Device,
		&obtainedLevel,
		&device->m_Context
	);

	if (FAILED(hr))
	{
		delete device;
		return nullptr;
	}

	if (!device->CreateSwapChain())
	{
		delete device;
		return nullptr;
	}

	device->CreateBackBufferViews();

	// Create samplers
	{
		D3D11_SAMPLER_DESC sampDesc = {};

		// Linear sampler (s0)
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device->m_Device->CreateSamplerState(&sampDesc, &device->m_LinearSampler);

		// Point sampler (s1)
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		device->m_Device->CreateSamplerState(&sampDesc, &device->m_PointSampler);

		// Comparison sampler for shadow mapping (s3)
		sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		sampDesc.BorderColor[0] = 1.0f;
		sampDesc.BorderColor[1] = 1.0f;
		sampDesc.BorderColor[2] = 1.0f;
		sampDesc.BorderColor[3] = 1.0f;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		device->m_Device->CreateSamplerState(&sampDesc, &device->m_ComparisonSampler);
	}

	return device;
}

bool D3D11Device::CreateSwapChain()
{
	IDXGIDevice* dxgiDevice = nullptr;
	HRESULT hr = m_Device->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
	if (FAILED(hr))
		return false;

	IDXGIAdapter* dxgiAdapter = nullptr;
	hr = dxgiDevice->GetAdapter(&dxgiAdapter);
	dxgiDevice->Release();
	if (FAILED(hr))
		return false;

	IDXGIFactory* dxgiFactory = nullptr;
	hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&dxgiFactory));
	dxgiAdapter->Release();
	if (FAILED(hr))
		return false;

	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = m_Desc.Width;
	sd.BufferDesc.Height = m_Desc.Height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2;
	sd.OutputWindow = static_cast<HWND>(m_Desc.WindowHandle);
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	hr = dxgiFactory->CreateSwapChain(m_Device, &sd, &m_SwapChain);
	dxgiFactory->Release();

	if (FAILED(hr))
		return false;

	if (m_Desc.Fullscreen)
	{
		m_SwapChain->SetFullscreenState(TRUE, nullptr);
	}

	return true;
}

void D3D11Device::CreateBackBufferViews()
{
	ID3D11Texture2D* backBuffer = nullptr;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer));
	if (FAILED(hr))
		return;

	hr = m_Device->CreateRenderTargetView(backBuffer, nullptr, &m_BackBufferRTV);
	backBuffer->Release();
	if (FAILED(hr))
		return;

	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = m_Desc.Width;
	depthDesc.Height = m_Desc.Height;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;

	hr = m_Device->CreateTexture2D(&depthDesc, nullptr, &m_DepthStencilBuffer);
	if (FAILED(hr))
		return;

	hr = m_Device->CreateDepthStencilView(m_DepthStencilBuffer, nullptr, &m_DepthStencilView);
	if (FAILED(hr))
		return;

	m_Context->OMSetRenderTargets(1, &m_BackBufferRTV, m_DepthStencilView);

	TextureDesc bbDesc{};
	bbDesc.Width = m_Desc.Width;
	bbDesc.Height = m_Desc.Height;
	bbDesc.Format = EFormat::R8G8B8A8_UNORM;
	bbDesc.IsRenderTarget = true;
	bbDesc.MipLevels = 1;
	bbDesc.ArraySize = 1;

	m_BackBufferTexture = D3D11Texture::Create(bbDesc, m_Device);
}

void D3D11Device::ReleaseBackBufferViews()
{
	if (m_BackBufferTexture)
	{
		m_BackBufferTexture->Release();
		m_BackBufferTexture = nullptr;
	}

	if (m_DepthStencilView)
	{
		m_DepthStencilView->Release();
		m_DepthStencilView = nullptr;
	}

	if (m_DepthStencilBuffer)
	{
		m_DepthStencilBuffer->Release();
		m_DepthStencilBuffer = nullptr;
	}

	if (m_BackBufferRTV)
	{
		m_BackBufferRTV->Release();
		m_BackBufferRTV = nullptr;
	}
}

void D3D11Device::Resize(u32 w, u32 h)
{
	if (w == 0 || h == 0)
		return;

	m_Context->OMSetRenderTargets(0, nullptr, nullptr);

	ReleaseBackBufferViews();

	m_SwapChain->ResizeBuffers(0, w, h, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

	CreateBackBufferViews();

	m_Desc.Width = w;
	m_Desc.Height = h;
}

void D3D11Device::Present()
{
	m_SwapChain->Present(m_Desc.VSync ? 1 : 0, 0);
}

void D3D11Device::ClearRenderTarget(Texture* rt, const f32 color[4])
{
	if (rt)
	{
		D3D11Texture* tex = static_cast<D3D11Texture*>(rt);
		ID3D11RenderTargetView* rtv = tex->GetRTV();
		if (rtv)
			m_Context->ClearRenderTargetView(rtv, color);
	}
	else if (m_BackBufferRTV)
	{
		m_Context->ClearRenderTargetView(m_BackBufferRTV, color);
	}
}

void D3D11Device::ClearDepthStencil(Texture* ds, f32 depth, u8 stencil)
{
	if (ds)
	{
		D3D11Texture* tex = static_cast<D3D11Texture*>(ds);
		ID3D11DepthStencilView* dsv = tex->GetDSV();
		if (dsv)
			m_Context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
	else if (m_DepthStencilView)
	{
		m_Context->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, stencil);
	}
}

void D3D11Device::SetViewport(ViewportDesc& vp)
{
	D3D11_VIEWPORT d3dVP{};
	d3dVP.TopLeftX = vp.TopLeftX;
	d3dVP.TopLeftY = vp.TopLeftY;
	d3dVP.Width = vp.Width;
	d3dVP.Height = vp.Height;
	d3dVP.MinDepth = vp.MinDepth;
	d3dVP.MaxDepth = vp.MaxDepth;
	m_Context->RSSetViewports(1, &d3dVP);
}

void D3D11Device::SetScissorRect(Rect2D& rect)
{
	D3D11_RECT d3dRect{};
	d3dRect.left = rect.Left;
	d3dRect.top = rect.Top;
	d3dRect.right = rect.Right;
	d3dRect.bottom = rect.Bottom;
	m_Context->RSSetScissorRects(1, &d3dRect);
}

void D3D11Device::SetPipeline(Pipeline* pipeline)
{
	if (!pipeline)
		return;

	D3D11Pipeline* p = static_cast<D3D11Pipeline*>(pipeline);

	m_Context->IASetInputLayout(p->GetInputLayout());

	D3D11Shader* vs = p->GetVS();
	D3D11Shader* ps = p->GetPS();
	D3D11Shader* gs = p->GetGS();
	D3D11Shader* hs = p->GetHS();
	D3D11Shader* ds = p->GetDS();

	m_Context->VSSetShader(vs ? vs->GetVertexShader() : nullptr, nullptr, 0);
	m_Context->PSSetShader(ps ? ps->GetPixelShader() : nullptr, nullptr, 0);
	m_Context->GSSetShader(gs ? gs->GetGeometryShader() : nullptr, nullptr, 0);
	m_Context->HSSetShader(hs ? hs->GetHullShader() : nullptr, nullptr, 0);
	m_Context->DSSetShader(ds ? ds->GetDomainShader() : nullptr, nullptr, 0);

	m_Context->RSSetState(p->GetRasterizerState());
	m_Context->OMSetDepthStencilState(p->GetDepthStencilState(), 0);
	m_Context->OMSetBlendState(p->GetBlendState(), nullptr, 0xFFFFFFFF);

	m_Context->IASetPrimitiveTopology(ToD3DTopology(pipeline->GetDesc().Topology));
}

void D3D11Device::SetVertexBuffer(Buffer* buffer, u32 slot)
{
	if (!buffer)
		return;

	D3D11Buffer* buf = static_cast<D3D11Buffer*>(buffer);
	ID3D11Buffer* d3dBuf = static_cast<ID3D11Buffer*>(buf->GetHandle());
	BufferDesc& desc = buf->GetDesc();
	UINT stride = desc.Stride;
	UINT offset = 0;
	m_Context->IASetVertexBuffers(slot, 1, &d3dBuf, &stride, &offset);
}

void D3D11Device::SetIndexBuffer(Buffer* buffer)
{
	if (!buffer)
		return;

	D3D11Buffer* buf = static_cast<D3D11Buffer*>(buffer);
	ID3D11Buffer* d3dBuf = static_cast<ID3D11Buffer*>(buf->GetHandle());
	BufferDesc& desc = buf->GetDesc();
	DXGI_FORMAT format = (desc.Stride == 2) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_Context->IASetIndexBuffer(d3dBuf, format, 0);
}

void D3D11Device::SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader)
{
	if (!buffer)
		return;

	D3D11Buffer* buf = static_cast<D3D11Buffer*>(buffer);
	ID3D11Buffer* d3dBuf = static_cast<ID3D11Buffer*>(buf->GetHandle());

	switch (shader)
	{
	case EShaderType::Vertex:   m_Context->VSSetConstantBuffers(slot, 1, &d3dBuf); break;
	case EShaderType::Pixel:    m_Context->PSSetConstantBuffers(slot, 1, &d3dBuf); break;
	case EShaderType::Geometry: m_Context->GSSetConstantBuffers(slot, 1, &d3dBuf); break;
	case EShaderType::Hull:     m_Context->HSSetConstantBuffers(slot, 1, &d3dBuf); break;
	case EShaderType::Domain:   m_Context->DSSetConstantBuffers(slot, 1, &d3dBuf); break;
	case EShaderType::Compute:  m_Context->CSSetConstantBuffers(slot, 1, &d3dBuf); break;
	}
}

void D3D11Device::SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil)
{
    ID3D11RenderTargetView* rtvs[8] = {};
    for (u32 i = 0; i < count && i < 8; ++i)
    {
        if (renderTargets[i])
        {
            D3D11Texture* tex = static_cast<D3D11Texture*>(renderTargets[i]);
            rtvs[i] = tex->GetRTV();
        }
    }

    ID3D11DepthStencilView* dsv = nullptr;
    if (depthStencil)
    {
        D3D11Texture* dsTex = static_cast<D3D11Texture*>(depthStencil);
        dsv = dsTex->GetDSV();
    }

    m_Context->OMSetRenderTargets(count, rtvs, dsv);
}

void D3D11Device::SetShaderResource(Texture* texture, u32 slot, EShaderType shader)
{
	if (!texture)
		return;

	D3D11Texture* tex = static_cast<D3D11Texture*>(texture);
	ID3D11ShaderResourceView* srv = tex->GetSRV();

	switch (shader)
	{
	case EShaderType::Vertex:   m_Context->VSSetShaderResources(slot, 1, &srv); break;
	case EShaderType::Pixel:    m_Context->PSSetShaderResources(slot, 1, &srv); break;
	case EShaderType::Geometry: m_Context->GSSetShaderResources(slot, 1, &srv); break;
	case EShaderType::Hull:     m_Context->HSSetShaderResources(slot, 1, &srv); break;
	case EShaderType::Domain:   m_Context->DSSetShaderResources(slot, 1, &srv); break;
	case EShaderType::Compute:  m_Context->CSSetShaderResources(slot, 1, &srv); break;
	}
}

void D3D11Device::Draw(u32 vertexCount, u32 startVertex)
{
	m_Context->Draw(vertexCount, startVertex);
}

void D3D11Device::DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex)
{
	m_Context->DrawIndexed(indexCount, startIndex, baseVertex);
}

void D3D11Device::Dispatch(u32 groupX, u32 groupY, u32 groupZ)
{
	m_Context->Dispatch(groupX, groupY, groupZ);
}

void D3D11Device::BeginFrame()
{
	ID3D11SamplerState* samplers[4] = { m_LinearSampler, m_PointSampler, nullptr, m_ComparisonSampler };
	m_Context->VSSetSamplers(0, 4, samplers);
	m_Context->PSSetSamplers(0, 4, samplers);
}

void D3D11Device::EndFrame()
{
}

Texture* D3D11Device::GetBackBuffer()
{
	return m_BackBufferTexture;
}

}
