#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

// Identifies which boss is being fought/represented
// The maze and the battle screen can agree on a single sprite/health/scale definition
// (see CreateBossEnemy()) instead of duplicating those numbers
enum class BossId {
	SkullBones, // Level 1
	Goblin,     // Level 2
	Maki,       // Level 3 (final boss, ruins interior)
	MrAndrew	// Secret boss
};

enum class AttackType {
	FourDirection,
	StarBounce,
	Gunshot,
	SpecialAttack
};

class Enemy {
private:
	BossId bossId;
	AttackType attackType;
	Sprite* sprite;		// Static enemy image
	Sprite* actSprite;	// Act button -> enemy blushing animation
	Sprite* enemyHealthBar;

	int health;
	int maxHealth;
	int attackDamage;

	bool actAnimation;
	int actFrame;
	int maxFrames;
	int actFrameCounter;
	int actFrameDelay;

public:
	Enemy(IDirect3DDevice9* d3dDevice,
		BossId bossId,
		const char* spritePath,
		float startX,
		float startY,
		int texWidth = 128,
		int texHeight = 128,
		int cols = 1,
		int rows = 1,
		int maxFrames = 1);
	~Enemy();

	AttackType GetAttackType() const;

	void Render(LPD3DXSPRITE sharedBrush, D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255));
	void StartActAnimation();
	void UpdateActAnimation();
	bool IsActAnimationFinished() const;

	void TakeDamage(int damage);
	int GetHealth() const;
	int GetMaxHealth() const;
	int GetAttackDamage() const;
	Sprite* GetSprite() const;
	bool IsAlive() const;

};

// Builds a fully-configured Enemy for the given boss
Enemy* CreateBossEnemy(IDirect3DDevice9* d3dDevice, BossId bossId, float startX, float startY);
