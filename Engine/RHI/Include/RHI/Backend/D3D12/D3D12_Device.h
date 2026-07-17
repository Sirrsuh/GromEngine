#pragma once
#include "../../RHI_Device.h"
#include "../../RHI_Texture.h"
#include "../../RHI_Shader.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace grom {

class D3D12Device : public Device
{
public:
    D3D12Device() = default;
    ~D3D12Device() override;

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
    void SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil) override;
    void SetShaderResource(Texture* texture, u32 slot, EShaderType shader) override;
    void SetUnorderedAccessView(Texture* texture, u32 slot, EShaderType shader) override;
    void SetBlendState(bool enable, EBlendFactor srcFactor, EBlendFactor dstFactor) override;
    void Draw(u32 vertexCount, u32 startVertex) override;
    void DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex) override;
    void Dispatch(u32 groupX, u32 groupY, u32 groupZ) override;
    void BeginFrame() override;
    void EndFrame() override;
    Texture* GetBackBuffer() override;

    static D3D12Device* Create(DeviceDesc& desc);

    ID3D12Device* GetD3D12Device() const { return m_Device.Get(); }
    ID3D12CommandQueue* GetCommandQueue() const { return m_CommandQueue.Get(); }
    IDXGISwapChain4* GetSwapChain() const { return m_SwapChain.Get(); }

private:
    bool CreateDevice();
    bool CreateCommandQueue();
    bool CreateSwapChain();
    bool CreateDescriptorHeaps();
    bool CreateCommandAllocators();
    bool CreateCommandList();
    bool CreateFence();
    void CreateRenderTargetViews();
    void WaitForGpu();

    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_CommandQueue;
    Microsoft::WRL::ComPtr<IDXGISwapChain4> m_SwapChain;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_RtvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_DsvHeap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_CbvSrvUavHeap;
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_CommandAllocator;
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
    Microsoft::WRL::ComPtr<ID3D12Fence> m_Fence;
    HANDLE m_FenceEvent = nullptr;
    UINT64 m_FenceValue = 1;
    UINT m_RtvDescriptorSize = 0;
    UINT m_DsvDescriptorSize = 0;
    UINT m_CbvSrvUavDescriptorSize = 0;

    std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> m_RenderTargets;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_DepthStencilBuffer;
    DeviceDesc m_Desc;
    Texture* m_BackBufferTexture = nullptr;
    UINT m_FrameIndex = 0;
    UINT m_NumFrames = 2;
    D3D12_VIEWPORT m_Viewport = {};
    D3D12_RECT m_ScissorRect = {};
    ID3D12PipelineState* m_CurrentPipelineState = nullptr;
    ID3D12RootSignature* m_CurrentRootSignature = nullptr;
};

} // namespace grom