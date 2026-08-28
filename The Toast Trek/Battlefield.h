#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include "Heart.h"
#include "Projectile.h"
#include "Enemy.h"
#include "BattleUI.h"
#include "GameState.h" // GameContext (mouse input)

class BattleUI;
class Battlefield {
private:
	IDirect3DDevice9* d3dDevice;
	
	float posX;
	float posY;

	float width;
	float height;

	Heart* heart;
	std::vector<Projectile*> projectiles;
	Enemy* enemy;
	BattleUI* battleUI;

	////Button when hover
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
	bool showProjectiles;
	bool projectileAttackFinished;
	//when to display projectile
	int spawningProjectile;
	int maxProjectiles;

	bool IsPointOverButton(float pointX, float pointY, BattleButton* button) const;
	//void UpdateMenuButtons(GameContext& context);

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
	Battlefield(IDirect3DDevice9* d3dDevice, BattleUI* battleUI, Enemy* enemy);
	~Battlefield();

	void Init();
	void Update(GameContext& context);
	void Render(LPD3DXSPRITE sharedBrush);
	int GetSelectButton(GameContext& context);
	void UpdateMenuButtons(GameContext& context);
	void PerformFight();
	void SetShowProjectiles(bool show);
	bool IsProjectileAttackFinished() const;

	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;
};