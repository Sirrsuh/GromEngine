#pragma once
#include <Core/Types.h>
#include <Core/Math.h>
#include <Platform/Window.h>
#include <Platform/Input.h>
#include <RHI/RHI.h>
#include <Jobs/JobSystem.h>

namespace grom {

class Game {
public:
    Game();
    ~Game();

    bool Initialize();
    void Run();
    void Shutdown();

private:
    void ProcessInput();
    void Update(f32 deltaTime);
    void Render();
    void HandleResize(u32 w, u32 h);

    Window* m_Window;
    Device* m_Device;
    InputState m_InputState;
    bool m_Running;
    f64 m_LastFrameTime;
};

}
