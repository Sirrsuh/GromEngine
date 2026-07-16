#include <Renderer/Renderer.h>
#include <algorithm>
#include <cstring>
#include <cstdio>
#include <vector>

namespace grom {

static GString ReadShaderFile(const char* path)
{
    FILE* f = nullptr;
    fopen_s(&f, path, "rb");
    if (!f) return GString();
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::vector<char> buf(static_cast<usize>(len) + 1);
    fread(buf.data(), 1, static_cast<usize>(len), f);
    fclose(f);
    buf[static_cast<usize>(len)] = 0;
    return GString(buf.data());
}

static const char* s_FullscreenVSSrc = R"(
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD0;
};
VSOutput VSMain(uint vertexID : SV_VertexID)
{
    VSOutput output;
    output.UV = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(output.UV * 2.0 - 1.0, 0.0, 1.0);
    return output;
}
)";

Renderer::Renderer() = default;
Renderer::~Renderer() = default;

bool Renderer::Initialize(Device* device, u32 width, u32 height)
{
    if (!device) return false;

    m_Width = width;
    m_Height = height;

    ERenderAPI api = device->GetAPI();

    if (!CreateGBuffer(width, height)) return false;

    ShaderDesc vsDesc = {};
    vsDesc.EntryPoint = "VSMain";
    vsDesc.Type = EShaderType::Vertex;
    vsDesc.Format = EFormat::Unknown;

    ShaderDesc psDesc = {};
    psDesc.EntryPoint = "PSMain";
    psDesc.Type = EShaderType::Pixel;
    psDesc.Format = EFormat::Unknown;

    // Fullscreen VS (inline)
    {
        ShaderDesc fsVSDesc = {};
        fsVSDesc.EntryPoint = "VSMain";
        fsVSDesc.Source = s_FullscreenVSSrc;
        fsVSDesc.Type = EShaderType::Vertex;
        fsVSDesc.Format = EFormat::Unknown;
        m_FullscreenVS = Shader::Create(fsVSDesc, api);
        if (!m_FullscreenVS) return false;
    }

    // GBuffer shaders from file
    {
        GString gbufferVSSrc = ReadShaderFile("Engine/Shaders/HLSL/GBuffer_VS.hlsl");
        vsDesc.Source = gbufferVSSrc;
        m_GBufferVS = Shader::Create(vsDesc, api);
        if (!m_GBufferVS) return false;

        GString gbufferPSSrc = ReadShaderFile("Engine/Shaders/HLSL/GBuffer_PS.hlsl");
        psDesc.Source = gbufferPSSrc;
        m_GBufferPS = Shader::Create(psDesc, api);
        if (!m_GBufferPS) return false;
    }

    // Deferred lighting shaders
    {
        GString defPSSrc = ReadShaderFile("Engine/Shaders/HLSL/DeferredLighting_PS.hlsl");
        psDesc.Source = defPSSrc;
        m_DeferredLightingPS = Shader::Create(psDesc, api);
        if (!m_DeferredLightingPS) return false;
    }

    // Skybox shaders from file
    {
        GString skyboxVSSrc = ReadShaderFile("Engine/Shaders/HLSL/Skybox_VS.hlsl");
        vsDesc.Source = skyboxVSSrc;
        m_SkyboxVS = Shader::Create(vsDesc, api);
        if (!m_SkyboxVS) return false;

        GString skyboxPSSrc = ReadShaderFile("Engine/Shaders/HLSL/Skybox_PS.hlsl");
        psDesc.Source = skyboxPSSrc;
        m_SkyboxPS = Shader::Create(psDesc, api);
        if (!m_SkyboxPS) return false;
    }

    // Constant buffers
    {
        BufferDesc frameCB = {};
        frameCB.Size = sizeof(FrameData);
        frameCB.Stride = 0;
        frameCB.Type = EBufferType::Constant;
        frameCB.Usage = EBufferUsage::Dynamic;
        frameCB.DebugName = "FrameConstants";
        m_FrameConstantBuffer = Buffer::Create(frameCB, api);
        if (!m_FrameConstantBuffer) return false;
    }

    {
        BufferDesc objCB = {};
        objCB.Size = sizeof(ObjectData);
        objCB.Stride = 0;
        objCB.Type = EBufferType::Constant;
        objCB.Usage = EBufferUsage::Dynamic;
        objCB.DebugName = "ObjectConstants";
        m_ObjectConstantBuffer = Buffer::Create(objCB, api);
        if (!m_ObjectConstantBuffer) return false;
    }

    {
        BufferDesc lightCB = {};
        lightCB.Size = sizeof(LightData);
        lightCB.Stride = 0;
        lightCB.Type = EBufferType::Constant;
        lightCB.Usage = EBufferUsage::Dynamic;
        lightCB.DebugName = "LightConstants";
        m_LightConstantBuffer = Buffer::Create(lightCB, api);
        if (!m_LightConstantBuffer) return false;
    }

    if (!CreatePipelines()) return false;

    m_Initialized = true;
    return true;
}

