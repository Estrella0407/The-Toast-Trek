#include "Battlefield.h"
#include "Heart.h"
#include "Physics.h"
#include <string>
#include <cmath>
#include <cstdlib>


Battlefield::Battlefield(IDirect3DDevice9* d3dDevice, BossId bossId) {
	this->d3dDevice = d3dDevice;

	posX = 300.0f;
	posY = 200.0f;

	width = 700.0f;
	height = 300.0f;

	projectileTimer = 0.0f;
	projectileSpawnInterval = 60.0f;

	topLine = new Line(d3dDevice, posX, posY, posX + width, posY);
	bottomLine = new Line(d3dDevice, posX, posY + height, posX + width, posY + height);
	leftLine = new Line(d3dDevice, posX, posY, posX, posY + height);
	rightLine = new Line(d3dDevice, posX + width, posY, posX + width, posY + height);

	heart = new Heart(d3dDevice);
	heart->SetPosition(posX + width / 2.0f - 32.0f, posY + height / 2.0f - 32.0f);

	enemy = CreateBossEnemy(d3dDevice, bossId, 600.0f, 50.0f);
	fightButton = new Sprite(d3dDevice, "Assets/UI/fightButton1.png", 256, 256, 1, 1, 1, 220.0f, 450.0f);
	actButton = new Sprite(d3dDevice, "Assets/UI/actButton1.png", 256, 256, 1, 1, 1, 420.0f, 450.0f);
	itemButton = new Sprite(d3dDevice, "Assets/UI/itemButton1.png", 256, 256, 1, 1, 1, 620.0f, 450.0f);
	mercyButton = new Sprite(d3dDevice, "Assets/UI/mercyButton1.png", 256, 256, 1, 1, 1, 820.0f, 450.0f);

	//Button when hover
	fightHovered = false;
	actHovered = false;
	itemHovered = false;
	mercyHovered = false;

	mouseWasDown = false;
	fled = false;
	fightDamage = 10;
	itemHealAmount = 5;
}

void Battlefield::SpawnProjectile(IDirect3DDevice9* d3dDevice, float x, float y, float velocityX, float velocityY) {
	projectiles.push_back(new Projectile(d3dDevice, x, y, velocityX, velocityY));
}

void Battlefield::SpawnProjectileAtAngle(IDirect3DDevice9* d3dDevice, float x, float y, float angleDegrees, float speed) {
	
	float angleRadians = D3DXToRadian(angleDegrees);
	float velocityX = cosf(angleRadians) * speed;
	float velocityY = sinf(angleRadians) * speed;

	OutputDebugStringA(("Velocity X: " + std::to_string(velocityX) + "Velocity Y: " + std::to_string(velocityY) + "\n").c_str());
	SpawnProjectile(d3dDevice, x, y, velocityX, velocityY);
}

void Battlefield::FourDirectionAttack() {
	float speed = 1.0f;
	SpawnProjectileAtAngle(d3dDevice, posX + width / 2.0f, posY + 20.0f, 90.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + width / 2.0f, posY + height - 20.0f, 270.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + 20.0f, posY + height / 20.0f, 0.0f, speed);
	SpawnProjectileAtAngle(d3dDevice, posX + width - 2.0f, posY + height / 2.0f, 180.0f, speed);
}

Battlefield::~Battlefield() {
	delete topLine;
	delete bottomLine;
	delete leftLine;
	delete rightLine;
	delete heart;
	delete enemy;
	delete fightButton;
	delete actButton;
	delete itemButton;
	delete mercyButton;

	for (Projectile* projectile : projectiles) {
		delete projectile;
	}
	projectiles.clear();
}

void Battlefield::Init() {

}

bool Battlefield::IsPointOverButton(float pointX, float pointY, Sprite* button) const {
	if (button == nullptr) return false;

	// Button art is a 256x256 canvas with a much smaller pill-shaped label
	// centered in it; hit-testing the full canvas would make adjacent
	// buttons' clickable areas overlap (they're only spaced 200px apart).
	const float hitWidth = 180.0f;
	const float hitHeight = 100.0f;

	D3DXVECTOR2 topLeft = button->GetPosition();
	float centerX = topLeft.x + 128.0f;
	float centerY = topLeft.y + 128.0f;

	return pointX >= centerX - hitWidth * 0.5f && pointX <= centerX + hitWidth * 0.5f &&
		pointY >= centerY - hitHeight * 0.5f && pointY <= centerY + hitHeight * 0.5f;
}

