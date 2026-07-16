#include <Game/Game.h>
#include <Platform/Platform.h>
#include <Win32_Window.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace grom {

Game::Game()
    : m_Window(nullptr)
    , m_Device(nullptr)
    , m_Running(false)
    , m_LastFrameTime(0.0)
{
}

Game::~Game()
{
}

bool Game::Initialize()
{
    JobSystem::Initialize();

    WindowDesc winDesc;
    winDesc.Title = "GromGame";
    winDesc.Width = 1280;
    winDesc.Height = 720;
    winDesc.Fullscreen = false;
    winDesc.VSync = true;

    m_Window = new Win32Window(winDesc);
    if (!m_Window || m_Window->IsClosed())
        return false;

    DeviceDesc devDesc;
    devDesc.API = ERenderAPI::D3D11;
    devDesc.WindowHandle = m_Window->GetNativeHandle();
    devDesc.Width = m_Window->GetWidth();
    devDesc.Height = m_Window->GetHeight();
    devDesc.VSync = true;
    devDesc.DebugMode = true;

    m_Device = Device::Create(devDesc);
    if (!m_Device)
        return false;

    m_Running = true;
    m_LastFrameTime = GetTimeSeconds();

    return true;
}

void Game::Run()
{
    while (m_Running)
    {
        m_Window->ProcessMessages();

        if (m_Window->IsClosed())
        {
            m_Running = false;
            break;
        }

        u32 w = m_Window->GetWidth();
        u32 h = m_Window->GetHeight();
        if (w > 0 && h > 0)
        {
            DeviceDesc& desc = m_Device->GetDesc();
            if (desc.Width != w || desc.Height != h)
                HandleResize(w, h);
        }

        f64 now = GetTimeSeconds();
        f32 dt = static_cast<f32>(now - m_LastFrameTime);
        m_LastFrameTime = now;

        ProcessInput();
        Update(dt);
        Render();
    }
}

void Game::Shutdown()
{
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

void Game::ProcessInput()
{
    HWND hWnd = static_cast<HWND>(m_Window->GetNativeHandle());
    PollInput(hWnd, m_InputState);
}

void Game::Update(f32 /*deltaTime*/)
{
    if (m_InputState.Keys[static_cast<u32>(EKeyCode::Escape)])
        m_Running = false;
}

void Game::Render()
{
    m_Device->BeginFrame();

    Texture* backBuffer = m_Device->GetBackBuffer();
    const f32 clearColor[4] = {0.392f, 0.584f, 0.929f, 1.0f};
    m_Device->ClearRenderTarget(backBuffer, clearColor);

    m_Device->EndFrame();
    m_Device->Present();
}

void Game::HandleResize(u32 w, u32 h)
{
    if (m_Device)
        m_Device->Resize(w, h);
}

} // namespace grom
