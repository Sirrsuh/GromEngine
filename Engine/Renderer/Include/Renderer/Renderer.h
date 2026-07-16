#pragma once
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Math.h>
#include <RHI/RHI.h>
#include <Scene/Scene.h>
#include <Scene/Camera.h>
#include <Scene/Light.h>
#include <Scene/Model.h>
#include <Materials/ShaderPermutation.h>

namespace grom {

struct FrameData
{
    GMatrix4x4 ViewMatrix;
    GMatrix4x4 ProjectionMatrix;
    GMatrix4x4 ViewProjectionMatrix;
    GMatrix4x4 InvViewMatrix;
    GMatrix4x4 InvProjectionMatrix;
    GMatrix4x4 InvViewProjectionMatrix;
    GVec4 CameraPosition;
    GVec4 CameraDirection;
    GVec2 ViewportSize;
    GVec2 ViewportPixelSize;
    f32 NearPlane;
    f32 FarPlane;
    f32 DeltaTime;
    f32 TotalTime;
    u32 FrameCount;
    u32 bOrthographic;
    f32 DummyFrame[2];
};

struct ObjectData
{
    GMatrix4x4 WorldMatrix;
    GMatrix4x4 WorldViewProjection;
    GMatrix4x4 PrevWorldMatrix;
    GMatrix4x4 NormalMatrix;
    GVec4 ObjectColor;
    u32 ObjectID;
    u32 bSelected;
    f32 DummyObject[2];
};

struct LightData
{
    GVec4 AmbientColor;
    GVec4 SunDirection;
    GVec4 SunColor;
    GVec4 IBLSpecFactor;
    u32 NumPointLights;
    u32 NumSpotLights;
    u32 NumDecals;
    f32 DummyLight;
};

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(Device* device, u32 width, u32 height);
    void Resize(u32 width, u32 height);
    void Shutdown();

    void RenderScene(Device* device, Scene* scene, f32 deltaTime);
    void RenderToBackbuffer(Device* device);

    Texture* GetGBufferAlbedo() const { return m_GBufferAlbedo; }
    Texture* GetGBufferNormal() const { return m_GBufferNormal; }
    Texture* GetGBufferRMA() const { return m_GBufferRMA; }
    Texture* GetGBufferEmissive() const { return m_GBufferEmissive; }
    Texture* GetDepthStencil() const { return m_DepthStencil; }

private:
    bool CreateGBuffer(u32 width, u32 height);
    bool CreatePipelines();

    void UpdateFrameConstants(Device* device, Camera* camera, f32 deltaTime);
    void UpdateLightConstants(Device* device, Scene* scene);
    void RenderGBufferPass(Device* device, Scene* scene);
    void RenderDeferredLightingPass(Device* device);
    void RenderSkyboxPass(Device* device, Scene* scene);

    Texture* m_GBufferAlbedo = nullptr;
    Texture* m_GBufferNormal = nullptr;
    Texture* m_GBufferRMA = nullptr;
    Texture* m_GBufferEmissive = nullptr;
    Texture* m_DepthStencil = nullptr;

    Shader* m_GBufferVS = nullptr;
    Shader* m_GBufferPS = nullptr;
    Pipeline* m_GBufferPipeline = nullptr;

    Shader* m_FullscreenVS = nullptr;
    Shader* m_DeferredLightingPS = nullptr;
    Pipeline* m_DeferredLightingPipeline = nullptr;

    Shader* m_SkyboxVS = nullptr;
    Shader* m_SkyboxPS = nullptr;
    Pipeline* m_SkyboxPipeline = nullptr;

    Buffer* m_FrameConstantBuffer = nullptr;
    Buffer* m_ObjectConstantBuffer = nullptr;
    Buffer* m_LightConstantBuffer = nullptr;

    u32 m_Width = 0;
    u32 m_Height = 0;
    u32 m_FrameCount = 0;
    f64 m_TotalTime = 0.0;
    bool m_Initialized = false;
};

} // namespace grom
