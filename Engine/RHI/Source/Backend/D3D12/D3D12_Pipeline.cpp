#include "RHI/Backend/D3D12/D3D12_Pipeline.h"
#include "RHI/Backend/D3D12/D3D12_Shader.h"
#include "RHI/Backend/D3D12/D3D12_Device.h"
#include <vector>

using Microsoft::WRL::ComPtr;

namespace grom {

D3D12Pipeline::~D3D12Pipeline()
{
}

void* D3D12Pipeline::GetHandle() { return m_PSO.Get(); }
PipelineDesc& D3D12Pipeline::GetDesc() { return m_Desc; }

static D3D12_BLEND ConvertBlendFactor(EBlendFactor factor)
{
    switch (factor) {
    case EBlendFactor::One:           return D3D12_BLEND_ONE;
    case EBlendFactor::Zero:          return D3D12_BLEND_ZERO;
    case EBlendFactor::SrcAlpha:      return D3D12_BLEND_SRC_ALPHA;
    case EBlendFactor::InvSrcAlpha:   return D3D12_BLEND_INV_SRC_ALPHA;
    default: return D3D12_BLEND_ONE;
    }
}

static D3D12_COMPARISON_FUNC ConvertCompareOp(ECompareOp op)
{
    switch (op) {
    case ECompareOp::Less:        return D3D12_COMPARISON_FUNC_LESS;
    case ECompareOp::LessEqual:   return D3D12_COMPARISON_FUNC_LESS_EQUAL;
    case ECompareOp::Greater:     return D3D12_COMPARISON_FUNC_GREATER;
    case ECompareOp::GreaterEqual: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
    case ECompareOp::Equal:       return D3D12_COMPARISON_FUNC_EQUAL;
    case ECompareOp::NotEqual:    return D3D12_COMPARISON_FUNC_NOT_EQUAL;
    case ECompareOp::Always:      return D3D12_COMPARISON_FUNC_ALWAYS;
    case ECompareOp::Never:       return D3D12_COMPARISON_FUNC_NEVER;
    default: return D3D12_COMPARISON_FUNC_LESS;
    }
}

static DXGI_FORMAT ConvertFormat(EFormat fmt)
{
    switch (fmt) {
    case EFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case EFormat::R8G8B8A8_SRGB:  return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case EFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case EFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case EFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case EFormat::D32_FLOAT:      return DXGI_FORMAT_D32_FLOAT;
    case EFormat::R32_FLOAT:      return DXGI_FORMAT_R32_FLOAT;
    case EFormat::R8_UNORM:       return DXGI_FORMAT_R8_UNORM;
    default: return DXGI_FORMAT_UNKNOWN;
    }
}

D3D12Pipeline* D3D12Pipeline::Create(PipelineDesc& desc, ID3D12Device* device, D3D12Device* /*d3d12Device*/)
{
    D3D12Pipeline* pipeline = new D3D12Pipeline();
    pipeline->m_Desc = desc;

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;

    if (desc.InputLayout.Elements.Size() > 0) {
        auto& layout = desc.InputLayout;
        for (u32 i = 0; i < layout.Elements.Size(); ++i) {
            auto& elem = layout.Elements[i];
            DXGI_FORMAT fmt = ConvertFormat(elem.Format);

            D3D12_INPUT_ELEMENT_DESC inputElem = {};
            inputElem.SemanticName = elem.Name.c_str();
            inputElem.SemanticIndex = 0;
            inputElem.Format = fmt;
            inputElem.InputSlot = elem.Slot;
            inputElem.AlignedByteOffset = elem.Offset;
            inputElem.InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
            inputElem.InstanceDataStepRate = 0;
            inputElements.push_back(inputElem);
        }
    }

    // Create root signature
    D3D12_ROOT_SIGNATURE_DESC rootSigDesc = {};
    rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_ROOT_PARAMETER rootParams[4] = {};
    UINT numRootParams = 0;

    // CBV slot 0
    rootParams[numRootParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParams[numRootParams].Descriptor.ShaderRegister = 0;
    rootParams[numRootParams].Descriptor.RegisterSpace = 0;
    rootParams[numRootParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    numRootParams++;

    // SRV slot 0
    rootParams[numRootParams].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
    rootParams[numRootParams].Descriptor.ShaderRegister = 0;
    rootParams[numRootParams].Descriptor.RegisterSpace = 0;
    rootParams[numRootParams].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    numRootParams++;

    // Sampler slot 0
    D3D12_STATIC_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    samplerDesc.MipLODBias = 0;
    samplerDesc.MaxAnisotropy = 16;
    samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
    samplerDesc.ShaderRegister = 0;
    samplerDesc.RegisterSpace = 0;
    samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    rootSigDesc.NumParameters = numRootParams;
    rootSigDesc.pParameters = rootParams;
    rootSigDesc.NumStaticSamplers = 1;
    rootSigDesc.pStaticSamplers = &samplerDesc;

    ComPtr<ID3DBlob> serializedRootSig;
    ComPtr<ID3DBlob> errorBlob;
    HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &serializedRootSig, &errorBlob);
    if (FAILED(hr)) {
        if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        delete pipeline;
        return nullptr;
    }

    hr = device->CreateRootSignature(0, serializedRootSig->GetBufferPointer(), serializedRootSig->GetBufferSize(), IID_PPV_ARGS(&pipeline->m_RootSignature));
    if (FAILED(hr)) {
        delete pipeline;
        return nullptr;
    }

    // Create PSO
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = pipeline->m_RootSignature.Get();

    // Shaders
    if (desc.VS) {
        D3D12Shader* vs = static_cast<D3D12Shader*>(desc.VS);
        psoDesc.VS = { vs->GetBytecode()->GetBufferPointer(), vs->GetBytecode()->GetBufferSize() };
    }
    if (desc.PS) {
        D3D12Shader* ps = static_cast<D3D12Shader*>(desc.PS);
        psoDesc.PS = { ps->GetBytecode()->GetBufferPointer(), ps->GetBytecode()->GetBufferSize() };
    }

    // Input layout
    psoDesc.InputLayout = { inputElements.data(), (UINT)inputElements.size() };

    // Primitive topology
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pipeline->m_PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

    // Rasterizer
    psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
    psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
    psoDesc.RasterizerState.FrontCounterClockwise = FALSE;
    psoDesc.RasterizerState.DepthBias = 0;
    psoDesc.RasterizerState.DepthBiasClamp = 0.0f;
    psoDesc.RasterizerState.SlopeScaledDepthBias = 0.0f;
    psoDesc.RasterizerState.DepthClipEnable = TRUE;
    psoDesc.RasterizerState.MultisampleEnable = FALSE;
    psoDesc.RasterizerState.AntialiasedLineEnable = FALSE;
    psoDesc.RasterizerState.ForcedSampleCount = 0;
    psoDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

    // Depth-stencil
    psoDesc.DepthStencilState.DepthEnable = desc.DepthStencil.DepthEnable;
    psoDesc.DepthStencilState.DepthWriteMask = desc.DepthStencil.DepthWrite ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
    psoDesc.DepthStencilState.DepthFunc = ConvertCompareOp(desc.DepthStencil.DepthFunc);
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.DSVFormat = ConvertFormat(EFormat::D32_FLOAT);

    // Blend
    D3D12_RENDER_TARGET_BLEND_DESC rtBlendDesc = {};
    rtBlendDesc.BlendEnable = desc.Blend.Enable;
    rtBlendDesc.LogicOpEnable = FALSE;
    rtBlendDesc.SrcBlend = ConvertBlendFactor(desc.Blend.SrcFactor);
    rtBlendDesc.DestBlend = ConvertBlendFactor(desc.Blend.DstFactor);
    rtBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
    rtBlendDesc.SrcBlendAlpha = ConvertBlendFactor(desc.Blend.SrcFactorAlpha);
    rtBlendDesc.DestBlendAlpha = ConvertBlendFactor(desc.Blend.DstFactorAlpha);
    rtBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
    rtBlendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
    rtBlendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    psoDesc.BlendState.IndependentBlendEnable = FALSE;
    psoDesc.BlendState.RenderTarget[0] = rtBlendDesc;

    psoDesc.SampleMask = UINT_MAX;
    psoDesc.RTVFormats[0] = ConvertFormat(EFormat::R8G8B8A8_UNORM);
    psoDesc.NumRenderTargets = 1;
    psoDesc.SampleDesc.Count = 1;

    hr = device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipeline->m_PSO));
    if (FAILED(hr)) {
        delete pipeline;
        return nullptr;
    }

    return pipeline;
}

} // namespace grom