#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

class Heart {
private:
	Sprite* sprite;
	int health;
	int maxHealth;
	int shield;
	int maxShield;
	float moveSpeed;

public:
	Heart(IDirect3DDevice9* d3dDevice);
	~Heart();

	void Update(BYTE* keys);
	void Render(LPD3DXSPRITE sharedBrush);

	void TakeDamage(int damage);
	void Heal(int amount);

	int GetHealth() const;
	int GetMaxHealth() const;
	int GetShield() const;
	int GetMaxShield() const;
	D3DXVECTOR2 GetPosition() const;
	Sprite* GetSprite() const;
	void SetPosition(float x, float y);

	void ClampToBoundary(float left, float top, float right, float bottom);
};
