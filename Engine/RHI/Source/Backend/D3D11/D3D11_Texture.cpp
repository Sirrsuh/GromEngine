#include "RHI/Backend/D3D11/D3D11_Texture.h"
#include "D3D11_Conversions.h"

namespace grom
{

D3D11Texture::~D3D11Texture()
{
	if (m_SRV) m_SRV->Release();
	if (m_RTV) m_RTV->Release();
	if (m_DSV) m_DSV->Release();
	if (m_UAV) m_UAV->Release();
	if (m_Texture) m_Texture->Release();
}

void* D3D11Texture::GetHandle()
{
	return m_Texture;
}

TextureDesc& D3D11Texture::GetDesc()
{
	return m_Desc;
}

void D3D11Texture::Upload(void* data, u32 mipLevel)
{
	if (!data || !m_Texture)
		return;

	UINT rowPitch = m_Desc.Width * 4;
	if (m_Desc.Format == EFormat::R32G32B32A32_FLOAT)
		rowPitch = m_Desc.Width * 16;
	else if (m_Desc.Format == EFormat::R16G16B16A16_FLOAT)
		rowPitch = m_Desc.Width * 8;
	else if (m_Desc.Format == EFormat::R32_FLOAT)
		rowPitch = m_Desc.Width * 4;
	else if (m_Desc.Format == EFormat::R8_UNORM)
		rowPitch = m_Desc.Width;

	ID3D11Device* dev = nullptr;
	m_Texture->GetDevice(&dev);
	if (!dev)
		return;

	ID3D11DeviceContext* ctx = nullptr;
	dev->GetImmediateContext(&ctx);
	dev->Release();

	if (ctx)
	{
		ctx->UpdateSubresource(m_Texture, mipLevel, nullptr, data, rowPitch, 0);
		ctx->Release();
	}
}

bool D3D11Texture::CreateViews(ID3D11Device* device)
{
	DXGI_FORMAT dxgiFormat = ToDXGIFormat(m_Desc.Format);

	if (m_Desc.Format == EFormat::Unknown)
	{
		if (m_Desc.IsDepthStencil)
			dxgiFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		else if (m_Desc.IsRenderTarget)
			dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		else
			dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	if (m_Desc.IsDepthStencil)
	{
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Format = dxgiFormat;
		dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Texture2D.MipSlice = 0;
		dsvDesc.Flags = 0;

		HRESULT hr = device->CreateDepthStencilView(m_Texture, &dsvDesc, &m_DSV);
		if (FAILED(hr))
			return false;
	}
	else
	{
		if (m_Desc.IsRenderTarget)
		{
			D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
			rtvDesc.Format = dxgiFormat;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			rtvDesc.Texture2D.MipSlice = 0;

			HRESULT hr = device->CreateRenderTargetView(m_Texture, &rtvDesc, &m_RTV);
			if (FAILED(hr))
				return false;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
		srvDesc.Format = dxgiFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = m_Desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		HRESULT hr = device->CreateShaderResourceView(m_Texture, &srvDesc, &m_SRV);
		if (FAILED(hr))
			return false;
	}

	if (m_Desc.IsUnorderedAccess)
	{
		D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
		uavDesc.Format = dxgiFormat;
		uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavDesc.Texture2D.MipSlice = 0;

		HRESULT hr = device->CreateUnorderedAccessView(m_Texture, &uavDesc, &m_UAV);
		if (FAILED(hr))
			return false;
	}

	return true;
}

D3D11Texture* D3D11Texture::Create(TextureDesc& desc, ID3D11Device* device)
{
	D3D11Texture* tex = new D3D11Texture();
	tex->m_Desc = desc;

	D3D11_TEXTURE2D_DESC texDesc{};
	texDesc.Width = desc.Width;
	texDesc.Height = desc.Height;
	texDesc.MipLevels = desc.MipLevels;
	texDesc.ArraySize = desc.ArraySize;
	texDesc.Format = ToDXGIFormat(desc.Format);

	if (texDesc.Format == DXGI_FORMAT_UNKNOWN)
	{
		if (desc.IsDepthStencil)
			texDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		else
			texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	}

	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	texDesc.BindFlags = 0;
	if (desc.IsRenderTarget)
		texDesc.BindFlags |= D3D11_BIND_RENDER_TARGET;
	if (desc.IsDepthStencil)
		texDesc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;
	if (desc.IsUnorderedAccess)
		texDesc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;
	if (!desc.IsDepthStencil)
		texDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData{};
	D3D11_SUBRESOURCE_DATA* pInitData = nullptr;

	if (desc.InitialData && !desc.IsDepthStencil)
	{
		initData.pSysMem = desc.InitialData;
		initData.SysMemPitch = desc.Width * 4;
		initData.SysMemSlicePitch = 0;
		pInitData = &initData;
	}

	HRESULT hr = device->CreateTexture2D(&texDesc, pInitData, &tex->m_Texture);
	if (FAILED(hr))
	{
		delete tex;
		return nullptr;
	}

	if (!tex->CreateViews(device))
	{
		delete tex;
		return nullptr;
	}

	return tex;
}

}
