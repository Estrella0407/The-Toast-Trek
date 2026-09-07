#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "GameObject.h"

// Pochi - the player character. Owns her sprite sheet (through GameObject)
// and her RPG stats (level / health / armor / attack). One instance lives
// for the whole run: the overworld drives her movement, the battle reads
// her stats.
class Pochi : public GameObject {
private:
	int level;
	int savedLevel;       // Level to return to when special mode ends
	bool specialMode;

	int health;
	int maxHealth;

	int armor;
	int maxArmor;

	int attackDamage;

public:
	explicit Pochi(IDirect3DDevice9* device, int level = 1);

	// --- Movement / pose (delegates to the owned sprite) ---
	void Move(float dx, float dy);
	void CropToFrame(int frame = 0);
	void AnimateWalk(animationState dir);
	void AnimateWalk();
	void SetIdlePose();

	// --- Damage or healing ---
	void TakeDamage(int damage);
	void Heal(int amount);
	void RecoverArmor(int amount);
	void SetLevel(int newLevel);
	// Special lvl
	void SetMaxHealth(int maxHealth);
	void SetHealth(int health);
	void SetMaxArmor(int maxArmor);
	void SetArmor(int armor);
	void SetAttackDamage(int attackDamage);
	void RestoreFull();

	// The Mr Andrew fight:
	// on -> HP 99 / armor 50 / AD 99 (refilled);
	// off -> back to the level Pochi had before. Idempotent
	void SetSpecialMode(bool on);
	bool IsSpecialMode() const { return specialMode; }

	int GetLevel() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	int GetArmor() const;
	int GetMaxArmor() const;
	int GetAttackDamage() const;

	bool IsAlive() const;
};
