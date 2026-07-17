#include "Platform/Backend/Linux/Linux_Window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

namespace grom {

LinuxWindow::LinuxWindow()
    : m_Display(nullptr)
    , m_Window(0)
    , m_WMDeleteMessage(0)
    , m_ShouldClose(false)
{
}

LinuxWindow::~LinuxWindow()
{
    Destroy();
}

bool LinuxWindow::Create(const WindowDesc& desc)
{
    m_Display = XOpenDisplay(nullptr);
    if (!m_Display)
        return false;

    int screen = DefaultScreen(m_Display);
    Window root = RootWindow(m_Display, screen);

    XSetWindowAttributes attrs;
    attrs.background_pixel = BlackPixel(m_Display, screen);
    attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask |
                       ButtonReleaseMask | PointerMotionMask | StructureNotifyMask | FocusChangeMask;

    unsigned long mask = CWBackPixel | CWEventMask;

    m_Window = XCreateWindow(m_Display, root,
                             desc.X, desc.Y, desc.Width, desc.Height,
                             0, DefaultDepth(m_Display, screen),
                             InputOutput, CopyFromParent,
                             mask, &attrs);

    if (!m_Window)
        return false;

    XStoreName(m_Display, m_Window, desc.Title.c_str());

    m_WMDeleteMessage = XInternAtom(m_Display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_Display, m_Window, &m_WMDeleteMessage, 1);

    XMapWindow(m_Display, m_Window);
    XFlush(m_Display);

    return true;
}

void LinuxWindow::Destroy()
{
    if (m_Window && m_Display)
    {
        XDestroyWindow(m_Display, m_Window);
        m_Window = 0;
    }
    if (m_Display)
    {
        XCloseDisplay(m_Display);
        m_Display = nullptr;
    }
}

void LinuxWindow::SetTitle(const GString& title)
{
    if (m_Display && m_Window)
        XStoreName(m_Display, m_Window, title.c_str());
}

void LinuxWindow::SetSize(u32 width, u32 height)
{
    if (m_Display && m_Window)
    {
        XResizeWindow(m_Display, m_Window, width, height);
        XFlush(m_Display);
    }
}

void* LinuxWindow::GetNativeHandle() const
{
    return reinterpret_cast<void*>(m_Window);
}

bool LinuxWindow::ShouldClose() const
{
    return m_ShouldClose;
}

void LinuxWindow::PollEvents()
{
    if (!m_Display)
        return;

    XEvent event;
    while (XPending(m_Display))
    {
        XNextEvent(m_Display, &event);

        switch (event.type)
        {
        case ClientMessage:
            if (static_cast<Atom>(event.xclient.data.l[0]) == m_WMDeleteMessage)
                m_ShouldClose = true;
            break;
        case DestroyNotify:
            m_ShouldClose = true;
            break;
        }
    }
}

} // namespace grom