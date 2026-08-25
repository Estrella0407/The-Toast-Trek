#pragma once
#include <d3d9.h>
#include <d3dx9.h>

class Sprite;

class Projectile {
private:
	Sprite* sprite;

	D3DXVECTOR2 velocity;

	float width;
	float height;
	bool active;

public:
	Projectile(IDirect3DDevice9* d3dDevice, float startX, float startY, float velocityX, float velocityY);
	~Projectile();
	void Update();
	void Render(LPD3DXSPRITE sharedBrush);
	D3DXVECTOR2 GetPosition() const;
	Sprite* GetSprite() const;
	bool IsActive() const;
	void Deactivate();
};
