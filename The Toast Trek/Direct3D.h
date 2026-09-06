#pragma once
#include <d3d9.h>
#include <d3dx9.h>

// Owns the Direct3D 9 device, its present parameters and the shared D3DX
// sprite brush. Wraps the per-frame Clear / BeginScene / Present dance.
class Direct3D {
private:
    IDirect3D9* direct3D9;
    IDirect3DDevice9* device;
    D3DPRESENT_PARAMETERS d3dPP;
    LPD3DXSPRITE spriteBrush;

public:
    Direct3D();

    // Create the device + sprite brush for the given window. Returns false on failure.
    bool CreateDevice(HWND hWnd, int width, int height);

    // Clear to colour, BeginScene and open the sprite brush (alpha-blended).
    void BeginFrame(D3DCOLOR clearColor);

    // Close the sprite brush, EndScene and present the back buffer.
    void EndFrame();

    IDirect3DDevice9* GetDevice() const { return device; }
    LPD3DXSPRITE GetSpriteBrush() const { return spriteBrush; }
    int BackBufferWidth() const { return d3dPP.BackBufferWidth; }
    int BackBufferHeight() const { return d3dPP.BackBufferHeight; }

    void Cleanup();
};