void Battlefield::UpdateMenuButtons(GameContext& context) {
	fightHovered = IsPointOverButton(context.mouseX, context.mouseY, fightButton);
	actHovered = IsPointOverButton(context.mouseX, context.mouseY, actButton);
	itemHovered = IsPointOverButton(context.mouseX, context.mouseY, itemButton);
	mercyHovered = IsPointOverButton(context.mouseX, context.mouseY, mercyButton);

	bool clicked = context.mouseLeftDown && !mouseWasDown;
	mouseWasDown = context.mouseLeftDown;

	if (!clicked) return;

	if (fightHovered) {
		enemy->TakeDamage(fightDamage);
		OutputDebugStringA(("Enemy hit for " + std::to_string(fightDamage) +
			", health now " + std::to_string(enemy->GetHealth()) + "\n").c_str());
	}
	else if (itemHovered) {
		heart->Heal(itemHealAmount);
	}
	else if (mercyHovered) {
		fled = true;
	}
	else if (actHovered) {
		// ACT has no per-boss dialogue/options defined yet - clickable, but a no-op for now.
	}
}

void Battlefield::Update(GameContext& context) {
	heart->Update(context.keys);

	heart->ClampToBoundary(posX, posY, posX + width, posY + height);
	AABB heartBounds = Physics::GetHeartBounds(heart->GetSprite());

	UpdateMenuButtons(context);


		for (Projectile* projectile : projectiles) {
			if (!projectile->IsActive())
				continue;

			projectile->Update();

			float projectileWidth = 32.0f;
			float projectileHeight = 32.0f;
			float fireTop = 7.0f;
			float fireLeft = 9.0f;
			float fireBottom = 7.0f;
			float fireRight = 9.0f;

			D3DXVECTOR2 projectilePosition = projectile->GetPosition();
			if (projectilePosition.x + 32.0f - fireRight > posX + width ||
				projectilePosition.x + fireLeft < posX ||
				projectilePosition.y + 32.0f - fireBottom > posY + height ||
				projectilePosition.y + fireTop < posY) {
				projectile->Deactivate();
				continue;
			}

			AABB projectileBounds = Physics::GetBounds(projectile->GetSprite());

			if (Physics::CheckAABBCollision(heartBounds, projectileBounds)) {
				heart->TakeDamage(1);
				projectile->Deactivate();
				OutputDebugStringA(
					("Health: " + std::to_string(heart->GetHealth()) + "\n").c_str());
			}
		}

		for (auto it = projectiles.begin(); it != projectiles.end();) {
			Projectile* projectile = *it;

			if (!projectile->IsActive()) {
				delete projectile;
				it = projectiles.erase(it);
			}
			else {
				++it;
			}
		}

		projectileTimer++;
		if (projectileTimer >= projectileSpawnInterval) {
			projectileTimer = 0.0f;

		float speed = 1.0f;
		int side = rand() % 4;
		//float centerX = posX + width / 2.0f;
		float spawnX = 0.0f;
		float spawnY = 0.0f;
		float angle = 0.0f;

		switch (side) {
		case 0: //TOP
			spawnX = posX + 20.0f + static_cast<float>(rand() % static_cast<int>(width - 40.0f));

			spawnY = posY + 20.0f;
			angle = 90.0f;
			break;

		case 1: //BOTTOM
			spawnX = posX + 20.0f + static_cast<float>(rand() % static_cast<int>(width - 40.0f));

			spawnY = posY + height - 32.0f;
			angle = 270.0f;
			break;

		case 2: //LEFT
			spawnX = posX + 20.0f;
			spawnY = posY + 20.0f + static_cast<float>(rand() % static_cast<int>(height - 40.0f));
			angle = 0.0f;
			break;

		case 3: //RIGHT
			spawnX = posX + width - 32.0f;
			spawnY = posY + 20.0f + static_cast<float>(rand() % static_cast<int>(height - 40.0f));
			angle = 180.0f;
			break;
		}
		SpawnProjectileAtAngle(d3dDevice, spawnX, spawnY, angle, speed);	
	}

		//attackTimer++;
		//if (!attackActive) {		
		//		StartRandomAttack();
		//				
		//}
		//else if (attackTimer >= attackInterval){
		//	attackTimer = false;
		//}
	}

bool Battlefield::IsPlayerDefeated() const {
	return heart->GetHealth() <= 0;
}

bool Battlefield::IsEnemyDefeated() const {
	return enemy != nullptr && !enemy->isAlive();
}

bool Battlefield::HasFled() const {
	return fled;
}

void Battlefield::Render(LPD3DXSPRITE sharedBrush) {
	D3DCOLOR black = D3DCOLOR_XRGB(0, 0, 0);
	topLine->Draw(black);
	bottomLine->Draw(black);
	leftLine->Draw(black);
	rightLine->Draw(black);

	heart->Render(sharedBrush);

	for (Projectile* projectile : projectiles) {
		projectile->Render(sharedBrush);
	}

	enemy->Render(sharedBrush);

	D3DCOLOR normalTint = D3DCOLOR_XRGB(255, 255, 255);
	D3DCOLOR hoverTint = D3DCOLOR_XRGB(255, 225, 120);
	fightButton->Draw(sharedBrush, fightHovered ? hoverTint : normalTint);
	actButton->Draw(sharedBrush, actHovered ? hoverTint : normalTint);
	itemButton->Draw(sharedBrush, itemHovered ? hoverTint : normalTint);
	mercyButton->Draw(sharedBrush, mercyHovered ? hoverTint : normalTint);
}