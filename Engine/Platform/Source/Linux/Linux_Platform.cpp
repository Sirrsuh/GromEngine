#include "Platform/Linux/Linux_Platform.h"
#include <Core/Types.h>
#include <Core/Container.h>
#include <Core/Assert.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <unistd.h>
#include <sys/time.h>

namespace grom {

LinuxWindow::~LinuxWindow()
{
    Destroy();
}

bool LinuxWindow::Create(const WindowDesc& desc)
{
    m_Display = XOpenDisplay(nullptr);
    if (!m_Display) return false;

    int screen = DefaultScreen(m_Display);
    Window root = RootWindow(m_Display, screen);

    XSetWindowAttributes attrs;
    attrs.background_pixel = BlackPixel(m_Display, screen);
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                       ButtonReleaseMask | PointerMotionMask | StructureNotifyMask;

    m_Window = XCreateWindow(m_Display, root, 0, 0, desc.Width, desc.Height, 0,
                             CopyFromParent, InputOutput, CopyFromParent,
                             CWBackPixel | CWEventMask, &attrs);

    XStoreName(m_Display, m_Window, desc.Title.c_str());
    XMapWindow(m_Display, m_Window);

    Atom wmDelete = XInternAtom(m_Display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_Display, m_Window, &wmDelete, 1);

    return true;
}

void LinuxWindow::Destroy()
{
    if (m_Window && m_Display)
    {
        XDestroyWindow(static_cast<Display*>(m_Display), static_cast<Window>(m_Window));
        m_Window = 0;
    }
    if (m_Display)
    {
        XCloseDisplay(static_cast<Display*>(m_Display));
        m_Display = nullptr;
    }
}

void LinuxWindow::Show()
{
    if (m_Display && m_Window)
        XMapWindow(static_cast<Display*>(m_Display), static_cast<Window>(m_Window));
}

void LinuxWindow::Hide()
{
    if (m_Display && m_Window)
        XUnmapWindow(static_cast<Display*>(m_Display), static_cast<Window>(m_Window));
}

void LinuxWindow::SetTitle(const GString& title)
{
    if (m_Display && m_Window)
        XStoreName(static_cast<Display*>(m_Display), static_cast<Window>(m_Window), title.c_str());
}

void LinuxWindow::SetSize(u32 width, u32 height)
{
    if (m_Display && m_Window)
        XResizeWindow(static_cast<Display*>(m_Display), static_cast<Window>(m_Window), width, height);
}

bool LinuxWindow::ShouldClose() const
{
    return m_ShouldClose;
}

void LinuxWindow::PollEvents()
{
    if (!m_Display) return;

    XEvent event;
    while (XPending(static_cast<Display*>(m_Display)))
    {
        XNextEvent(static_cast<Display*>(m_Display), &event);

        switch (event.type)
        {
        case ClientMessage:
            if (static_cast<Atom>(event.xclient.data.l[0]) == 
                XInternAtom(static_cast<Display*>(m_Display), "WM_DELETE_WINDOW", False))
                m_ShouldClose = true;
            break;
        case DestroyNotify:
            m_ShouldClose = true;
            break;
        }
    }
}

LinuxPlatform::~LinuxPlatform()
{
    Shutdown();
}

bool LinuxPlatform::Initialize()
{
    m_Display = XOpenDisplay(nullptr);
    return m_Display != nullptr;
}

void LinuxPlatform::Shutdown()
{
    for (auto* window : m_Windows)
    {
        window->Destroy();
        delete window;
    }
    m_Windows.Clear();

    if (m_Display)
    {
        XCloseDisplay(static_cast<Display*>(m_Display));
        m_Display = nullptr;
    }
}

Window* LinuxPlatform::CreateWindow(const WindowDesc& desc)
{
    LinuxWindow* window = new LinuxWindow();
    if (window->Create(desc))
    {
        m_Windows.Add(window);
        return window;
    }
    delete window;
    return nullptr;
}

void LinuxPlatform::PollEvents()
{
    for (auto* window : m_Windows)
        window->PollEvents();
}

double LinuxPlatform::GetTime() const
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

void LinuxPlatform::Sleep(u32 ms)
{
    usleep(ms * 1000);
}

LinuxPlatform* LinuxPlatform::Create()
{
    LinuxPlatform* platform = new LinuxPlatform();
    if (platform->Initialize())
        return platform;
    delete platform;
    return nullptr;
}

} // namespace grom