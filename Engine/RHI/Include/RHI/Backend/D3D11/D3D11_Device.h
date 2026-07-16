#pragma once

#include "../../RHI_Device.h"
#include "../../RHI_Texture.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>

namespace grom
{

class D3D11Device : public Device
{
public:
	D3D11Device() = default;
	~D3D11Device() override;

	ERenderAPI GetAPI() override;
	void* GetNativeDevice() override;
	void* GetNativeContext() override;
	DeviceDesc& GetDesc() override;
	void Present() override;
	void Resize(u32 w, u32 h) override;
	void ClearRenderTarget(Texture* rt, const f32 color[4]) override;
	void ClearDepthStencil(Texture* ds, f32 depth, u8 stencil) override;
	void SetViewport(ViewportDesc& vp) override;
	void SetScissorRect(Rect2D& rect) override;
	void SetPipeline(Pipeline* pipeline) override;
	void SetVertexBuffer(Buffer* buffer, u32 slot) override;
	void SetIndexBuffer(Buffer* buffer) override;
	void SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader) override;
	void SetShaderResource(Texture* texture, u32 slot, EShaderType shader) override;
	void Draw(u32 vertexCount, u32 startVertex) override;
	void DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex) override;
	void Dispatch(u32 groupX, u32 groupY, u32 groupZ) override;
	void BeginFrame() override;
	void EndFrame() override;
	Texture* GetBackBuffer() override;

	static D3D11Device* Create(DeviceDesc& desc);

private:
	bool CreateSwapChain();
	void CreateBackBufferViews();
	void ReleaseBackBufferViews();

	ID3D11Device* m_Device = nullptr;
	ID3D11DeviceContext* m_Context = nullptr;
	IDXGISwapChain* m_SwapChain = nullptr;
	ID3D11RenderTargetView* m_BackBufferRTV = nullptr;
	ID3D11DepthStencilView* m_DepthStencilView = nullptr;
	ID3D11Texture2D* m_DepthStencilBuffer = nullptr;
	Texture* m_BackBufferTexture = nullptr;
	DeviceDesc m_Desc;
};

} // namespace grom
