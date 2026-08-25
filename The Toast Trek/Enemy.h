#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

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
		int health);
	~Enemy();

	void Render(LPD3DXSPRITE sharedBrush);
	void TakeDamage(int damage);
	int GetHealth() const;
	int GetMaxHealth() const;
	Sprite* GetSprite() const;
	bool isAlive() const;

};
