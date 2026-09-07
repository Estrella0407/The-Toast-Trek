#pragma once
#include <d3d9.h>
#include "GameObject.h"
#include "Inventory.h"   // ItemType

// A pickup lying in an overworld map. Loads its own texture; OverworldScene
// walks Pochi over it and calls Type() to add it to the inventory.
class Item : public GameObject {
private:
    ItemType type;

public:
    Item(IDirect3DDevice9* device, ItemType type, const char* texture,
         int texWidth, int texHeight, float x, float y, float scale);

    ItemType Type() const { return type; }
};
