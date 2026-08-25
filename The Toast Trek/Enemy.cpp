#include "Enemy.h"

Enemy::Enemy(IDirect3DDevice9* d3dDevice, const char* spritePath, float startX, float startY, int health) {
	this->health = health;
	this->maxHealth = health;

	sprite = new Sprite(d3dDevice, spritePath, 128, 128, 1, 1, 1, startX, startY);

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