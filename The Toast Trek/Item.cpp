#include "Item.h"

Item::Item(IDirect3DDevice9* device, ItemType type, const char* texture,
           int texWidth, int texHeight, float x, float y, float scale)
    : GameObject(new Sprite(device, texture, texWidth, texHeight, 1, 1, 1, x, y)), type(type)
{
    if (sprite != nullptr) sprite->SetScale(scale);
}
