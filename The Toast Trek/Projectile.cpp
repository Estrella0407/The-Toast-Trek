#include "Projectile.h"
#include "Sprite.h"

Projectile::Projectile(IDirect3DDevice9* d3dDevice, float startX, float startY, float velocityX, float velocityY) {
	velocity = D3DXVECTOR2(velocityX, velocityY);
	width = 32.0f;
	height = 32.0f;

	active = true;

	sprite = new Sprite(d3dDevice, "Assets/Attack/fire.png", 32, 32, 1, 1, 1, startX, startY);
}

Projectile::~Projectile() {
	delete sprite;
	sprite = nullptr;
}

void Projectile::Update() {
	if (!active)
		return;

	D3DXVECTOR2 position = sprite->GetPosition();
	position += velocity;
	sprite->SetPosition(position.x, position.y);
}

void Projectile::Render(LPD3DXSPRITE sharedBrush) {
	if (!active)
		return;

	sprite->Draw(sharedBrush);
}

D3DXVECTOR2 Projectile::GetPosition() const {
	return sprite->GetPosition();
}

Sprite* Projectile::GetSprite() const {
	return sprite;
}

bool Projectile::IsActive() const {
	return active;
}

void Projectile::Deactivate() {
	active = false;
}