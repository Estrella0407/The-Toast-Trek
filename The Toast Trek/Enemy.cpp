#include "Enemy.h"

Enemy::Enemy(IDirect3DDevice9* d3dDevice, const char* spritePath, float startX, float startY, int health,
	int texWidth, int texHeight, int cols, int rows, int maxFrames) {
	this->health = health;
	this->maxHealth = health;

	sprite = new Sprite(d3dDevice, spritePath, texWidth, texHeight, cols, rows, maxFrames, startX, startY);
	if (sprite != nullptr) sprite->CropToFrame(0);
}

Enemy::~Enemy() {
	delete sprite;
}

void Enemy::Render(LPD3DXSPRITE sharedBrush) {
	if (sprite != nullptr)
		sprite->Draw(sharedBrush);
}

void Enemy::TakeDamage(int damage) {
	health -= damage;
	if (health < 0)
		health = 0;
}

int Enemy::GetHealth() const {
	return health;
}

int Enemy::GetMaxHealth() const {
	return maxHealth;
}

Sprite* Enemy::GetSprite() const {
	return sprite;
}

bool Enemy::isAlive() const{
	return health > 0;
}

Enemy* CreateBossEnemy(IDirect3DDevice9* d3dDevice, BossId bossId, float startX, float startY) {
	// Both boss images are full-body art on a large, mostly-transparent
	// square-ish canvas (well over 1000px). Request the texture pre-scaled
	// straight to its on-screen size (matching each file's real aspect
	// ratio) instead of loading full-res and calling Sprite::SetScale() -
	// SetScale() shrinks around the sprite's own center, so on a canvas
	// this big it visibly drags the sprite away from (startX, startY)
	// (that's what was sending Goblin toward the bottom-right corner).
	switch (bossId) {
	case BossId::Goblin:
		return new Enemy(d3dDevice, "Assets/characters/goblin.png", startX, startY, 80,
			100, 87, 1, 1, 1);
	case BossId::Maki:
		return new Enemy(d3dDevice, "Assets/characters/makima.png", startX, startY, 120,
			120, 120, 1, 1, 1);
	case BossId::SkullBones:
	default:
		return new Enemy(d3dDevice, "Assets/characters/skullBones.png", startX, startY, 50,
			100, 100, 1, 1, 1);
	}
}