#include "Battlefield.h"
#include "Enemy.h"
#include "BattleUI.h"
#include "BattleStatusBars.h"
#include "Pochi.h"
#include "Heart.h"
#include "Physics.h"
#include <algorithm>
#include <string>
#include <cmath>
#include <cstdlib>

Battlefield::Battlefield(IDirect3DDevice9* d3dDevice, BattleUI* battleUI, Enemy* enemy, Pochi* pochi, Inventory* inventory) {
	this->d3dDevice = d3dDevice;
	this->battleUI = battleUI;
	this->enemy = enemy;
	this->pochi = pochi;
	this->inventory = inventory;
	statusFont = new Font(d3dDevice, 300.0f, 510.0f, 700, 35, 18, "Arial");
	playerBars = new BattleStatusBars(d3dDevice);
	displayedEnemyHealth = (float)enemy->GetHealth();
	hitStartHealth = displayedEnemyHealth;
	hitAnimationStart = 0;
	enemyHitAnimating = false;

	posX = 300.0f;
	posY = 200.0f;

	width = 700.0f;
	height = 300.0f;

	showProjectiles = false;
	projectileAttackFinished = false;
	projectileAttackStart = 0;
	projectileAttackDuration = 20000;
	spawningProjectile = 0;
	maxProjectiles = 26;

	projectileTimer = 0.0f;
	projectileSpawnInterval = 750.0f;

	heart = new Heart(d3dDevice);
	heart->SetPosition(posX + width / 2.0f - 32.0f, posY + height / 2.0f - 32.0f);

	//enemy = CreateBossEnemy(d3dDevice, bossId, 600.0f, 50.0f);
	fightHovered = false;

	mouseWasDown = false;
	fled = false;
	fightDamage = 10;
	itemHealAmount = 5;


	//enemyHealthBar = new Sprite(d3dDevice, ....);
}


void Battlefield::SpawnProjectile(IDirect3DDevice9* d3dDevice, float x, float y, float velocityX, float velocityY, ProjectileType type) {
	projectiles.push_back(new Projectile(d3dDevice, x, y, velocityX, velocityY, type));
}

void Battlefield::SpawnProjectileAtAngle(IDirect3DDevice9* d3dDevice, float x, float y, float angleDegrees, float speed) {
	
	float angleRadians = D3DXToRadian(angleDegrees);
	float velocityX = cosf(angleRadians) * speed;
	float velocityY = sinf(angleRadians) * speed;

	OutputDebugStringA(("Velocity X: " + std::to_string(velocityX) + "Velocity Y: " + std::to_string(velocityY) + "\n").c_str());
	SpawnProjectile(d3dDevice, x, y, velocityX, velocityY, ProjectileType::fire);
}

void Battlefield::FourDirectionAttack() {
	float speed = 1.0f;
	SpawnProjectileAtAngle(d3dDevice, posX + width / 2.0f, posY + 20.0f, 90.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + width / 2.0f, posY + height - 20.0f, 270.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + 20.0f, posY + height / 20.0f, 0.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + width - 2.0f, posY + height / 2.0f, 180.0f, speed);
}

void Battlefield::StarBounceAttack() {
	const int STAR_COUNT = 3;

	// The star sprite is ~64x64 on screen. Keep the whole wave clear of the
	// heart's current position - a star spawned on top of it used to hit
	// before the player could react (several at once = instant death).
	const D3DXVECTOR2 heartPos = heart->GetPosition();
	const float keepOut = 150.0f;

	for (int i = 0; i < STAR_COUNT; i++) {
		float x = posX + 10.0f;
		float y = posY + 10.0f;
		for (int tries = 0; tries < 24; ++tries) {
			x = posX + 10.0f + rand() % (int)(width - 74.0f);
			y = posY + 10.0f + rand() % (int)(height - 74.0f);
			if (fabsf(x - heartPos.x) > keepOut || fabsf(y - heartPos.y) > keepOut) break;
		}

		float velocityX = (rand() % 201 - 100) / 100.0f;
		float velocityY = (rand() % 201 - 100) / 100.0f;

		// No near-stationary stars - normalise slow ones up to a real speed
		// so the wave actually travels and bounces.
		float speed = sqrtf(velocityX * velocityX + velocityY * velocityY);
		if (speed < 0.6f) {
			if (speed < 0.001f) { velocityX = 1.0f; velocityY = 0.4f; speed = sqrtf(1.16f); }
			velocityX = velocityX / speed * 0.9f;
			velocityY = velocityY / speed * 0.9f;
		}

		SpawnProjectile(d3dDevice, x, y, velocityX, velocityY, ProjectileType::star);
	}
}

