#include "Projectile.h"
#include "Sprite.h"

Projectile::Projectile(IDirect3DDevice9* d3dDevice, float startX, float startY, float velocityX, float velocityY, ProjectileType type)
	: sprite(nullptr), type(type), velocity(velocityX, velocityY), width(32.0f), height(32.0f), active(true), frameCounter(0), frameDelay(10) {
	velocity = D3DXVECTOR2(velocityX, velocityY);
	width = 32.0f;
	height = 32.0f;

	active = true;

	//sprite = new Sprite(d3dDevice, "Assets/Attack/fire.png", 32, 32, 1, 1, 1, startX, startY);

	switch (type) {
	case ProjectileType::fire:
		sprite = new Sprite(d3dDevice, "Assets/Attack/fire.png", 32, 32, 1, 1, 1, startX, startY);
		sprite->CropToFrame(0);
		break;
	case ProjectileType::star:
		sprite = new Sprite(d3dDevice, "Assets/Attack/starRotate.png", 128, 32, 4, 1, 4, startX, startY);
		sprite->CropToFrame(0);
		sprite->SetScale(2.0f);
		break;
	}
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

	if (type == ProjectileType::star) {
		frameCounter++;
		if (frameCounter >= frameDelay) {
			frameCounter = 0;
			sprite->NextFrame();
		}
	}
}

void Projectile::Render(LPD3DXSPRITE sharedBrush) {
	if (!active)
		return;

	sprite->Draw(sharedBrush);
}

D3DXVECTOR2 Projectile::GetPosition() const {
	return sprite->GetPosition();
}

void Projectile::SetPosition(float x, float y) {
	if (sprite != nullptr) sprite->SetPosition(x, y);
}

D3DXVECTOR2 Projectile::GetVelocity() const {
	return velocity;
}

void Projectile::SetVelocity(float velocityX, float velocityY) {
	velocity.x = velocityX;
	velocity.y = velocityY;
}

ProjectileType Projectile::GetType() const {
	return type;
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
