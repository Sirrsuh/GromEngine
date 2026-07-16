#pragma once

#include "RHI_RefCounted.h"
#include "RHI_Enums.h"

namespace grom
{

class Shader;
class Buffer;
class Texture;
class Pipeline;

struct DeviceDesc
{
	ERenderAPI API = ERenderAPI::None;
	void* WindowHandle = nullptr;
	u32 Width = 0;
	u32 Height = 0;
	bool VSync = true;
	bool Fullscreen = false;
	bool DebugMode = false;
};

class Device : public RefCounted
{
public:
	virtual ERenderAPI GetAPI() = 0;
	virtual void* GetNativeDevice() = 0;
	virtual void* GetNativeContext() = 0;
	virtual DeviceDesc& GetDesc() = 0;
	virtual void Present() = 0;
	virtual void Resize(u32 w, u32 h) = 0;
	virtual void ClearRenderTarget(Texture* rt, const f32 color[4]) = 0;
	virtual void ClearDepthStencil(Texture* ds, f32 depth, u8 stencil) = 0;
	virtual void SetViewport(ViewportDesc& vp) = 0;
	virtual void SetScissorRect(Rect2D& rect) = 0;
	virtual void SetPipeline(Pipeline* pipeline) = 0;
	virtual void SetVertexBuffer(Buffer* buffer, u32 slot) = 0;
	virtual void SetIndexBuffer(Buffer* buffer) = 0;
	virtual void SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader) = 0;
	virtual void SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil) = 0;
	virtual void SetShaderResource(Texture* texture, u32 slot, EShaderType shader) = 0;
	virtual void Draw(u32 vertexCount, u32 startVertex = 0) = 0;
	virtual void DrawIndexed(u32 indexCount, u32 startIndex = 0, u32 baseVertex = 0) = 0;
	virtual void Dispatch(u32 groupX, u32 groupY, u32 groupZ) = 0;
	virtual void BeginFrame() = 0;
	virtual void EndFrame() = 0;
	virtual Texture* GetBackBuffer() = 0;
	virtual ~Device() = default;

	GROM_RHI_API static Device* Create(DeviceDesc& desc);
	GROM_RHI_API static Device* GetActiveDevice();
};

} // namespace grom