void Battlefield::StartEnemyAttack() {
	for (Projectile* projectile : projectiles) delete projectile;
	projectiles.clear();

	switch (enemy->GetAttackType()) {
	case AttackType::FourDirection:
		// Fire projectiles are spawned gradually from random sides in Update().
		break;
	case AttackType::StarBounce:
		StarBounceAttack();
		spawningProjectile = 3;
		break;
	}
	showProjectiles = true;
	projectileAttackFinished = false;
	projectileAttackStart = GetTickCount64();
	if (enemy->GetAttackType() == AttackType::FourDirection) spawningProjectile = 0;
	projectileTimer = enemy->GetAttackType() == AttackType::StarBounce ? 4000.0f : projectileSpawnInterval;
}

void Battlefield::PerformFight() {
	hitStartHealth = displayedEnemyHealth;

	if (enemy == nullptr || pochi == nullptr)
		return;
	int damage = pochi->GetAttackDamage();
	enemy->TakeDamage(damage);
	
	//Health bar sprite decreasing
	float percentage = (float)enemy->GetHealth() / (float)enemy->GetMaxHealth();
	int frame = (int)((1.0f - percentage) * 10);
	//enemyHealthBar->CropToFrame(frame);

	hitAnimationStart = GetTickCount64();
	enemyHitAnimating = true;
	OutputDebugStringA(("Enemy hit for " + std::to_string(damage) +
				", health now " + std::to_string(enemy->GetHealth()) + "\n").c_str());

}

void Battlefield::PerformAct() {
	if (enemy == nullptr || pochi == nullptr)
		return;

	const int actDamage = 2;
	hitStartHealth = displayedEnemyHealth;
	enemy->TakeDamage(actDamage);
	float percentage = (float)enemy->GetHealth() / (float)enemy->GetMaxHealth();
	
	int frame = (int)((1.0f - percentage) * 10);
	//enemyHealthBar->CropToFrame(frame);

	hitAnimationStart = GetTickCount64();
	enemyHitAnimating = true;
	OutputDebugStringA(("Enemy hit for " + std::to_string(fightDamage) +
		", health now " + std::to_string(enemy->GetHealth()) + "\n").c_str());

}

bool Battlefield::IsEnemyHitAnimationFinished() const {
	return !enemyHitAnimating;
}

bool Battlefield::PerformItem(ItemType item) {
	if (inventory == nullptr || !inventory->Consume(item)) return false;
	if (item == ItemType::HealthPotion) {
		if (pochi != nullptr) pochi->Heal(3);
		heart->Heal(3);
	}
	else if (item == ItemType::Bone) {
		if (pochi != nullptr) pochi->RecoverArmor(2);
	}
	else if (item == ItemType::Toast) {
		if (pochi != nullptr) {
			pochi->Heal(pochi->GetMaxHealth());
			pochi->RecoverArmor(pochi->GetMaxArmor());
		}
		heart->Heal(heart->GetMaxHealth());
	}
	return true;
}

void Battlefield::StartActAnimation() {
	enemy->StartActAnimation();
}

void Battlefield::UpdateActAnimation() {
	enemy->UpdateActAnimation();
}

bool Battlefield::IsActAnimationFinished() const {
	return enemy->IsActAnimationFinished();
}

void Battlefield::Flee() {
	fled = true;
}

