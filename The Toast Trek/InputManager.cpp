#include "InputManager.h"
#include "Keys.h"

InputManager::InputManager()
    : mouseX(0.0f), mouseY(0.0f), mouseLeftDown(false)
{
}

bool InputManager::Create(HWND hWnd)
{
    return directInput.Create(hWnd);
}

void InputManager::Update(HWND hWnd, int backBufferWidth, int backBufferHeight)
{
    directInput.Poll();

    // The DirectInput mouse reports relative motion, not an absolute
    // position, so read the cursor directly and map it into back-buffer space.
    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(hWnd, &cursor);

    RECT clientRect = {};
    GetClientRect(hWnd, &clientRect);
    const float clientWidth = (float)(clientRect.right - clientRect.left);
    const float clientHeight = (float)(clientRect.bottom - clientRect.top);

    mouseX = clientWidth > 0.0f ? cursor.x * backBufferWidth / clientWidth : 0.0f;
    mouseY = clientHeight > 0.0f ? cursor.y * backBufferHeight / clientHeight : 0.0f;
    mouseLeftDown = KeyHeldAsync(VK_LBUTTON);
}
