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
class BattleStatusBars;
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
	BattleStatusBars* playerBars;
	float displayedEnemyHealth;
	float hitStartHealth;
	unsigned long long hitAnimationStart;
	unsigned long long lastPlayerHitTime;
	bool enemyHitAnimating;

	bool mouseWasDown;
	bool fled;
	int fightDamage;

	float projectileTimer;
	float projectileSpawnInterval;
	bool showProjectiles;
	bool projectileAttackFinished;
	unsigned long long projectileAttackStart;
	unsigned long long projectileAttackDuration;
	int spawningProjectile;			// When to display projectile
	int maxProjectiles;

	// Maki projectile
	bool bulletAttackActive;
	bool bulletAiming;
	int bulletShotCount;
	unsigned long long bulletAimStartTime;
	unsigned long long bulletHoleStartTime;
	Projectile* currentBulletAim;
	Projectile* currentBulletHole;
	std::vector<Projectile*> currentBulletAims;
	std::vector<Projectile*> currentBulletHoles;
	std::vector<D3DXVECTOR2> currentBulletTargets;


	bool specialAttackActive;
	int specialAttackWave;		// Combine skull bone, goblin and maki projectile in one go

	// Projectiles
	void FourDirectionAttack();		// Skull bones
	void SpawnProjectile(IDirect3DDevice9* d3dDevice, float x, float y, float velocityX, float velocityY, ProjectileType type);
	void SpawnProjectileAtAngle(IDirect3DDevice9* d3dDevice, float x, float y, float angleDegrees, float speed);
	void StarBounceAttack();		// Goblin
	void UpdateStarBounce(Projectile* projectile);
	void StartGunshotAttack();		// Maki
	void UpdateGunshotAttack();
	void StartGunshotBurst();

public:
	Battlefield(IDirect3DDevice9* d3dDevice, BattleUI* battleUI, BossId bossId, Enemy* enemy, Pochi* pochi, Inventory* inventory);
	~Battlefield();

	void Update(GameContext& context);
	void Render(LPD3DXSPRITE sharedBrush);

	void StartEnemyAttack();
	void PerformFight();	// Fight button damage
	void PerformAct();		// Act damage for act cute, roll on ground and bark. All deal the same damage by 2
	bool IsEnemyHitAnimationFinished() const;
	bool PerformItem(ItemType item);
	void Flee();
	void SetShowProjectiles(bool show);
	bool IsProjectileAttackFinished() const;

	void StartSpecialBossAttack();

	void StartActAnimation();
	void UpdateActAnimation();
	bool IsActAnimationFinished() const;

	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;
};
