#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Owns the Win32 window: registers the class, creates the window, pumps
// messages and tears everything down. Reusable across projects - nothing
// game-specific lives here.
class Window {
private:
    HWND hWnd;
    WNDCLASS wndClass;
    MSG msg;

    static LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
    Window();

    // Register the class and create + show the window. Returns false on failure.
    bool Create(const char* title, int width, int height);

    // Pump every pending message. Returns false once WM_QUIT has arrived.
    bool IsRunning();

    // Destroy the window and unregister the class.
    void Destroy();

    HWND GetHandle() const { return hWnd; }
    void GetClientSize(int& width, int& height) const;
};
