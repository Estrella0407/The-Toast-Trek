#include "Heart.h"
#include "Sprite.h"
#include "Keys.h"

Heart::Heart(IDirect3DDevice9* d3dDevice)
	: GameObject(new Sprite(d3dDevice, "Assets/characters/pochiHeart.png", 64, 64, 1, 1, 1, 0.0f, 0.0f)) {
	moveSpeed = 3.0f;
	health = 20;
	maxHealth = 20;
	shield = 10;
	maxShield = 10;
}

Heart::~Heart() {
	// GameObject deletes the sprite
}

void Heart::Update(BYTE* keys) {
	D3DXVECTOR2 position = sprite->GetPosition();

	// Arrow keys and WASD both steer the heart
	if (KeyHeldAsync('A') || KeyHeldAsync(VK_LEFT))  position.x -= moveSpeed;
	if (KeyHeldAsync('D') || KeyHeldAsync(VK_RIGHT)) position.x += moveSpeed;
	if (KeyHeldAsync('W') || KeyHeldAsync(VK_UP))    position.y -= moveSpeed;
	if (KeyHeldAsync('S') || KeyHeldAsync(VK_DOWN))  position.y += moveSpeed;

	sprite->SetPosition(position.x, position.y);
}

// Heart collision
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
	if (shield > 0) {
		const int absorbed = damage < shield ? damage : shield;
		shield -= absorbed;
		damage -= absorbed;
	}
	if (damage <= 0) return;
	health -= damage;
	if (health < 0)
		health = 0;
}

void Heart::Heal(int amount) {
	health += amount;
	if (health > maxHealth)
		health = maxHealth;
}

int Heart::GetHealth() const {
	return health;
}

int Heart::GetMaxHealth() const {
	return maxHealth;
}

int Heart::GetShield() const {
	return shield;
}

int Heart::GetMaxShield() const {
	return maxShield;
}
