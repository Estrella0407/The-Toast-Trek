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

	//Maki projectile
	bool bulletAttackActive;
	bool bulletAiming;
	float bulletAimTimer;
	float bulletAimDuration;
	int bulletShotCount;
	int maxBulletShots;
	float bulletTargetX;
	float bulletTargetY;
	unsigned long long bulletAimStartTime;
	unsigned long long bulletHoleStartTime;
	Projectile* currentBulletAim;
	Projectile* currentBulletHole;
	std::vector<Projectile*> currentBulletAims;
	std::vector<Projectile*> currentBulletHoles;
	std::vector<D3DXVECTOR2> currentBulletTargets;


	bool specialAttackActive;
	int specialAttackWave;		//combine skull bone, goblin and maki projectile in one go
	//float specialAttackTimer;

	bool IsPointOverButton(float pointX, float pointY, BattleButton* button) const;
	//void UpdateMenuButtons(GameContext& context);

	//Projectiles
	void FourDirectionAttack();		//skull bones 
	void SpawnProjectile(IDirect3DDevice9* d3dDevice, float x, float y, float velocityX, float velocityY, ProjectileType type);
	void SpawnProjectileAtAngle(IDirect3DDevice9* d3dDevice, float x, float y, float angleDegrees, float speed);
	void StarBounceAttack();		//goblin
	void UpdateStarBounce(Projectile* projectile);
	void StartGunshotAttack();		//maki
	void UpdateGunshotAttack();
	void FireBullet();
	void ChooseRandomBulletTarget();
	void StartGunshotBurst();

	//Special 
	void SpecialAttack();
	void UpdateSpecialBossAttack();
	bool AreAllProjectilesInactive();

public:
	Battlefield(IDirect3DDevice9* d3dDevice, BattleUI* battleUI, BossId bossId, Enemy* enemy, Pochi* pochi, Inventory* inventory);
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

	void StartSpecialBossAttack();
	bool isSpecialAttackFinished() const;

	void StartActAnimation();
	void UpdateActAnimation();
	bool IsActAnimationFinished() const;

	bool IsPlayerDefeated() const;
	bool IsEnemyDefeated() const;
	bool HasFled() const;
};
