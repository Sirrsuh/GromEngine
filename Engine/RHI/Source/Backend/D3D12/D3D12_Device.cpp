#include "RHI/Backend/D3D12/D3D12_Device.h"
#include "RHI/Backend/D3D12/D3D12_Texture.h"
#include "RHI/Backend/D3D12/D3D12_Shader.h"
#include "RHI/Backend/D3D12/D3D12_Buffer.h"
#include "RHI/Backend/D3D12/D3D12_Pipeline.h"
#include <cassert>
#include <vector>

namespace grom {

D3D12Device::~D3D12Device()
{
    WaitForGpu();
}

ERenderAPI D3D12Device::GetAPI() { return ERenderAPI::D3D12; }
void* D3D12Device::GetNativeDevice() { return m_Device.Get(); }
void* D3D12Device::GetNativeContext() { return m_CommandList.Get(); }
DeviceDesc& D3D12Device::GetDesc() { return m_Desc; }

D3D12Device* D3D12Device::Create(DeviceDesc& desc)
{
    D3D12Device* device = new D3D12Device();
    device->m_Desc = desc;

    if (!device->CreateDevice()) { delete device; return nullptr; }
    if (!device->CreateCommandQueue()) { delete device; return nullptr; }
    if (!device->CreateSwapChain()) { delete device; return nullptr; }
    if (!device->CreateDescriptorHeaps()) { delete device; return nullptr; }
    if (!device->CreateCommandAllocators()) { delete device; return nullptr; }
    if (!device->CreateCommandList()) { delete device; return nullptr; }
    if (!device->CreateFence()) { delete device; return nullptr; }
    device->CreateRenderTargetViews();

    return device;
}

bool D3D12Device::CreateDevice()
{
    UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
    Microsoft::WRL::ComPtr<ID3D12Debug> debugController;
    if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
        debugController->EnableDebugLayer();
        dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory))))
        return false;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    for (UINT adapterIndex = 0; DXGI_ERROR_NOT_FOUND != factory->EnumAdapters1(adapterIndex, &hardwareAdapter); ++adapterIndex) {
        DXGI_ADAPTER_DESC1 desc;
        hardwareAdapter->GetDesc1(&desc);
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            continue;

        if (SUCCEEDED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), nullptr)))
            break;
        hardwareAdapter.Reset();
    }

    if (!hardwareAdapter) {
        if (FAILED(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
            return false;
    } else {
        if (FAILED(D3D12CreateDevice(hardwareAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_Device))))
            return false;
    }

    return true;
}

bool D3D12Device::CreateCommandQueue()
{
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    return SUCCEEDED(m_Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_CommandQueue)));
}

bool D3D12Device::CreateSwapChain()
{
    Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
    CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = m_NumFrames;
    swapChainDesc.Width = m_Desc.Width;
    swapChainDesc.Height = m_Desc.Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
    if (FAILED(factory->CreateSwapChainForHwnd(m_CommandQueue.Get(), static_cast<HWND>(m_Desc.WindowHandle), &swapChainDesc, nullptr, nullptr, &swapChain)))
        return false;

    if (FAILED(swapChain.As(&m_SwapChain)))
        return false;

    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    return true;
}

bool D3D12Device::CreateDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = m_NumFrames;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_RtvHeap))))
        return false;
    m_RtvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
    dsvHeapDesc.NumDescriptors = 1;
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    if (FAILED(m_Device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_DsvHeap))))
        return false;
    m_DsvDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

    D3D12_DESCRIPTOR_HEAP_DESC cbvSrvUavHeapDesc = {};
    cbvSrvUavHeapDesc.NumDescriptors = 1024;
    cbvSrvUavHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    cbvSrvUavHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_Device->CreateDescriptorHeap(&cbvSrvUavHeapDesc, IID_PPV_ARGS(&m_CbvSrvUavHeap))))
        return false;
    m_CbvSrvUavDescriptorSize = m_Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    return true;
}

bool D3D12Device::CreateCommandAllocators()
{
    return SUCCEEDED(m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_CommandAllocator)));
}

bool D3D12Device::CreateCommandList()
{
    if (FAILED(m_Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_CommandList))))
        return false;
    m_CommandList->Close();
    return true;
}

bool D3D12Device::CreateFence()
{
    if (FAILED(m_Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence))))
        return false;
    m_FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    return m_FenceEvent != nullptr;
}