void Battlefield::SetShowProjectiles(bool show) {
	if (show) {
		StartEnemyAttack();
	}
	else {
		showProjectiles = false;
	}
}

bool Battlefield::IsProjectileAttackFinished() const {
	return projectileAttackFinished;
}

Battlefield::~Battlefield() {
	delete statusFont;
	delete playerBars;
	delete heart;
	delete enemy;

	for (Projectile* projectile : projectiles) {
		delete projectile;
	}
	projectiles.clear();
}

void Battlefield::Init() {

}

void Battlefield::UpdateMenuButtons(GameContext& context) {
	battleUI->UpdateMenuButtons(context);
	bool clicked = context.mouseLeftDown && !mouseWasDown;
	mouseWasDown = context.mouseLeftDown;

	if (!clicked) return;

	//if (fightHovered) {
	//	enemy->TakeDamage(fightDamage);
	//	OutputDebugStringA(("Enemy hit for " + std::to_string(fightDamage) +
	//		", health now " + std::to_string(enemy->GetHealth()) + "\n").c_str());
	//}
	//else if (itemHovered) {
	//	heart->Heal(itemHealAmount);
	//}
	//else if (mercyHovered) {
	//	fled = true;
	//}
	//else if (actHovered) {
	//	// ACT has no per-boss dialogue/options defined yet - clickable, but a no-op for now.
	//}

	//fightButton->SetHovered(fightHovered);
}

int Battlefield::GetSelectButton(GameContext& context) {
	return battleUI->GetSelectButton(context);
}

void Battlefield::Update(GameContext& context) {
	if (enemyHitAnimating) {
		float progress = (GetTickCount64() - hitAnimationStart) / 1000.0f;
		if (progress > 1.0f) progress = 1.0f;
		displayedEnemyHealth = hitStartHealth + (enemy->GetHealth() - hitStartHealth) * progress;
		if (progress >= 1.0f) enemyHitAnimating = false;
	}

	heart->Update(context.keys);

	heart->ClampToBoundary(posX, posY, posX + width, posY + height);
	AABB heartBounds = Physics::GetHeartBounds(heart->GetSprite());

	if (!showProjectiles) return;

	const bool starAttack = enemy->GetAttackType() == AttackType::StarBounce;
	const unsigned long long elapsedAttackTime = GetTickCount64() - projectileAttackStart;
	const bool attackTimedOut = elapsedAttackTime >= projectileAttackDuration;
	if (!attackTimedOut && spawningProjectile < maxProjectiles && elapsedAttackTime >= projectileTimer) {
		if (starAttack) {
			StarBounceAttack();
			spawningProjectile += 3;
			projectileTimer += 4000.0f;
		}
		else {
			projectileTimer += projectileSpawnInterval;

			const float speed = 1.0f;
			const int side = rand() % 4;
			float spawnX = 0.0f;
			float spawnY = 0.0f;
			float angle = 0.0f;

			switch (side) {
			case 0: // top, moving down
				spawnX = posX + 20.0f + static_cast<float>(rand() % static_cast<int>(width - 64.0f));
				spawnY = posY + 20.0f;
				angle = 90.0f;
				break;
			case 1: // bottom, moving up
				spawnX = posX + 20.0f + static_cast<float>(rand() % static_cast<int>(width - 64.0f));
				spawnY = posY + height - 32.0f;
				angle = 270.0f;
				break;
			case 2: // left, moving right
				spawnX = posX + 20.0f;
				spawnY = posY + 20.0f + static_cast<float>(rand() % static_cast<int>(height - 64.0f));
				angle = 0.0f;
				break;
			case 3: // right, moving left
				spawnX = posX + width - 32.0f;
				spawnY = posY + 20.0f + static_cast<float>(rand() % static_cast<int>(height - 64.0f));
				angle = 180.0f;
				break;
			}

			SpawnProjectileAtAngle(d3dDevice, spawnX, spawnY, angle, speed);
			++spawningProjectile;
		}
	}

	for (Projectile* projectile : projectiles) {
		if (!projectile->IsActive())
			continue;
		if (attackTimedOut) {
			projectile->Deactivate();
			continue;
		}

		projectile->Update();
		D3DXVECTOR2 position = projectile->GetPosition();

		if (projectile->GetType() == ProjectileType::star) {
			D3DXVECTOR2 velocity = projectile->GetVelocity();
			const float starSize = 32.0f;
			if (position.x <= posX) {
				position.x = posX;
				velocity.x = fabsf(velocity.x);
			}
			else if (position.x + starSize >= posX + width) {
				position.x = posX + width - starSize;
				velocity.x = -fabsf(velocity.x);
			}
			if (position.y <= posY) {
				position.y = posY;
				velocity.y = fabsf(velocity.y);
			}
			else if (position.y + starSize >= posY + height) {
				position.y = posY + height - starSize;
				velocity.y = -fabsf(velocity.y);
			}
			projectile->SetPosition(position.x, position.y);
			projectile->SetVelocity(velocity.x, velocity.y);
		}
		else {
			// The PNG has transparent padding around the visible flame. Test its
			// visible pixels so the fire vanishes at the border instead of looking
			// as though it passes through the line.
			const float fireLeft = position.x + 9.0f;
			const float fireRight = position.x + 32.0f - 9.0f;
			const float fireTop = position.y + 7.0f;
			const float fireBottom = position.y + 32.0f - 7.0f;
			if (fireLeft <= posX || fireRight >= posX + width ||
				fireTop <= posY || fireBottom >= posY + height) {
				projectile->Deactivate();
				continue;
			}
		}

		AABB projectileBounds = Physics::GetBounds(projectile->GetSprite());
		if (Physics::CheckAABBCollision(heartBounds, projectileBounds)) {
			pochi->TakeDamage(enemy->GetAttackDamage());
			projectile->Deactivate();
		}
	}

	for (auto it = projectiles.begin(); it != projectiles.end();) {
		if (!(*it)->IsActive()) {
			delete *it;
			it = projectiles.erase(it);
		}
		else {
			++it;
		}
	}

	if (attackTimedOut && projectiles.empty()) {
		projectileAttackFinished = true;
		showProjectiles = false;
		OutputDebugStringA("PROJECTILE ATTACK FINISHED!\n");
	}
}

