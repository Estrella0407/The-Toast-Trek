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
    filter = NO_FILTER;

    // Load texture from file
    D3DXCreateTextureFromFileEx(
        d3dDevice,
        filePath,
        texWidth,   // EXACT source dimensions, not D3DX_DEFAULT. D3DX_DEFAULT can
        texHeight,  // silently round a non-power-of-two image (e.g. Pochi's 250x60)
        // up to the next power-of-two texture size. If that happens,
        // every CropToFrame() rect - computed from the file's REAL
        // dimensions - lands on the wrong spot inside the actual
        // (resized) texture, showing the wrong/blended frames.
        1,
        0,
        D3DFMT_UNKNOWN,
        D3DPOOL_MANAGED,
        D3DX_DEFAULT,
        D3DX_DEFAULT,
        0,                              // No color key - use the PNG's real alpha channel.
        // A colorkey punches out every pixel that EXACTLY matches
        // the given RGB, anywhere in the image. For sprites/tiles
        // whose art reuses standard shading colors (very common in
        // pixel art), a colorkey can accidentally erase legitimate
        // opaque pixels scattered throughout the art.
        NULL,
        NULL,
        &texture
    );

    spriteWidth = textureWidth / textureCols;
    spriteHeight = textureHeight / textureRows;
    position = D3DXVECTOR2(startX, startY);
    velocity = D3DXVECTOR2(0.0f, 0.0f);
    acceleration = D3DXVECTOR2(0.0f, 0.0f);
    isJumping = false;
    boundaryX = 1280.0f - spriteWidth;
    boundaryY = 720.0f - spriteHeight;

    // Matrix
    // Centerpoint for scaling
    // Scaling rotation factor
    // Scaling amount
    // Centerpont for rotation
    // Rotation amount
    // Translation

    scalingCenter = D3DXVECTOR2(spriteWidth * 0.5f, spriteHeight * 0.5f);
    scalingRotation = 0.0f;
    scaling = D3DXVECTOR2(1.0f, 1.0f);
    rotationCenter = D3DXVECTOR2(spriteWidth * 0.5f, spriteHeight * 0.5f);
    rotation = 0.0f; // was 1.0f - that's ~57 degrees, an unintended default rotation on every sprite
    rotationSpeed = 0.0f;

    enginePower = 1;
    engineForce = D3DXVECTOR2(0.0f, 0.0f);
    mass = 1.0f;

    UpdateRect();
}

void Sprite::DrawBar(LPD3DXSPRITE sharedBrush, float percentage) {
	if (texture == NULL || sharedBrush == NULL)
		return;
    if (percentage < 0.0f)
        percentage = 0.0f;
    if (percentage > 1.0f)
        percentage = 1.0f;
    int drawWidth = (int)(spriteWidth * percentage);

    if (drawWidth <= 0)
        return;
    RECT sourceRect;
    sourceRect.left = 0;
    sourceRect.top = 0;
    sourceRect.right = drawWidth;
    sourceRect.bottom = spriteHeight;

	// Bars use absolute screen coordinates. Do not inherit the transform of
	// the previously rendered character or projectile sprite.
	D3DXMATRIX identity;
	D3DXMatrixIdentity(&identity);
	sharedBrush->SetTransform(&identity);
    D3DXVECTOR3 position(this->position.x, this->position.y, 0.0f);
	sharedBrush->Draw(texture, &sourceRect, NULL, &position,
		D3DCOLOR_ARGB(255, 255, 255, 255));
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

void Sprite::CustomCrop(int left, int top, int width, int height) {
    rect.left = left;
    rect.top = top;
    rect.right = left + width;
    rect.bottom = top + height;
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

// Maps animationState to the correct row index
int Sprite::GetRowForState(animationState state) const {
    if (textureRows == 2) {
        // Row 0 = LEFT, Row 1 = RIGHT
        return (state == LEFT) ? 0 : 1;
    }
    return static_cast<int>(state);
}

void Sprite::SetIdlePose(animationState dir) {
    currentState = dir;
    int row = GetRowForState(dir);
    currentFrame = row * textureCols;
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

void Sprite::Jump() {
    if (!isJumping) {
        velocity.y = -15.0f;
        acceleration.y = 0.8f;
        isJumping = true;

        if (currentState == LEFT) {
            velocity.x = -3.0f; // Towards left
        }
        else if (currentState == RIGHT) {
            velocity.x = 3.0f; // Towards right
        }
    }
}

void Sprite::SetX(float windowWidth) {
    // Keeps the sprite on the right boundary of the screen
    boundaryX = windowWidth - spriteWidth;
}

void Sprite::SetY(float windowHeight) {
    // Keeps the sprite on the bottom boundary of the screen
    boundaryY = windowHeight - spriteHeight;
}

void Sprite::UpdatePhysics() {
    //if (isJumping) {
    //    position.x += velocity.x;
    //    position.y += velocity.y;

    //    velocity.y += acceleration.y;

    //    if (velocity.y > 0.0f) {
    //        acceleration.y += 0.05f;
    //    }

    //    if (position.y >= boundaryY) {
    //        position.y = boundaryY;
    //        velocity = D3DXVECTOR2(0.0f, 0.0f);
    //        isJumping = false;
    //    }
    //}

    if (position.x < 0 || position.x > boundaryX) {
        velocity.x *= -1;
    }

    if (position.y < 0 || position.y > boundaryY) {
        velocity.y *= -1;
    }
}

void Sprite::SetRotation(float newRotation) {
    rotation = newRotation;
}

void Sprite::Rotate(float amount) {
    rotation += amount;
}

float Sprite::GetRotation() const {
    return rotation;
}

void Sprite::SetRotationSpeed(float newRotationSpeed) {
    rotationSpeed = newRotationSpeed;
}

float Sprite::GetRotationSpeed() const {
    return rotationSpeed;
}

void Sprite::SetScale(float uniformScale) {
    SetScale(uniformScale, uniformScale);
}

void Sprite::SetScale(float scaleX, float scaleY) {
    // A zero/negative scale would make rendering and collision bounds invalid.
    scaling.x = scaleX > 0.0f ? scaleX : 1.0f;
    scaling.y = scaleY > 0.0f ? scaleY : 1.0f;
}

D3DXVECTOR2 Sprite::GetScale() const {
    return scaling;
}

void Sprite::SetEnginePower(float power) {
    enginePower = power;
}

void Sprite::ApplyEngine() {
    engineForce.x = enginePower * cosf(rotation);
    engineForce.y = enginePower * sinf(rotation);
}

void Sprite::UpdateEngine() {
    // F = ma -> a = F / m
    acceleration = engineForce / mass;
    velocity += acceleration;
    position += velocity;
}

// Collision detection and response


void Sprite::SetColorFilter(colorfilter newFilter) {
    filter = newFilter;
}

void Sprite::Draw(LPD3DXSPRITE sharedBrush, D3DCOLOR colorTint) {
    if (texture != NULL && sharedBrush != NULL) {
        // Calculate dynamic transformation matrix internal to Sprite class
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