bool Renderer::CreateGBuffer(u32 width, u32 height)
{
    ERenderAPI api = Device::GetActiveDevice()->GetAPI();

    TextureDesc albedoDesc = {};
    albedoDesc.Width = width;
    albedoDesc.Height = height;
    albedoDesc.Format = EFormat::R8G8B8A8_UNORM;
    albedoDesc.IsRenderTarget = true;
    albedoDesc.DebugName = "GBuffer_Albedo";
    m_GBufferAlbedo = Texture::Create(albedoDesc, api);
    if (!m_GBufferAlbedo) return false;

    TextureDesc normalDesc = {};
    normalDesc.Width = width;
    normalDesc.Height = height;
    normalDesc.Format = EFormat::R16G16B16A16_FLOAT;
    normalDesc.IsRenderTarget = true;
    normalDesc.DebugName = "GBuffer_Normal";
    m_GBufferNormal = Texture::Create(normalDesc, api);
    if (!m_GBufferNormal) return false;

    TextureDesc rmaDesc = {};
    rmaDesc.Width = width;
    rmaDesc.Height = height;
    rmaDesc.Format = EFormat::R8G8B8A8_UNORM;
    rmaDesc.IsRenderTarget = true;
    rmaDesc.DebugName = "GBuffer_RMA";
    m_GBufferRMA = Texture::Create(rmaDesc, api);
    if (!m_GBufferRMA) return false;

    TextureDesc emissiveDesc = {};
    emissiveDesc.Width = width;
    emissiveDesc.Height = height;
    emissiveDesc.Format = EFormat::R8G8B8A8_UNORM;
    emissiveDesc.IsRenderTarget = true;
    emissiveDesc.DebugName = "GBuffer_Emissive";
    m_GBufferEmissive = Texture::Create(emissiveDesc, api);
    if (!m_GBufferEmissive) return false;

    TextureDesc dsDesc = {};
    dsDesc.Width = width;
    dsDesc.Height = height;
    dsDesc.Format = EFormat::D24_UNORM_S8_UINT;
    dsDesc.IsDepthStencil = true;
    dsDesc.DebugName = "GBuffer_DepthStencil";
    m_DepthStencil = Texture::Create(dsDesc, api);
    if (!m_DepthStencil) return false;

    return true;
}

