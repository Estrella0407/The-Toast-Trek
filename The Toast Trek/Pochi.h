#pragma once

class Pochi {
private:
	int level;

	int health;
	int maxHealth;

	int armor;
	int maxArmor;

	int attackDamage;
public:
	Pochi(int level = 1);

	//Damage or healing
	void TakeDamage(int damage);
	void Heal(int amount);
	void RecoverArmor(int amount);
	void SetLevel(int newLevel);
	//Special lvl
	void SetMaxHealth(int maxHealth);
	void SetHealth(int health);
	void SetMaxArmor(int maxArmor);
	void SetArmor(int armor);
	void SetAttackDamage(int attackDamage);
	void RestoreFull();

	void IncreaseMaxHealth(int amount);
	void IncreaseMaxArmor(int amount);

	int GetLevel() const;
	int GetHealth() const;
	int GetMaxHealth() const;
	int GetArmor() const;
	int GetMaxArmor() const;
	int GetAttackDamage() const;

	bool isAlive() const;
};
