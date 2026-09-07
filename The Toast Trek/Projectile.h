#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "GameObject.h"
#include "AABB.h"

enum class ProjectileType {
	fire,
	star,
	aim,
	bullet
};

// A single enemy bullet in the battle box. Sprite / position / bounds come
// from GameObject; the per-frame motion keeps its own simple velocity add
// (no dt) so the bullet-hell timing is unchanged.
class Projectile : public GameObject {
private:
	ProjectileType type;

	D3DXVECTOR2 velocity;

	float width;
	float height;
	bool active;
	bool damageApplied;

	int frameCounter;
	int frameDelay;

public:
	Projectile(IDirect3DDevice9* d3dDevice, float startX, float startY, float velocityX, float velocityY, ProjectileType type);
	~Projectile();
	void Update();
	void Render(LPD3DXSPRITE sharedBrush, D3DCOLOR tint = D3DCOLOR_XRGB(255, 255, 255)) override;

	D3DXVECTOR2 GetVelocity() const;
	void SetVelocity(float velocityX, float velocityY);

	ProjectileType GetType() const;
	AABB GetCollisionBounds() const;
	bool IsActive() const;
	void Deactivate();
	bool HasAppliedDamage() const;
	void MarkDamageApplied();
};
