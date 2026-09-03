#include "Enemy.h"

Enemy::Enemy(IDirect3DDevice9* d3dDevice, BossId bossId, const char* spritePath, float startX, float startY, int texWidth, 
	int texHeight, int cols, int rows, int maxFrames) : bossId(bossId), sprite(nullptr), actSprite(nullptr), health(0), 
	maxHealth(0), attackDamage(0), actAnimation(false), actFrame(0), maxFrames(0), actFrameCounter(0), actFrameDelay(90) {
	
	switch (bossId) {
	case BossId::SkullBones:
		health = 10;
		attackDamage = 1;
		break;
	case BossId::Goblin:
		health = 20;
		attackDamage = 3;
		break;
	case BossId::Maki:
		health = 30;
		attackDamage = 5;
		break;
	case BossId::MrAndrew:
		health = 1000;
		attackDamage = 30;
		break;
	}
	maxHealth = health;

	sprite = new Sprite(d3dDevice, spritePath, texWidth, texHeight, cols, rows, maxFrames, startX, startY);
	if (sprite != nullptr) sprite->CropToFrame(0);

	switch (bossId) {
	case BossId::SkullBones:
		actSprite = new Sprite(d3dDevice, "Assets/characters/skullBlush.png", 200, 100, 2, 1, 2, startX, startY);
		break;
	case BossId::Goblin:
		actSprite = new Sprite(d3dDevice, "Assets/characters/goblinBlush.png", 200, 100, 2, 1, 2, startX, startY);
		break;
	case BossId::MrAndrew:
		actSprite = new Sprite(d3dDevice, "Assets/characters/MrAndrewBlush.png", 200, 100, 2, 1, 2, startX, startY);
		break;
	}

	switch (bossId) {
	case BossId::SkullBones:
		attackType = AttackType::FourDirection;		// FourDirection
		break;
	case BossId::Goblin:
		attackType = AttackType::StarBounce;		// StarBounce
		break;
	case BossId::Maki:
		attackType = AttackType::Gunshot;
		break;
	case BossId::MrAndrew:
		attackType = AttackType::SpecialAttack;
		break;
	}
}

Enemy::~Enemy() {
	delete sprite;
	delete actSprite;
}

void Enemy::Render(LPD3DXSPRITE sharedBrush, D3DCOLOR tint) {
	if (actAnimation && actSprite != nullptr) {
		actSprite->Draw(sharedBrush, tint);
	}
	else if (sprite != nullptr) {
		sprite->Draw(sharedBrush, tint);
	}
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

int Enemy::GetAttackDamage() const {
	return attackDamage;
}

Sprite* Enemy::GetSprite() const {
	return sprite;
}

bool Enemy::IsAlive() const{
	return health > 0;
}

void Enemy::StartActAnimation() {
	if (actSprite == nullptr) return;
	actAnimation = true;
	actFrame = 0;
	actFrameCounter = 0;
	actSprite->CropToFrame(0);
}

void Enemy::UpdateActAnimation() {
	if (!actAnimation)
		return;
	actFrameCounter++;

	if (actFrameCounter >= actFrameDelay) {
		actFrameCounter = 0;
		actFrame++;

		if (actFrame >= maxFrames) {
			// When animation finish
			actAnimation = false;
			actFrame = 0;
			return;
		}
		actSprite->CropToFrame(actFrame);
	}
}

bool Enemy::IsActAnimationFinished() const {
	return !actAnimation;
}

AttackType Enemy::GetAttackType() const {
	return attackType;
}


Enemy* CreateBossEnemy(IDirect3DDevice9* d3dDevice, BossId bossId, float startX, float startY) {
	switch (bossId) {
	case BossId::Goblin:
		return new Enemy(d3dDevice, BossId::Goblin, "Assets/characters/goblin.png", startX, startY, 
			100, 87, 1, 1, 1);
	case BossId::Maki:
		return new Enemy(d3dDevice, BossId::Maki, "Assets/characters/makima.png", startX, startY,
			100, 100, 1, 1, 1);
	case BossId::MrAndrew:
		return new Enemy(d3dDevice, BossId::MrAndrew, "Assets/characters/MrAndrew.png", startX, startY,
			100, 100, 1, 1, 1);
	case BossId::SkullBones:
	default:
		return new Enemy(d3dDevice, BossId::SkullBones, "Assets/characters/skullBones.png", startX, startY,
			100, 100, 1, 1, 1);
	}
}
