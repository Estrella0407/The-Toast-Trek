#pragma once
#include <d3d9.h>
#include <d3dx9.h>

enum animationState { DOWN = 0, LEFT = 1, RIGHT = 2, UP = 3 };
enum colorfilter { NO_FILTER, RED_FILTER, GREEN_FILTER, BLUE_FILTER };

class Sprite {
private:
    RECT rect;
    int textureWidth, textureHeight;
    int textureCols, textureRows;
    int spriteWidth, spriteHeight;
    int currentFrame, maxFrame;
    LPDIRECT3DTEXTURE9 texture;

    D3DXVECTOR2 position;
    D3DXVECTOR2 velocity;
    D3DXVECTOR2 acceleration;
    bool isJumping;
    float boundaryX, boundaryY;

    // Transformation variables
    D3DXVECTOR2 scalingCenter;
    float scalingRotation;
    D3DXVECTOR2 scaling;
    D3DXVECTOR2 rotationCenter;
    float rotation;
    D3DXMATRIX transformMatrix;
    float rotationSpeed;

    float enginePower;
    D3DXVECTOR2 engineForce;
    float mass;

    // Animation timing counters
    int frameCounter;
    int frameDelay;

    animationState currentState;
    colorfilter filter;

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

    //health or armor bar
    void DrawBar(LPD3DXSPRITE sharedBrush, float percentage);

    void UpdateRect();
    void Move(float offsetX, float offsetY);
    void SetPosition(float x, float y);
    D3DXVECTOR2 GetPosition() const; // Standardized to Vector2

    void CustomCrop(int left, int top, int width, int height);
    void CropToFrame(int frame = 0);
    void NextFrame();
    int GetRowForState(animationState state) const;

    void SetIdlePose(animationState dir);
    void SetIdlePose();

    void AnimateWalk(animationState dir);
    void AnimateWalk();
    void Jump();

    void SetX(float windowWidth);
    void SetY(float windowHeight);

    void UpdatePhysics();

    void SetRotation(float newRotation);
    void Rotate(float amount);
    float GetRotation() const;

    void SetRotationSpeed(float newRotationSpeed);
    float GetRotationSpeed() const;

    void SetScale(float uniformScale);
    void SetScale(float scaleX, float scaleY);
    D3DXVECTOR2 GetScale() const;

    void SetEnginePower(float power);
    void ApplyEngine();
    void UpdateEngine();

    void SetColorFilter(colorfilter newFilter);
    void Draw(LPD3DXSPRITE sharedBrush, D3DCOLOR colorTint = D3DCOLOR_XRGB(255, 255, 255));

    RECT GetRect() const;
};
