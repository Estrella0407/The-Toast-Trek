#include "Direct3D.h"

Direct3D::Direct3D()
    : direct3D9(NULL), device(NULL), spriteBrush(NULL)
{
    ZeroMemory(&d3dPP, sizeof(d3dPP));
}

bool Direct3D::CreateDevice(HWND hWnd, int width, int height)
{
    // Instantiate the Direct3D 9 object
    direct3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!direct3D9) {
        MessageBox(NULL, "Direct3DCreate9 failed", "Error", MB_OK | MB_ICONERROR);
        return false;
    }

    // Describe how the screen presents
    ZeroMemory(&d3dPP, sizeof(d3dPP));
    d3dPP.Windowed = true;
    d3dPP.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dPP.BackBufferFormat = D3DFMT_X8R8G8B8;
    d3dPP.BackBufferCount = 1;
    d3dPP.BackBufferWidth = width;
    d3dPP.BackBufferHeight = height;
    d3dPP.hDeviceWindow = hWnd;

    HRESULT hr = direct3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dPP, &device);
    if (FAILED(hr)) {
        MessageBox(NULL, "CreateDevice failed", "Error", MB_OK | MB_ICONERROR);
        device = NULL;
        return false;
    }

    D3DXCreateSprite(device, &spriteBrush);
    return true;
}

void Direct3D::BeginFrame(D3DCOLOR clearColor)
{
    if (!device) return;

    device->Clear(0, NULL, D3DCLEAR_TARGET, clearColor, 1.0f, 0);
    device->BeginScene();

    // Alpha blend so sprites honour their own alpha channel over the background
    if (spriteBrush) spriteBrush->Begin(D3DXSPRITE_ALPHABLEND);
}

void Direct3D::EndFrame()
{
    if (!device) return;

    if (spriteBrush) spriteBrush->End();
    device->EndScene();
    device->Present(NULL, NULL, NULL, NULL);
}

void Direct3D::Cleanup()
{
    if (spriteBrush) { spriteBrush->Release(); spriteBrush = NULL; }
    if (device) { device->Release(); device = NULL; }
    if (direct3D9) { direct3D9->Release(); direct3D9 = NULL; }
}