void D3D12Device::CreateRenderTargetViews()
{
    m_RenderTargets.resize(m_NumFrames);
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_RtvHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT i = 0; i < m_NumFrames; ++i) {
        if (FAILED(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&m_RenderTargets[i]))))
            return;
        m_Device->CreateRenderTargetView(m_RenderTargets[i].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_RtvDescriptorSize);
    }

    D3D12_RESOURCE_DESC depthDesc = {};
    depthDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthDesc.Width = m_Desc.Width;
    depthDesc.Height = m_Desc.Height;
    depthDesc.DepthOrArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

    D3D12_HEAP_PROPERTIES heapProps = {};
    heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_D32_FLOAT;
    clearValue.DepthStencil.Depth = 1.0f;

    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &depthDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&m_DepthStencilBuffer));

    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_DsvHeap->GetCPUDescriptorHandleForHeapStart());
    m_Device->CreateDepthStencilView(m_DepthStencilBuffer.Get(), nullptr, dsvHandle);

    m_Viewport = { 0.0f, 0.0f, static_cast<float>(m_Desc.Width), static_cast<float>(m_Desc.Height), 0.0f, 1.0f };
    m_ScissorRect = { 0, 0, static_cast<LONG>(m_Desc.Width), static_cast<LONG>(m_Desc.Height) };
}

void D3D12Device::WaitForGpu()
{
    if (m_CommandQueue && m_Fence) {
        m_FenceValue++;
        m_CommandQueue->Signal(m_Fence.Get(), m_FenceValue);
        if (m_Fence->GetCompletedValue() < m_FenceValue) {
            m_Fence->SetEventOnCompletion(m_FenceValue, m_FenceEvent);
            WaitForSingleObject(m_FenceEvent, INFINITE);
        }
    }
}

void D3D12Device::Present()
{
    m_SwapChain->Present(1, 0);
    WaitForGpu();
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
}

void D3D12Device::Resize(u32 w, u32 h)
{
    m_Desc.Width = w;
    m_Desc.Height = h;
    WaitForGpu();
    for (UINT i = 0; i < m_NumFrames; ++i) m_RenderTargets[i].Reset();
    m_DepthStencilBuffer.Reset();
    m_SwapChain->ResizeBuffers(m_NumFrames, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    m_FrameIndex = m_SwapChain->GetCurrentBackBufferIndex();
    CreateRenderTargetViews();
}

void D3D12Device::ClearRenderTarget(Texture* rt, const f32 color[4])
{
    // Implementation would use command list to clear RTV
}

void D3D12Device::ClearDepthStencil(Texture* ds, f32 depth, u8 stencil)
{
    // Implementation would use command list to clear DSV
}

void D3D12Device::SetViewport(ViewportDesc& vp)
{
    m_Viewport = { vp.X, vp.Y, vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth };
}

void D3D12Device::SetScissorRect(Rect2D& rect)
{
    m_ScissorRect = { rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height };
}

void D3D12Device::SetPipeline(Pipeline* pipeline)
{
    if (pipeline) {
        D3D12Pipeline* d3d12Pipeline = static_cast<D3D12Pipeline*>(pipeline);
        m_CurrentPipelineState = d3d12Pipeline->GetPSO();
        m_CurrentRootSignature = d3d12Pipeline->GetRootSignature();
    }
}

void D3D12Device::SetVertexBuffer(Buffer* buffer, u32 slot)
{
    // Implementation
}

void D3D12Device::SetIndexBuffer(Buffer* buffer)
{
    // Implementation
}

void D3D12Device::SetConstantBuffer(Buffer* buffer, u32 slot, EShaderType shader)
{
    // Implementation
}

void D3D12Device::SetRenderTargets(Texture* const* renderTargets, u32 count, Texture* depthStencil)
{
    // Implementation
}

void D3D12Device::SetShaderResource(Texture* texture, u32 slot, EShaderType shader)
{
    // Implementation
}

void D3D12Device::SetUnorderedAccessView(Texture* texture, u32 slot, EShaderType shader)
{
    // Implementation
}

void D3D12Device::SetBlendState(bool enable, EBlendFactor srcFactor, EBlendFactor dstFactor)
{
    // Implementation
}

void D3D12Device::Draw(u32 vertexCount, u32 startVertex)
{
    // Implementation
}

void D3D12Device::DrawIndexed(u32 indexCount, u32 startIndex, u32 baseVertex)
{
    // Implementation
}

void D3D12Device::Dispatch(u32 groupX, u32 groupY, u32 groupZ)
{
    // Implementation
}

void D3D12Device::BeginFrame()
{
    m_CommandAllocator->Reset();
    m_CommandList->Reset(m_CommandAllocator.Get(), m_CurrentPipelineState);
    m_CommandList->SetGraphicsRootSignature(m_CurrentRootSignature);
    m_CommandList->RSSetViewports(1, &m_Viewport);
    m_CommandList->RSSetScissorRects(1, &m_ScissorRect);
}

void D3D12Device::EndFrame()
{
    m_CommandList->Close();
    ID3D12CommandList* ppCommandLists[] = { m_CommandList.Get() };
    m_CommandQueue->ExecuteCommandLists(1, ppCommandLists);
}

Texture* D3D12Device::GetBackBuffer()
{
    return m_BackBufferTexture;
}

} // namespace grom