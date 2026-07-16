#pragma once

#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>

namespace grom
{

class D3D11Texture : public Texture
{
public:
	D3D11Texture() = default;
	~D3D11Texture() override;

	void* GetHandle() override;
	TextureDesc& GetDesc() override;
	void Upload(void* data, u32 mipLevel) override;

	ID3D11ShaderResourceView* GetSRV() const { return m_SRV; }
	ID3D11RenderTargetView* GetRTV() const { return m_RTV; }
	ID3D11DepthStencilView* GetDSV() const { return m_DSV; }
	ID3D11UnorderedAccessView* GetUAV() const { return m_UAV; }

	static D3D11Texture* Create(TextureDesc& desc, ID3D11Device* device);

private:
	bool CreateViews(ID3D11Device* device);

	ID3D11Texture2D* m_Texture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
	ID3D11RenderTargetView* m_RTV = nullptr;
	ID3D11DepthStencilView* m_DSV = nullptr;
	ID3D11UnorderedAccessView* m_UAV = nullptr;
	TextureDesc m_Desc;
};

} // namespace grom
