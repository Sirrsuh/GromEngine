#include <Editor/Editor.h>
#include <Scene/Camera.h>
#include <Scene/Light.h>
#include <Scene/Mesh.h>
#include <Scene/Model.h>

namespace grom
{

namespace {
    u32 s_PendingResizeW = 0;
    u32 s_PendingResizeH = 0;
    bool s_HasResize = false;
}

Editor::Editor()
    : m_Window(nullptr)
    , m_Device(nullptr)
    , m_Canvas(nullptr)
    , m_Renderer(nullptr)
    , m_Scene(nullptr)
    , m_Running(false)
    , m_LastFrameTime(0.0)
    , m_DeltaTime(0.0f)
    , m_PrevMouseDown(false)
{
}

Editor::~Editor()
{
}

bool Editor::Initialize()
{
    JobSystem::Initialize();

    WindowDesc winDesc;
    winDesc.Title = "GromEditor";
    winDesc.Width = 1920;
    winDesc.Height = 1080;
    m_Window = CreatePlatformWindow(winDesc);

    DeviceDesc deviceDesc;
    deviceDesc.API = ERenderAPI::D3D11;
    deviceDesc.WindowHandle = m_Window->GetNativeHandle();
    deviceDesc.Width = 1920;
    deviceDesc.Height = 1080;
    deviceDesc.VSync = true;
    m_Device = Device::Create(deviceDesc);
    if (m_Device)
        m_Device->AddRef();

    m_Scene = new Scene();
    Camera* cam = new Camera("EditorCamera");
    cam->SetPosition(GVec3(0.0f, 2.0f, -5.0f));
    cam->SetAspectRatio(1920.0f / 1080.0f);
    m_Scene->AddCamera(cam);
    m_Scene->SetActiveCamera(cam);

    Light* sun = new Light("Sun");
    sun->SetLightType(ELightType::Directional);
    sun->SetIntensity(2.0f);
    m_Scene->AddLight(sun);

    m_Renderer = new Renderer();
    m_Renderer->Initialize(m_Device, 1920, 1080);

    m_Canvas = new UICanvas();
    m_Canvas->Initialize(m_Device, 1920, 1080);

    m_Window->SetResizeCallback([](u32 w, u32 h) {
        s_PendingResizeW = w;
        s_PendingResizeH = h;
        s_HasResize = true;
    });

    m_Running = true;
    m_LastFrameTime = GetTimeSeconds();
    return true;
}

void Editor::Run()
{
    while (m_Running)
    {
        f64 now = GetTimeSeconds();
        f32 dt = (f32)(now - m_LastFrameTime);
        m_LastFrameTime = now;

        m_Window->ProcessMessages();

        if (m_Window->IsClosed())
            m_Running = false;

        if (s_HasResize)
        {
            HandleResize(s_PendingResizeW, s_PendingResizeH);
            s_HasResize = false;
        }

        m_DeltaTime = dt;
        ProcessInput();
        Update(dt);
        Render();
    }
}

void Editor::ProcessInput()
{
    HWND hWnd = static_cast<HWND>(m_Window->GetNativeHandle());
    PollInput(hWnd, m_InputState);

    if (m_InputState.Keys[(u32)EKeyCode::Escape])
        m_Running = false;

    bool mouseDown = m_InputState.MouseButtons[(u32)EMouseButton::Left] && !m_PrevMouseDown;
    bool mouseUp = !m_InputState.MouseButtons[(u32)EMouseButton::Left] && m_PrevMouseDown;
    m_Canvas->DispatchEvent(
        GVec2{(f32)m_InputState.MouseX, (f32)m_InputState.MouseY},
        mouseDown,
        mouseUp
    );
    m_PrevMouseDown = m_InputState.MouseButtons[(u32)EMouseButton::Left];
}

void Editor::Update(f32 deltaTime)
{
    if (m_Scene)
        m_Scene->Update(deltaTime);
    m_Canvas->Update(deltaTime);
}

void Editor::Render()
{
    m_Device->BeginFrame();

    if (m_Scene)
    {
        m_Renderer->RenderScene(m_Device, m_Scene, m_DeltaTime);
    }
    else
    {
        f32 clearColor[4] = { 0.15f, 0.15f, 0.15f, 1.0f };
        m_Device->ClearRenderTarget(nullptr, clearColor);
        m_Device->ClearDepthStencil(nullptr, 1.0f, 0);
    }

    m_Renderer->RenderToBackbuffer(m_Device);

    m_Canvas->Render(m_Device);

    m_Device->Present();
}

void Editor::HandleResize(u32 w, u32 h)
{
    if (m_Device)
        m_Device->Resize(w, h);
    if (m_Renderer)
        m_Renderer->Resize(w, h);
    if (m_Canvas)
        m_Canvas->Resize(w, h);
}

void Editor::Shutdown()
{
    if (m_Canvas)
    {
        delete m_Canvas;
        m_Canvas = nullptr;
    }

    if (m_Device)
    {
        m_Device->Release();
        m_Device = nullptr;
    }

    if (m_Window)
    {
        delete m_Window;
        m_Window = nullptr;
    }

    JobSystem::Shutdown();
}

}
