#include "Window.h"

// Window Procedure, for event handling
LRESULT CALLBACK Window::WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        //	The message is posted when we destroy the window
    case WM_DESTROY:
        PostQuitMessage(0);
        break;

        // ESC is a normal gameplay key (the tab menu uses it to close);
        // quitting is via the window's close button
    case WM_KEYDOWN:
        break;

        //	Default handling for other messages
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

Window::Window() : hWnd(NULL)
{
    ZeroMemory(&wndClass, sizeof(wndClass));
    ZeroMemory(&msg, sizeof(msg));
}

bool Window::Create(const char* title, int width, int height)
{
    // Step 1 - define and register the window class
    ZeroMemory(&wndClass, sizeof(wndClass));
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    wndClass.hInstance = GetModuleHandle(NULL);
    wndClass.lpfnWndProc = WindowProcedure;
    wndClass.lpszClassName = "My Window";
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&wndClass);

    // Step 2 - create the window
    hWnd = CreateWindowEx(0, wndClass.lpszClassName, title, WS_OVERLAPPEDWINDOW,
        0, 100, width, height, NULL, NULL, wndClass.hInstance, NULL);
    if (hWnd == NULL) return false;

    ShowWindow(hWnd, 1);
    ZeroMemory(&msg, sizeof(msg));
    return true;
}

bool Window::IsRunning()
{
    // Step 3 - handle window messages
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return false;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return true;
}

void Window::Destroy()
{
    if (hWnd != NULL)
    {
        DestroyWindow(hWnd);
        hWnd = NULL;
    }
    UnregisterClass(wndClass.lpszClassName, GetModuleHandle(NULL));
    ZeroMemory(&wndClass, sizeof(wndClass));
}

void Window::GetClientSize(int& width, int& height) const
{
    RECT clientRect = {};
    if (hWnd != NULL) GetClientRect(hWnd, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;
}
