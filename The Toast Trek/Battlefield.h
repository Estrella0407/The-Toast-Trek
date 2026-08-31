#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include "Pochi.h"
#include "Heart.h"
#include "Projectile.h"
#include "Enemy.h"
#include "BattleUI.h"
#include "GameState.h" // GameContext (mouse input)
#include "Inventory.h"

class Pochi;
class BattleUI;
class Battlefield {
private:
	IDirect3DDevice9* d3dDevice;
	
	float posX;
	float posY;

	float width;
	float height;

	Pochi* pochi;
	Heart* heart;
	std::vector<Projectile*> projectiles;
	Enemy* enemy;
	BattleUI* battleUI;
	Inventory* inventory;
	Font* statusFont;
	float displayedEnemyHealth;
	float hitStartHealth;
	unsigned long long hitAnimationStart;
	unsigned long long lastPlayerHitTime;
	bool enemyHitAnimating;

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
	unsigned long long projectileAttackStart;
	unsigned long long projectileAttackDuration;
	int spawningProjectile;			//when to display projectile
	int maxProjectiles;

	bool IsPointOverButton(float pointX, float pointY, BattleButton* button) const;
	//void UpdateMenuButtons(GameContext& context);

	void FourDirectionAttack();
	void SpawnProjectile(IDirect3DDevice9* d3dDevice, float x, float y, float velocityX, float velocityY, ProjectileType type);
	void SpawnProjectileAtAngle(IDirect3DDevice9* d3dDevice, float x, float y, float angleDegrees, float speed);
	void StarBounceAttack();
	void UpdateStarBounce(Projectile* projectile);
	
public:
	Battlefield(IDirect3DDevice9* d3dDevice, BattleUI* battleUI, Enemy* enemy, Pochi* pochi, Inventory* inventory);
	~Battlefield();

	void Init();
	void Update(GameContext& context);
	void Render(LPD3DXSPRITE sharedBrush);
	int GetSelectButton(GameContext& context);
	void UpdateMenuButtons(GameContext& context);

	void StartEnemyAttack();
	void PerformFight();	//fight button damage
	void PerformAct();		//act damage for act cute, roll on ground and bark. All deal the same damage by 2
	bool IsEnemyHitAnimationFinished() const;
	bool PerformItem(ItemType item);
	void Flee();
	void SetShowProjectiles(bool show);
	bool IsProjectileAttackFinished() const;

	void StartActAnimation();
	void UpdateActAnimation();
	bool IsActAnimationFinished() const;

	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;
};