bool Renderer::CreatePipelines()
{
    Device* device = Device::GetActiveDevice();
    ERenderAPI api = device->GetAPI();

    // GBuffer pass pipeline (opaque, depth-test, no blend, backface cull)
    {
        BufferLayout layout = {};
        layout.Stride = sizeof(Vertex);
        layout.Elements.Reserve(6);

        BufferLayout::Element posElem = {};
        posElem.Name = "POSITION";
        posElem.Format = EFormat::R32G32B32_FLOAT;
        posElem.Offset = offsetof(Vertex, Position);
        posElem.Slot = 0;
        layout.Elements.Add(posElem);

        BufferLayout::Element normElem = {};
        normElem.Name = "NORMAL";
        normElem.Format = EFormat::R32G32B32_FLOAT;
        normElem.Offset = offsetof(Vertex, Normal);
        normElem.Slot = 0;
        layout.Elements.Add(normElem);

        BufferLayout::Element tanElem = {};
        tanElem.Name = "TANGENT";
        tanElem.Format = EFormat::R32G32B32A32_FLOAT;
        tanElem.Offset = offsetof(Vertex, Tangent);
        tanElem.Slot = 0;
        layout.Elements.Add(tanElem);

        BufferLayout::Element uv0Elem = {};
        uv0Elem.Name = "TEXCOORD0";
        uv0Elem.Format = EFormat::R32G32_FLOAT;
        uv0Elem.Offset = offsetof(Vertex, UV0);
        uv0Elem.Slot = 0;
        layout.Elements.Add(uv0Elem);

        BufferLayout::Element uv1Elem = {};
        uv1Elem.Name = "TEXCOORD1";
        uv1Elem.Format = EFormat::R32G32_FLOAT;
        uv1Elem.Offset = offsetof(Vertex, UV1);
        uv1Elem.Slot = 0;
        layout.Elements.Add(uv1Elem);

        BufferLayout::Element colElem = {};
        colElem.Name = "COLOR";
        colElem.Format = EFormat::R8G8B8A8_UNORM;
        colElem.Offset = offsetof(Vertex, Color);
        colElem.Slot = 0;
        layout.Elements.Add(colElem);

        PipelineDesc desc = {};
        desc.VS = m_GBufferVS;
        desc.PS = m_GBufferPS;
        desc.InputLayout = layout;
        desc.Rasterizer.CullMode = ECullMode::Back;
        desc.Rasterizer.DepthClip = true;
        desc.DepthStencil.DepthEnable = true;
        desc.DepthStencil.DepthWrite = true;
        desc.DepthStencil.DepthFunc = ECompareOp::Less;
        desc.Blend.Enable = false;
        desc.Topology = EPrimitiveTopology::TriangleList;

        m_GBufferPipeline = Pipeline::Create(desc, api);
        if (!m_GBufferPipeline) return false;
    }

    // Deferred lighting fullscreen pass (no depth, no cull, additive blend)
    {
        PipelineDesc desc = {};
        desc.VS = m_FullscreenVS;
        desc.PS = m_DeferredLightingPS;
        desc.Rasterizer.CullMode = ECullMode::None;
        desc.DepthStencil.DepthEnable = false;
        desc.DepthStencil.DepthWrite = false;
        desc.Blend.Enable = false;
        desc.Topology = EPrimitiveTopology::TriangleList;

        m_DeferredLightingPipeline = Pipeline::Create(desc, api);
        if (!m_DeferredLightingPipeline) return false;
    }

    // Skybox pass (no cull, LEQUAL depth)
    {
        BufferLayout skyLayout = {};
        skyLayout.Stride = sizeof(Vertex);
        skyLayout.Elements.Reserve(1);

        BufferLayout::Element posElem = {};
        posElem.Name = "POSITION";
        posElem.Format = EFormat::R32G32B32_FLOAT;
        posElem.Offset = 0;
        posElem.Slot = 0;
        skyLayout.Elements.Add(posElem);

        PipelineDesc desc = {};
        desc.VS = m_SkyboxVS;
        desc.PS = m_SkyboxPS;
        desc.InputLayout = skyLayout;
        desc.Rasterizer.CullMode = ECullMode::None;
        desc.DepthStencil.DepthEnable = true;
        desc.DepthStencil.DepthWrite = false;
        desc.DepthStencil.DepthFunc = ECompareOp::LessEqual;
        desc.Blend.Enable = false;
        desc.Topology = EPrimitiveTopology::TriangleList;

        m_SkyboxPipeline = Pipeline::Create(desc, api);
        if (!m_SkyboxPipeline) return false;
    }

    return true;
}

