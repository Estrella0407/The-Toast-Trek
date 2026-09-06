#include "Sprite.h"

Sprite::Sprite(
    IDirect3DDevice9* d3dDevice,
    const char* filePath,
    int texWidth,
    int texHeight,
    int cols,
    int rows,
    int maxFrames,
    float startX,
    float startY
) {
    textureWidth = texWidth;
    textureHeight = texHeight;
    textureCols = cols;
    textureRows = rows;
    maxFrame = maxFrames;
    currentFrame = 0;
    texture = NULL;

    frameCounter = 0;
    frameDelay = 5;
    currentState = RIGHT;

    // Pass the EXACT source size, not D3DX_DEFAULT: D3DX_DEFAULT can round a
    // non-power-of-two image (e.g. Pochi's 250x60) up to the next power of
    // two, which throws every CropToFrame() rect off. No colour key - use
    // the PNG's own alpha channel
    D3DXCreateTextureFromFileEx(
        d3dDevice, filePath, texWidth, texHeight, 1, 0,
        D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
        0, NULL, NULL, &texture);

    spriteWidth = textureWidth / textureCols;
    spriteHeight = textureHeight / textureRows;
    position = D3DXVECTOR2(startX, startY);

    scalingCenter = D3DXVECTOR2(spriteWidth * 0.5f, spriteHeight * 0.5f);
    scalingRotation = 0.0f;
    scaling = D3DXVECTOR2(1.0f, 1.0f);
    rotationCenter = D3DXVECTOR2(spriteWidth * 0.5f, spriteHeight * 0.5f);
    rotation = 0.0f;

    UpdateRect();
}

void Sprite::UpdateRect() {
    rect.left = 0;
    rect.top = 0;
    rect.right = textureWidth;
    rect.bottom = textureHeight;
}

void Sprite::Move(float offsetX, float offsetY) {
    position.x += offsetX;
    position.y += offsetY;
}

void Sprite::SetPosition(float x, float y) {
    position.x = x;
    position.y = y;
}

D3DXVECTOR2 Sprite::GetPosition() const {
    return position;
}

void Sprite::CropToFrame(int frame) {
    int currentRow = frame / textureCols;
    int currentCol = frame % textureCols;

    rect.left = currentCol * spriteWidth;
    rect.top = currentRow * spriteHeight;
    rect.right = rect.left + spriteWidth;
    rect.bottom = rect.top + spriteHeight;
}

void Sprite::NextFrame() {
    currentFrame = (currentFrame + 1) % maxFrame;
    CropToFrame(currentFrame);
}

// Maps an animationState to its row in the sprite sheet
int Sprite::GetRowForState(animationState state) const {
    if (textureRows == 2) {
        // Row 0 = LEFT, Row 1 = RIGHT
        return (state == LEFT) ? 0 : 1;
    }
    return static_cast<int>(state);
}

void Sprite::SetIdlePose(animationState dir) {
    currentState = dir;
    currentFrame = GetRowForState(dir) * textureCols;
    CropToFrame(currentFrame);
}

void Sprite::SetIdlePose() {
    SetIdlePose(currentState);
}

void Sprite::AnimateWalk(animationState dir) {
    currentState = dir;
    frameCounter++;
    if (frameCounter >= frameDelay) {
        frameCounter = 0;
        int row = GetRowForState(dir);
        int startFrame = row * textureCols;
        int endFrame = startFrame + textureCols;

        currentFrame++;
        if (currentFrame < startFrame || currentFrame >= endFrame) {
            currentFrame = startFrame;
        }
        CropToFrame(currentFrame);
    }
}

void Sprite::AnimateWalk() {
    AnimateWalk(currentState);
}

float Sprite::GetRotation() const {
    return rotation;
}

void Sprite::SetScale(float uniformScale) {
    SetScale(uniformScale, uniformScale);
}

void Sprite::SetScale(float scaleX, float scaleY) {
    // A zero/negative scale would make rendering and collision bounds invalid
    scaling.x = scaleX > 0.0f ? scaleX : 1.0f;
    scaling.y = scaleY > 0.0f ? scaleY : 1.0f;
}

D3DXVECTOR2 Sprite::GetScale() const {
    return scaling;
}

void Sprite::Draw(LPD3DXSPRITE sharedBrush, D3DCOLOR colorTint) {
    if (texture != NULL && sharedBrush != NULL) {
        D3DXMatrixTransformation2D(
            &transformMatrix,
            &scalingCenter,
            scalingRotation,
            &scaling,
            &rotationCenter,
            rotation,
            &position
        );

        sharedBrush->SetTransform(&transformMatrix);
        sharedBrush->Draw(texture, &rect, NULL, NULL, colorTint);
    }
}

RECT Sprite::GetRect() const {
    return rect;
}
