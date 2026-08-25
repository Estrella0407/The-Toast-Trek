#include "Battlefield.h"
#include "Heart.h"
#include "Physics.h"
#include <string>
#include <cmath>
#include <cstdlib>


Battlefield::Battlefield(IDirect3DDevice9* d3dDevice) {
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

	enemy = new Enemy(d3dDevice, "Assets/characters/skullBones.png", 600.0f, 50.0f, 50);
	fightButton = new Sprite(d3dDevice, "Assets/UI/fightButton1.png", 256, 256, 1, 1, 1, 220.0f, 450.0f);
	actButton = new Sprite(d3dDevice, "Assets/UI/actButton1.png", 256, 256, 1, 1, 1, 420.0f, 450.0f);
	itemButton = new Sprite(d3dDevice, "Assets/UI/itemButton1.png", 256, 256, 1, 1, 1, 620.0f, 450.0f);
	mercyButton = new Sprite(d3dDevice, "Assets/UI/mercyButton1.png", 256, 256, 1, 1, 1, 820.0f, 450.0f);

	//Button when hover 
	fightHovered = false;
	actHovered = false;
	itemHovered = false;
	mercyHovered = false;

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

void Battlefield::Update(BYTE* keys) {
	heart->Update(keys);

	heart->ClampToBoundary(posX, posY, posX + width, posY + height);
	AABB heartBounds = Physics::GetHeartBounds(heart->GetSprite());


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
	fightButton->Draw(sharedBrush);
	actButton->Draw(sharedBrush);
	itemButton->Draw(sharedBrush);
	mercyButton->Draw(sharedBrush);
	
}