void Battlefield::UpdateStarBounce(Projectile* projectile) {
	D3DXVECTOR2 position = projectile->GetPosition();
	D3DXVECTOR2 velocity = projectile->GetVelocity();

	if (position.x <= posX || position.x + 64.0f >= posX + width) {
		velocity.x *= -1.0f;
		projectile->SetVelocity(velocity.x, velocity.y);
	}
	if (position.y <= posY || position.y + 64.0f >= posY + height) {
		velocity.y *= -1.0f;
		projectile->SetVelocity(velocity.x, velocity.y);
	}
}

bool Battlefield::IsPlayerDefeated() const {
	return pochi != nullptr && !pochi->isAlive();
}

bool Battlefield::IsEnemyDefeated() const {
	return enemy != nullptr && !enemy->isAlive();
}

bool Battlefield::HasFled() const {
	return fled;
}

void Battlefield::Render(LPD3DXSPRITE sharedBrush) {
	heart->Render(sharedBrush);

	if (showProjectiles) {
		for (Projectile* projectile : projectiles) {
			projectile->Render(sharedBrush);
		}
	}

	enemy->Render(sharedBrush, enemyHitAnimating
		? D3DCOLOR_XRGB(255, 60, 60)
		: D3DCOLOR_XRGB(255, 255, 255));

	// Player health + shield bars (top-left) and the enemy HP bar (under
	// the enemy) - all framed strips, driven by live values.
	const float enemyRatio = enemy->GetMaxHealth() > 0
		? std::clamp(displayedEnemyHealth / enemy->GetMaxHealth(), 0.0f, 1.0f) : 0.0f;

	if (playerBars != nullptr && pochi != nullptr) {
		playerBars->Draw(sharedBrush, *pochi, enemyRatio);
	}
}
