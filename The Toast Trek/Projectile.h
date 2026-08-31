#pragma once
#include <d3d9.h>
#include <d3dx9.h>

enum class ProjectileType {
	fire,
	star,
	aim,
	bullet
};

class Sprite;
struct AABB;
class Projectile {
private:
	Sprite* sprite;
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
	void Render(LPD3DXSPRITE sharedBrush);

	D3DXVECTOR2 GetPosition() const;
	void SetPosition(float x, float y);
	D3DXVECTOR2 GetVelocity() const;
	void SetVelocity(float velocityX, float velocityY);

	ProjectileType GetType() const;
	Sprite* GetSprite() const;
	AABB GetCollisionBounds() const;
	bool IsActive() const;
	void Deactivate();
	bool HasAppliedDamage() const;
	void MarkDamageApplied();
};