void Renderer::Resize(u32 width, u32 height)
{
    if (width == 0 || height == 0) return;

    m_Width = width;
    m_Height = height;

    if (m_GBufferAlbedo) { m_GBufferAlbedo->Release(); m_GBufferAlbedo = nullptr; }
    if (m_GBufferNormal) { m_GBufferNormal->Release(); m_GBufferNormal = nullptr; }
    if (m_GBufferRMA) { m_GBufferRMA->Release(); m_GBufferRMA = nullptr; }
    if (m_GBufferEmissive) { m_GBufferEmissive->Release(); m_GBufferEmissive = nullptr; }
    if (m_DepthStencil) { m_DepthStencil->Release(); m_DepthStencil = nullptr; }

    CreateGBuffer(width, height);
}

void Renderer::Shutdown()
{
    if (m_GBufferAlbedo) { m_GBufferAlbedo->Release(); m_GBufferAlbedo = nullptr; }
    if (m_GBufferNormal) { m_GBufferNormal->Release(); m_GBufferNormal = nullptr; }
    if (m_GBufferRMA) { m_GBufferRMA->Release(); m_GBufferRMA = nullptr; }
    if (m_GBufferEmissive) { m_GBufferEmissive->Release(); m_GBufferEmissive = nullptr; }
    if (m_DepthStencil) { m_DepthStencil->Release(); m_DepthStencil = nullptr; }

    if (m_GBufferVS) { m_GBufferVS->Release(); m_GBufferVS = nullptr; }
    if (m_GBufferPS) { m_GBufferPS->Release(); m_GBufferPS = nullptr; }
    if (m_GBufferPipeline) { m_GBufferPipeline->Release(); m_GBufferPipeline = nullptr; }

    if (m_FullscreenVS) { m_FullscreenVS->Release(); m_FullscreenVS = nullptr; }
    if (m_DeferredLightingPS) { m_DeferredLightingPS->Release(); m_DeferredLightingPS = nullptr; }
    if (m_DeferredLightingPipeline) { m_DeferredLightingPipeline->Release(); m_DeferredLightingPipeline = nullptr; }

    if (m_SkyboxVS) { m_SkyboxVS->Release(); m_SkyboxVS = nullptr; }
    if (m_SkyboxPS) { m_SkyboxPS->Release(); m_SkyboxPS = nullptr; }
    if (m_SkyboxPipeline) { m_SkyboxPipeline->Release(); m_SkyboxPipeline = nullptr; }

    if (m_FrameConstantBuffer) { m_FrameConstantBuffer->Release(); m_FrameConstantBuffer = nullptr; }
    if (m_ObjectConstantBuffer) { m_ObjectConstantBuffer->Release(); m_ObjectConstantBuffer = nullptr; }
    if (m_LightConstantBuffer) { m_LightConstantBuffer->Release(); m_LightConstantBuffer = nullptr; }
}

void Renderer::UpdateFrameConstants(Device* device, Camera* camera, f32 deltaTime)
{
    m_TotalTime += deltaTime;
    ++m_FrameCount;

    GMatrix4x4 view = camera->GetViewMatrix();
    GMatrix4x4 proj = camera->GetProjectionMatrix();
    GMatrix4x4 viewProj = camera->GetViewProjectionMatrix();
    GMatrix4x4 invView = view.Inverse();
    GMatrix4x4 invProj = proj.Inverse();
    GMatrix4x4 invViewProj = viewProj.Inverse();

    GVec3 camPos = camera->GetWorldPosition();
    GVec3 camDir = camera->GetForward();

    FrameData data;
    data.ViewMatrix = view;
    data.ProjectionMatrix = proj;
    data.ViewProjectionMatrix = viewProj;
    data.InvViewMatrix = invView;
    data.InvProjectionMatrix = invProj;
    data.InvViewProjectionMatrix = invViewProj;
    data.CameraPosition = GVec4(camPos.x, camPos.y, camPos.z, 1.0f);
    data.CameraDirection = GVec4(camDir.x, camDir.y, camDir.z, 0.0f);
    data.ViewportSize = GVec2(static_cast<f32>(m_Width), static_cast<f32>(m_Height));
    data.ViewportPixelSize = GVec2(1.0f / static_cast<f32>(m_Width), 1.0f / static_cast<f32>(m_Height));
    data.NearPlane = camera->GetNearPlane();
    data.FarPlane = camera->GetFarPlane();
    data.DeltaTime = deltaTime;
    data.TotalTime = static_cast<f32>(m_TotalTime);
    data.FrameCount = m_FrameCount;
    data.bOrthographic = camera->IsOrthographic() ? 1u : 0u;
    data.DummyFrame[0] = 0.0f;
    data.DummyFrame[1] = 0.0f;

    void* mapped = m_FrameConstantBuffer->Map();
    if (mapped)
    {
        std::memcpy(mapped, &data, sizeof(FrameData));
        m_FrameConstantBuffer->Unmap();
    }

    device->SetConstantBuffer(m_FrameConstantBuffer, 0, EShaderType::Vertex);
    device->SetConstantBuffer(m_FrameConstantBuffer, 0, EShaderType::Pixel);
}

