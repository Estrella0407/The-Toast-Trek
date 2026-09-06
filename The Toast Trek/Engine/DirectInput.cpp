#include "DirectInput.h"

DirectInput::DirectInput()
    : dInput(NULL), keyboardDevice(NULL), mouseDevice(NULL)
{
    ZeroMemory(keys, sizeof(keys));
    ZeroMemory(&mouseState, sizeof(mouseState));
}

bool DirectInput::Create(HWND hWnd)
{
    HRESULT hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&dInput, NULL);
    if (FAILED(hr)) return false;

    // Keyboard
    dInput->CreateDevice(GUID_SysKeyboard, &keyboardDevice, NULL);
    keyboardDevice->SetDataFormat(&c_dfDIKeyboard);
    keyboardDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    keyboardDevice->Acquire();

    // Mouse
    dInput->CreateDevice(GUID_SysMouse, &mouseDevice, NULL);
    mouseDevice->SetDataFormat(&c_dfDIMouse);
    mouseDevice->SetCooperativeLevel(hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    mouseDevice->Acquire();

    return true;
}

void DirectInput::Poll()
{
    // Immediate keyboard data
    HRESULT hr = keyboardDevice->GetDeviceState(256, keys);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        keyboardDevice->Acquire();
        hr = keyboardDevice->GetDeviceState(256, keys);
    }
    if (FAILED(hr)) ZeroMemory(keys, sizeof(keys));

    // Immediate mouse data (relative motion deltas)
    hr = mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        mouseDevice->Acquire();
        hr = mouseDevice->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState);
    }
    if (FAILED(hr)) ZeroMemory(&mouseState, sizeof(mouseState));
}

void DirectInput::Cleanup()
{
    if (keyboardDevice) {
        keyboardDevice->Unacquire();
        keyboardDevice->Release();
        keyboardDevice = NULL;
    }
    if (mouseDevice) {
        mouseDevice->Unacquire();
        mouseDevice->Release();
        mouseDevice = NULL;
    }
    if (dInput) {
        dInput->Release();
        dInput = NULL;
    }
}
