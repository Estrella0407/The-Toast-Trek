#include "Pochi.h"
#include <algorithm>

Pochi::Pochi(int level) : level(level), savedLevel(level), specialMode(false),
	health(0), maxHealth(0), armor(0), maxArmor(0), attackDamage(0) {
	SetLevel(level);
}

void Pochi::SetSpecialMode(bool on) {
	if (on == specialMode) return;
	if (on) {
		savedLevel = level;
		specialMode = true;
		maxHealth = 99;
		maxArmor = 50;
		attackDamage = 99;
		health = maxHealth;
		armor = maxArmor;
	}
	else {
		specialMode = false;
		SetLevel(savedLevel);   // restores stats and refills
	}
}

void Pochi::SetLevel(int newLevel) {
	// An explicit level change always ends the Mr Andrew special boost.
	specialMode = false;
	level = newLevel;
	switch (level) {
	case 1:
		maxHealth = 3;
		maxArmor = 1;
		attackDamage = 3;
		break;
	case 2:
		maxHealth = 6;
		maxArmor = 2;
		attackDamage = 6;
		break;
	case 3:
		maxHealth = 9;
		maxArmor = 3;
		attackDamage = 9;
		break;
	default:
		maxHealth = 3;
		maxArmor = 1;
		attackDamage = 3;
		break;
	}
	health = maxHealth;
	armor = maxArmor;
}

void Pochi::RestoreFull() {
	health = maxHealth;
	armor = maxArmor;
}

void Pochi::TakeDamage(int damage) {
	//Armor decrease first
	if (armor > 0) {
		int absorbed = damage;
		if (absorbed > armor)
			absorbed = armor;
		armor -= absorbed;
		damage -= absorbed;
	}
	//remaining damage go to heart
	if (damage > 0) {
		health -= damage;
		if (health < 0)
			health = 0;
	}
}

void Pochi::Heal(int amount) {
	health += amount;
	if (health > maxHealth)
		health = maxHealth;
}

void Pochi::RecoverArmor(int amount) {
	armor += amount;
	if (armor > maxArmor)
		armor = maxArmor;
}

void Pochi::IncreaseMaxHealth(int amount) {
	maxHealth += amount;
	health += amount;
}

void Pochi::IncreaseMaxArmor(int amount) {
	maxArmor += amount;
	armor += amount;
}

int Pochi::GetLevel() const {
	return level;
}

int Pochi::GetHealth() const {
	return health;
}

int Pochi::GetMaxHealth() const {
	return maxHealth;
}

int Pochi::GetArmor() const {
	return armor;
}

int Pochi::GetMaxArmor() const {
	return maxArmor;
}

int Pochi::GetAttackDamage() const {
	return attackDamage;
}

bool Pochi::isAlive() const {
	return health > 0;
}

//Special lvl
void Pochi::SetHealth(int health) {
	this->health = health;
}

void Pochi::SetMaxHealth(int maxHealth) {
	this->maxHealth = maxHealth;
}

void Pochi::SetArmor(int armor) {
	this->armor = armor;
}

void Pochi::SetMaxArmor(int maxArmor) {
	this->maxArmor = maxArmor;
}

void Pochi::SetAttackDamage(int attackDamage) {
	this->attackDamage = attackDamage;
}