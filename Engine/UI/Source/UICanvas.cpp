#include <UI/UICanvas.h>
#include <cstring>

namespace grom
{

struct UIVertex
{
    GVec2 Position;
    GVec2 TexCoord;
    GColor Color;
};

static const char* s_UIVertexShaderSrc = R"(
struct VSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = float4(input.Position, 0.0f, 1.0f);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    return output;
}
)";

static const char* s_UIPixelShaderSrc = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

float4 main(PSInput input) : SV_TARGET
{
    return input.Color;
}
)";

UICanvas::UICanvas()
    : m_RenderTarget(nullptr)
    , m_DepthStencil(nullptr)
    , m_Pipeline(nullptr)
    , m_VertexBuffer(nullptr)
    , m_IndexBuffer(nullptr)
    , m_VertexShader(nullptr)
    , m_PixelShader(nullptr)
    , m_Width(0)
    , m_Height(0)
    , m_Initialized(false)
    , m_HoveredWidget(nullptr)
    , m_PressedWidget(nullptr)
{
}

UICanvas::~UICanvas()
{
    if (m_RenderTarget) m_RenderTarget->Release();
    if (m_DepthStencil) m_DepthStencil->Release();
    if (m_Pipeline) m_Pipeline->Release();
    if (m_VertexBuffer) m_VertexBuffer->Release();
    if (m_IndexBuffer) m_IndexBuffer->Release();
    if (m_VertexShader) m_VertexShader->Release();
    if (m_PixelShader) m_PixelShader->Release();
}

bool UICanvas::Initialize(Device* device, u32 width, u32 height)
{
    if (!device) return false;

    m_Width = width;
    m_Height = height;

    ERenderAPI api = device->GetAPI();

    TextureDesc rtDesc = {};
    rtDesc.Width = width;
    rtDesc.Height = height;
    rtDesc.Format = EFormat::R8G8B8A8_UNORM;
    rtDesc.IsRenderTarget = true;
    rtDesc.DebugName = "UIRenderTarget";
    m_RenderTarget = Texture::Create(rtDesc, api);
    if (!m_RenderTarget) return false;

    TextureDesc dsDesc = {};
    dsDesc.Width = width;
    dsDesc.Height = height;
    dsDesc.Format = EFormat::D32_FLOAT;
    dsDesc.IsDepthStencil = true;
    dsDesc.DebugName = "UIDepthStencil";
    m_DepthStencil = Texture::Create(dsDesc, api);
    if (!m_DepthStencil) return false;

    ShaderDesc vsDesc = {};
    vsDesc.EntryPoint = "main";
    vsDesc.Source = s_UIVertexShaderSrc;
    vsDesc.Type = EShaderType::Vertex;
    vsDesc.Format = EFormat::Unknown;
    m_VertexShader = Shader::Create(vsDesc, api);
    if (!m_VertexShader) return false;

    ShaderDesc psDesc = {};
    psDesc.EntryPoint = "main";
    psDesc.Source = s_UIPixelShaderSrc;
    psDesc.Type = EShaderType::Pixel;
    psDesc.Format = EFormat::Unknown;
    m_PixelShader = Shader::Create(psDesc, api);
    if (!m_PixelShader) return false;

    BufferLayout layout = {};
    layout.Stride = sizeof(UIVertex);
    layout.Elements.Reserve(3);

    BufferLayout::Element posElem = {};
    posElem.Name = "POSITION";
    posElem.Format = EFormat::R32G32_FLOAT;
    posElem.Offset = 0;
    posElem.Slot = 0;
    layout.Elements.Add(posElem);

    BufferLayout::Element texElem = {};
    texElem.Name = "TEXCOORD0";
    texElem.Format = EFormat::R32G32_FLOAT;
    texElem.Offset = 8;
    texElem.Slot = 0;
    layout.Elements.Add(texElem);

    BufferLayout::Element colorElem = {};
    colorElem.Name = "COLOR0";
    colorElem.Format = EFormat::R8G8B8A8_UNORM;
    colorElem.Offset = 16;
    colorElem.Slot = 0;
    layout.Elements.Add(colorElem);

    PipelineDesc pipeDesc = {};
    pipeDesc.VS = m_VertexShader;
    pipeDesc.PS = m_PixelShader;
    pipeDesc.InputLayout = layout;
    pipeDesc.Rasterizer.CullMode = ECullMode::None;
    pipeDesc.Rasterizer.Wireframe = false;
    pipeDesc.DepthStencil.DepthEnable = false;
    pipeDesc.DepthStencil.DepthWrite = false;
    pipeDesc.Blend.Enable = true;
    pipeDesc.Blend.SrcFactor = EBlendFactor::SrcAlpha;
    pipeDesc.Blend.DstFactor = EBlendFactor::InvSrcAlpha;
    pipeDesc.Blend.SrcFactorAlpha = EBlendFactor::One;
    pipeDesc.Blend.DstFactorAlpha = EBlendFactor::InvSrcAlpha;
    pipeDesc.Topology = EPrimitiveTopology::TriangleList;

    m_Pipeline = Pipeline::Create(pipeDesc, api);
    if (!m_Pipeline) return false;

    u32 maxVerts = 16384;
    u32 maxIndices = 24576;

    BufferDesc vbDesc = {};
    vbDesc.Size = maxVerts * sizeof(UIVertex);
    vbDesc.Stride = sizeof(UIVertex);
    vbDesc.Type = EBufferType::Vertex;
    vbDesc.Usage = EBufferUsage::Dynamic;
    vbDesc.DebugName = "UIVertexBuffer";
    m_VertexBuffer = Buffer::Create(vbDesc, api);
    if (!m_VertexBuffer) return false;

    BufferDesc ibDesc = {};
    ibDesc.Size = maxIndices * sizeof(u32);
    ibDesc.Stride = sizeof(u32);
    ibDesc.Type = EBufferType::Index;
    ibDesc.Usage = EBufferUsage::Dynamic;
    ibDesc.DebugName = "UIIndexBuffer";
    m_IndexBuffer = Buffer::Create(ibDesc, api);
    if (!m_IndexBuffer) return false;

    m_Size = GVec2(static_cast<f32>(width), static_cast<f32>(height));
    m_Initialized = true;
    return true;
}