void Renderer::UpdateLightConstants(Device* device, Scene* scene)
{
    LightData data;
    data.AmbientColor = GVec4(0.03f, 0.03f, 0.03f, 1.0f);
    data.SunDirection = GVec4(0.0f, -1.0f, 0.0f, 0.0f);
    data.SunColor = GVec4(1.0f, 1.0f, 1.0f, 1.0f);
    data.IBLSpecFactor = GVec4(0.0f, 0.0f, 0.0f, 0.0f);
    data.NumPointLights = 0;
    data.NumSpotLights = 0;
    data.NumDecals = 0;
    data.DummyLight = 0.0f;

    Light* sun = scene->GetSunLight();
    if (sun)
    {
        GColor col = sun->GetColor();
        f32 intensity = sun->GetIntensity();
        GVec3 dir = sun->GetForward();
        data.SunDirection = GVec4(dir.x, dir.y, dir.z, 0.0f);
        data.SunColor = GVec4(
            col.r / 255.0f * intensity,
            col.g / 255.0f * intensity,
            col.b / 255.0f * intensity,
            1.0f
        );
    }

    void* mapped = m_LightConstantBuffer->Map();
    if (mapped)
    {
        std::memcpy(mapped, &data, sizeof(LightData));
        m_LightConstantBuffer->Unmap();
    }

    device->SetConstantBuffer(m_LightConstantBuffer, 3, EShaderType::Pixel);
}

