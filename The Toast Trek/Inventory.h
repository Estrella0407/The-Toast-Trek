#pragma once

enum class ItemType {
	HealthPotion,
	Bone,
	Toast
};

class Inventory {
private:
	int healthPotions;
	int bones;
	int toast;

public:
	Inventory();

	void Add(ItemType item);
	bool Consume(ItemType item);
	int GetCount(ItemType item) const;
	void SetCount(ItemType item, int count);   // Used when loading a save

	// Empty the pack - called when a fresh run starts
	void Reset();
};
