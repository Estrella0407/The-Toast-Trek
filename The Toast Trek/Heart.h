#pragma once

#include <d3d9.h>
#include <d3dx9.h>
#include "GameObject.h"

// The player's avatar in the bullet-hell battle box. Its Sprite, position
// and bounds come from GameObject; this class adds the HP/shield and the
// WASD/arrow steering.
class Heart : public GameObject {
private:
	int health;
	int maxHealth;
	int shield;
	int maxShield;
	float moveSpeed;

public:
	Heart(IDirect3DDevice9* d3dDevice);
	~Heart();

	void Update(BYTE* keys);

	void TakeDamage(int damage);
	void Heal(int amount);

	int GetHealth() const;
	int GetMaxHealth() const;
	int GetShield() const;
	int GetMaxShield() const;

	void ClampToBoundary(float left, float top, float right, float bottom);
};
