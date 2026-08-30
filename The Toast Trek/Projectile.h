#pragma once
#include <d3d9.h>
#include <d3dx9.h>

enum class ProjectileType {
	fire,
	star,
	bomb,
	sparkle
};

class Sprite;
class Projectile {
private:
	Sprite* sprite;
	ProjectileType type;

	D3DXVECTOR2 velocity;

	float width;
	float height;
	bool active;

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
	bool IsActive() const;
	void Deactivate();
};
