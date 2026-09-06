#pragma once
#include <d3d9.h>
#include <d3dx9.h>

enum animationState { DOWN = 0, LEFT = 1, RIGHT = 2, UP = 3 };

class Sprite {
private:
    RECT rect;
    int textureWidth, textureHeight;
    int textureCols, textureRows;
    int spriteWidth, spriteHeight;
    int currentFrame, maxFrame;
    LPDIRECT3DTEXTURE9 texture;

    D3DXVECTOR2 position;

    // Transformation used by Draw()
    D3DXVECTOR2 scalingCenter;
    float scalingRotation;
    D3DXVECTOR2 scaling;
    D3DXVECTOR2 rotationCenter;
    float rotation;
    D3DXMATRIX transformMatrix;

    // Walk-animation timing
    int frameCounter;
    int frameDelay;

    animationState currentState;

public:
    Sprite(
        IDirect3DDevice9* d3dDevice,
        const char* filePath,
        int texWidth,
        int texHeight,
        int cols,
        int rows,
        int maxFrames,
        float startX,
        float startY
    );

    void UpdateRect();
    void Move(float offsetX, float offsetY);
    void SetPosition(float x, float y);
    D3DXVECTOR2 GetPosition() const;

    void CropToFrame(int frame = 0);
    void NextFrame();
    int GetRowForState(animationState state) const;

    void SetIdlePose(animationState dir);
    void SetIdlePose();

    void AnimateWalk(animationState dir);
    void AnimateWalk();

    float GetRotation() const;

    void SetScale(float uniformScale);
    void SetScale(float scaleX, float scaleY);
    D3DXVECTOR2 GetScale() const;

    void Draw(LPD3DXSPRITE sharedBrush, D3DCOLOR colorTint = D3DCOLOR_XRGB(255, 255, 255));

    RECT GetRect() const;
};
