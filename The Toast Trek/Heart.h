#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "Sprite.h"

class Heart {
private:
	Sprite* sprite;
	int health;
	int maxHealth;
	float moveSpeed;
	
	float boundaryLeft;
	float boundaryRight;
	float boundaryTop;
	float boundaryBottom;

public:
	Heart(IDirect3DDevice9* d3dDevice);
	~Heart();

	void Update(BYTE* keys);
	void Render(LPD3DXSPRITE sharedBrush);

	void Move(float deltaTime);

	void setBoundaries(float left, float right, float top, float bottom);
	void TakeDamage(int damage);
	void Heal(int amount);

	int GetHealth() const;
	int GetMaxHealth() const;
	D3DXVECTOR2 GetPosition() const;
	Sprite* GetSprite() const;
	void SetPosition(float x, float y);

	void ClampToBoundary(float left, float top, float right, float bottom);
};