void Renderer::RenderGBufferPass(Device* device, Scene* scene)
{
    Texture* rtvs[4] = { m_GBufferAlbedo, m_GBufferNormal, m_GBufferRMA, m_GBufferEmissive };
    device->SetRenderTargets(rtvs, 4, m_DepthStencil);

    const f32 clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    device->ClearRenderTarget(m_GBufferAlbedo, clearColor);
    device->ClearRenderTarget(m_GBufferNormal, clearColor);
    device->ClearRenderTarget(m_GBufferRMA, clearColor);
    device->ClearRenderTarget(m_GBufferEmissive, clearColor);
    device->ClearDepthStencil(m_DepthStencil, 1.0f, 0);

    ViewportDesc vp = {};
    vp.Width = static_cast<f32>(m_Width);
    vp.Height = static_cast<f32>(m_Height);
    vp.MaxDepth = 1.0f;
    device->SetViewport(vp);

    device->SetPipeline(m_GBufferPipeline);

    Camera* camera = scene->GetActiveCamera();
    if (!camera) return;

    ObjectData objData;
    objData.ObjectID = 0;
    objData.bSelected = 0;
    objData.ObjectColor = GVec4(1.0f, 1.0f, 1.0f, 1.0f);
    objData.DummyObject[0] = 0.0f;
    objData.DummyObject[1] = 0.0f;

    const TArray<MeshComponent*>& comps = scene->GetMeshComponents();
    for (usize i = 0; i < comps.Size(); ++i)
    {
        MeshComponent* comp = comps[i];
        if (!comp || !comp->bVisible || !comp->MeshData) continue;

        objData.WorldMatrix = GMatrix4x4::Identity();
        objData.WorldViewProjection = camera->GetViewProjectionMatrix();
        objData.NormalMatrix = GMatrix4x4::Identity();
        objData.PrevWorldMatrix = objData.WorldMatrix;

        void* mapped = m_ObjectConstantBuffer->Map();
        if (mapped)
        {
            std::memcpy(mapped, &objData, sizeof(ObjectData));
            m_ObjectConstantBuffer->Unmap();
        }

        device->SetConstantBuffer(m_ObjectConstantBuffer, 1, EShaderType::Vertex);
        device->SetConstantBuffer(m_ObjectConstantBuffer, 1, EShaderType::Pixel);

        Mesh* mesh = comp->MeshData;
        device->SetVertexBuffer(mesh->GetVertexBuffer(), 0);
        device->SetIndexBuffer(mesh->GetIndexBuffer());

        const TArray<MeshSection>& sections = mesh->GetSections();
        u32 sectionIdx = comp->SectionIndex;
        if (static_cast<usize>(sectionIdx) < sections.Size())
        {
            const MeshSection& section = sections[sectionIdx];
            device->DrawIndexed(section.IndexCount, section.IndexOffset, section.VertexOffset);
        }
    }
}

void Renderer::RenderDeferredLightingPass(Device* device)
{
    Texture* bb = device->GetBackBuffer();
    device->SetRenderTargets(&bb, 1, nullptr);

    ViewportDesc vp2 = {};
    vp2.Width = static_cast<f32>(m_Width);
    vp2.Height = static_cast<f32>(m_Height);
    vp2.MaxDepth = 1.0f;
    device->SetViewport(vp2);

    device->SetPipeline(m_DeferredLightingPipeline);

    device->SetShaderResource(m_GBufferAlbedo, 0, EShaderType::Pixel);
    device->SetShaderResource(m_GBufferNormal, 1, EShaderType::Pixel);
    device->SetShaderResource(m_GBufferRMA, 2, EShaderType::Pixel);
    device->SetShaderResource(m_GBufferEmissive, 3, EShaderType::Pixel);
    device->SetShaderResource(m_DepthStencil, 4, EShaderType::Pixel);

    device->Draw(3, 0);
}

void Renderer::RenderSkyboxPass(Device* device, Scene* scene)
{
    Camera* camera = scene->GetActiveCamera();
    if (!camera) return;

    ObjectData objData;
    objData.WorldViewProjection = camera->GetViewProjectionMatrix();
    objData.ObjectID = 0;
    objData.bSelected = 0;
    objData.ObjectColor = GVec4(1.0f, 1.0f, 1.0f, 1.0f);

    void* mapped = m_ObjectConstantBuffer->Map();
    if (mapped)
    {
        std::memcpy(mapped, &objData, sizeof(ObjectData));
        m_ObjectConstantBuffer->Unmap();
    }

    device->SetConstantBuffer(m_ObjectConstantBuffer, 1, EShaderType::Vertex);
    device->SetPipeline(m_SkyboxPipeline);

    device->Draw(3, 0);
}

void Renderer::RenderScene(Device* device, Scene* scene, f32 deltaTime)
{
    if (!device || !scene || !m_Initialized) return;

    Camera* camera = scene->GetActiveCamera();
    if (!camera) return;

    UpdateFrameConstants(device, camera, deltaTime);
    UpdateLightConstants(device, scene);

    RenderGBufferPass(device, scene);

    RenderDeferredLightingPass(device);

    RenderSkyboxPass(device, scene);
}

void Renderer::RenderToBackbuffer(Device* device)
{
    Texture* backBuffer = device->GetBackBuffer();
    if (!backBuffer) return;

    device->SetRenderTargets(&backBuffer, 1, nullptr);
}

} // namespace grom
