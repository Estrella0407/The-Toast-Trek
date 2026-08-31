#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

// Identifies which boss is being fought/represented, so the maze and the
// battle screen can agree on a single sprite/health/scale definition
// (see CreateBossEnemy()) instead of duplicating those numbers.
enum class BossId {
	SkullBones, // Level 1 - maze
	Goblin,     // Level 2 - maze
	Maki,       // Level 3 - final boss, ruins interior
	MrAndrew	// Secret boss
};

class Enemy {
private:
	Sprite* sprite;
	int health;
	int maxHealth;

public:
	Enemy(IDirect3DDevice9* d3dDevice,
		const char* spritePath,
		float startX,
		float startY,
		int health,
		int texWidth = 128,
		int texHeight = 128,
		int cols = 1,
		int rows = 1,
		int maxFrames = 1);
	~Enemy();

	void Render(LPD3DXSPRITE sharedBrush);
	void TakeDamage(int damage);
	int GetHealth() const;
	int GetMaxHealth() const;
	Sprite* GetSprite() const;
	bool isAlive() const;

};

// Builds a fully-configured Enemy for the given boss - sprite file, texture
// size, health and display scale all come from this one place so the maze
// (walk-up encounter) and the battle screen (portrait) always agree.
Enemy* CreateBossEnemy(IDirect3DDevice9* d3dDevice, BossId bossId, float startX, float startY);