void UICanvas::Resize(u32 width, u32 height)
{
    if (!m_Initialized) return;

    m_Width = width;
    m_Height = height;
    m_Size = GVec2(static_cast<f32>(width), static_cast<f32>(height));

    if (m_RenderTarget)
    {
        m_RenderTarget->Release();
        m_RenderTarget = nullptr;
    }
    if (m_DepthStencil)
    {
        m_DepthStencil->Release();
        m_DepthStencil = nullptr;
    }

    ERenderAPI api = Device::GetActiveDevice()->GetAPI();

    TextureDesc rtDesc = {};
    rtDesc.Width = width;
    rtDesc.Height = height;
    rtDesc.Format = EFormat::R8G8B8A8_UNORM;
    rtDesc.IsRenderTarget = true;
    rtDesc.DebugName = "UIRenderTarget";
    m_RenderTarget = Texture::Create(rtDesc, api);

    TextureDesc dsDesc = {};
    dsDesc.Width = width;
    dsDesc.Height = height;
    dsDesc.Format = EFormat::D32_FLOAT;
    dsDesc.IsDepthStencil = true;
    dsDesc.DebugName = "UIDepthStencil";
    m_DepthStencil = Texture::Create(dsDesc, api);
}

void UICanvas::Render(Device* device)
{
    if (!m_Initialized || !device) return;

    const f32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    device->ClearRenderTarget(m_RenderTarget, clearColor);
    device->ClearDepthStencil(m_DepthStencil, 1.0f, 0);

    ViewportDesc vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width = static_cast<f32>(m_Width);
    vp.Height = static_cast<f32>(m_Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    device->SetViewport(vp);

    Rect2D scissor = {};
    scissor.Left = 0;
    scissor.Top = 0;
    scissor.Right = static_cast<i32>(m_Width);
    scissor.Bottom = static_cast<i32>(m_Height);
    device->SetScissorRect(scissor);

    device->SetPipeline(m_Pipeline);

    UIElement::Render(this);
}

void UICanvas::Update(f32 deltaTime)
{
    UIElement::Update(deltaTime);
}

void UICanvas::DispatchEvent(const GVec2& mousePos, bool mouseDown, bool mouseUp)
{
    UIElement* hit = HitTest(mousePos);

    UIWidget* widget = nullptr;
    if (hit)
    {
        widget = dynamic_cast<UIWidget*>(hit);
    }

    if (m_HoveredWidget && m_HoveredWidget != widget)
    {
        m_HoveredWidget->OnMouseLeave();
        m_HoveredWidget = nullptr;
    }

    if (widget && !m_HoveredWidget)
    {
        m_HoveredWidget = widget;
        m_HoveredWidget->OnMouseEnter(mousePos);
    }

    if (m_HoveredWidget)
    {
        m_HoveredWidget->OnMouseMove(mousePos);
    }

    if (mouseDown && widget)
    {
        m_PressedWidget = widget;
        m_PressedWidget->OnMouseDown(mousePos);
    }

    if (mouseUp && m_PressedWidget)
    {
        m_PressedWidget->OnMouseUp(mousePos);

        if (m_PressedWidget == widget)
        {
            m_PressedWidget->OnClick();
        }
        m_PressedWidget = nullptr;
    }
}

static UIElement* HitTestRecursive(UIElement* element, const GVec2& point)
{
    if (!element->IsVisible() || !element->ContainsPoint(point)) return nullptr;

    const TArray<UIElement*>& kids = element->GetChildren();
    for (usize i = kids.Size(); i > 0; --i)
    {
        UIElement* result = HitTestRecursive(kids[i - 1], point);
        if (result) return result;
    }

    return element;
}

UIElement* UICanvas::HitTest(const GVec2& point)
{
    for (usize i = m_Children.Size(); i > 0; --i)
    {
        UIElement* result = HitTestRecursive(m_Children[i - 1], point);
        if (result) return result;
    }
    return nullptr;
}

}
