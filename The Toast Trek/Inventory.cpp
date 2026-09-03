#include "Inventory.h"

Inventory::Inventory() : healthPotions(0), bones(0), toast(0) {}

void Inventory::Add(ItemType item) {
	if (item == ItemType::HealthPotion) ++healthPotions;
	else if (item == ItemType::Bone) ++bones;
	else if (item == ItemType::Toast) ++toast;
}

bool Inventory::Consume(ItemType item) {
	int* count = &toast;
	if (item == ItemType::HealthPotion) count = &healthPotions;
	else if (item == ItemType::Bone) count = &bones;
	if (*count <= 0) return false;
	--(*count);
	return true;
}

int Inventory::GetCount(ItemType item) const {
	if (item == ItemType::HealthPotion) return healthPotions;
	if (item == ItemType::Bone) return bones;
	return toast;
}

void Inventory::SetCount(ItemType item, int count) {
	if (count < 0) count = 0;
	if (item == ItemType::HealthPotion) healthPotions = count;
	else if (item == ItemType::Bone) bones = count;
	else if (item == ItemType::Toast) toast = count;
}

void Inventory::Reset() {
	healthPotions = 0;
	bones = 0;
	toast = 0;
}
