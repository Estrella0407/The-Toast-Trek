#include "Heart.h"
#include "Sprite.h"
#include <Windows.h>

Heart::Heart(IDirect3DDevice9* d3dDevice) {
	moveSpeed = 3.0f;
	health = 20;

	boundaryLeft = 0.0f;
	boundaryRight = 0.0f;
	boundaryTop = 0.0f;
	boundaryBottom = 0.0f;

	sprite = new Sprite(d3dDevice, "Assets/characters/pochiHeart.png", 64, 64, 1, 1, 1, 0.0f, 0.0f);

}

Heart::~Heart() {
	delete sprite;
	sprite = nullptr;
}

void Heart::Update(BYTE* keys) {
	D3DXVECTOR2 position = sprite->GetPosition();

	if (GetAsyncKeyState('A') & 0x8000)
		position.x -= moveSpeed;

	if (GetAsyncKeyState('D') & 0x8000)
		position.x += moveSpeed;

	if (GetAsyncKeyState('W') & 0x8000)
		position.y -= moveSpeed;

	if (GetAsyncKeyState('S') & 0x8000)
		position.y += moveSpeed;

	sprite->SetPosition(position.x, position.y);
}

void Heart::Render(LPD3DXSPRITE sharedBrush) {
	sprite->Draw(sharedBrush);
}

D3DXVECTOR2 Heart::GetPosition() const {
	return sprite->GetPosition();
}

Sprite* Heart::GetSprite() const {
	return sprite;
}

void Heart::SetPosition(float x, float y) {
	sprite->SetPosition(x, y);
}

//heart collision
void Heart::ClampToBoundary(float left, float top, float right, float bottom) {
	D3DXVECTOR2 position = sprite->GetPosition();

	const float textureWidth = 64.0f;
	const float textureHeight = 64.0f;

	const float heartWidth = 42.0f;
	const float heartHeight = 31.0f;

	const float offsetX = (textureWidth - heartWidth) / 2.0f;
	const float offsetY = (textureHeight - heartHeight) / 2.0f;

	float visibleLeft = position.x + offsetX;
	float visibleTop = position.y + offsetY;

	float visibleRight = visibleLeft + heartWidth;
	float visibleBottom = visibleTop + heartHeight;

	if (visibleLeft < left)
		position.x += left - visibleLeft;
	if (visibleTop < top)
		position.y += top - visibleTop;
	if (visibleRight > right)
		position.x -= visibleRight - right;
	if (visibleBottom > bottom)
		position.y -= visibleBottom - bottom;

	sprite->SetPosition(position.x, position.y);
}

void Heart::TakeDamage(int damage) {
	health -= damage;
	if (health < 0)
		health = 0;
}

int Heart::GetHealth() const {
	return health;
}