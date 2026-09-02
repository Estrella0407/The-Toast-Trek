#include "Projectile.h"
#include "Sprite.h"
#include "Physics.h"

Projectile::Projectile(IDirect3DDevice9* d3dDevice, float startX, float startY, float velocityX, float velocityY, ProjectileType type)
	: sprite(nullptr), type(type), velocity(velocityX, velocityY), width(32.0f), height(32.0f),
	active(true), damageApplied(false), frameCounter(0), frameDelay(10) {
	velocity = D3DXVECTOR2(velocityX, velocityY);
	width = 32.0f;
	height = 32.0f;

	active = true;

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
	case ProjectileType::aim:
		sprite = new Sprite(d3dDevice, "Assets/Attack/aim.png", 32, 32, 1, 1, 1, startX, startY);
		sprite->CropToFrame(0);
		sprite->SetScale(2.0f);
		break;
	case ProjectileType::bullet:
		sprite = new Sprite(d3dDevice, "Assets/Attack/bullet.png", 32, 32, 1, 1, 1, startX, startY);
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

AABB Projectile::GetCollisionBounds() const {
	AABB box = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (sprite == nullptr) return box;

	const D3DXVECTOR2 position = sprite->GetPosition();
	switch (type) {
	case ProjectileType::fire:
		// Visible alpha pixels: x 9..22, y 7..24 on a 32x32 image
		box = { position.x + 9.0f, position.y + 7.0f,
			position.x + 23.0f, position.y + 25.0f };
		break;
	case ProjectileType::star:
		// Every animation frame uses alpha pixels around x 8..24 and
		// Y 7..23. The sprite is scaled 2x around its 16,16 center
		box = { position.x, position.y - 2.0f,
			position.x + 34.0f, position.y + 32.0f };
		break;
	case ProjectileType::bullet:
		// Visible alpha pixels: x 0..30, y 3..28, also scaled 2x
		box = { position.x - 16.0f, position.y - 10.0f,
			position.x + 46.0f, position.y + 42.0f };
		break;
	case ProjectileType::aim:
		// Aim markers are non-damaging, but return their visible bounds
		box = { position.x - 16.0f, position.y - 14.0f,
			position.x + 48.0f, position.y + 46.0f };
		break;
	}
	return box;
}

bool Projectile::IsActive() const {
	return active;
}

void Projectile::Deactivate() {
	active = false;
}

bool Projectile::HasAppliedDamage() const {
	return damageApplied;
}

void Projectile::MarkDamageApplied() {
	damageApplied = true;
}
