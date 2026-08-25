#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include "Heart.h"
#include "Line.h"
#include "Projectile.h"
#include "Enemy.h"
#include "GameState.h" // GameContext (mouse input)

class Battlefield {
private:
	IDirect3DDevice9* d3dDevice;
	
	float posX;
	float posY;

	float width;
	float height;

	Line* topLine;
	Line* bottomLine;
	Line* leftLine;
	Line* rightLine;

	Heart* heart;
	std::vector<Projectile*> projectiles;
	Enemy* enemy;
	Sprite* fightButton;
	Sprite* actButton;
	Sprite* itemButton;
	Sprite* mercyButton;

	//Button when hover
	bool fightHovered;
	bool actHovered;
	bool itemHovered;
	bool mercyHovered;

	bool mouseWasDown;
	bool fled;
	int fightDamage;
	int itemHealAmount;

	float projectileTimer;
	float projectileSpawnInterval;
	float horizontalSweepTimer;
	float horizontalSweepInterval;

	bool IsPointOverButton(float pointX, float pointY, Sprite* button) const;
	void UpdateMenuButtons(GameContext& context);

	void FourDirectionAttack();
	void SpawnProjectile(
		IDirect3DDevice9* d3dDevice, 
		float x, 
		float y, 
		float velocityX, 
		float velocityY);

	void SpawnProjectileAtAngle(
		IDirect3DDevice9* d3dDevice,
		float x,
		float y,
		float angleDegrees,
		float speed);
	
public:
	Battlefield(IDirect3DDevice9* d3dDevice, BossId bossId);
	~Battlefield();

	void Init();
	void Update(GameContext& context);
	void Render(LPD3DXSPRITE sharedBrush);

	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;
};