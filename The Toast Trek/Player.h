#pragma once
#include <d3d9.h>
#include "GameObject.h"

// Pochi in the overworld. Loads and owns her sprite sheet; the walk /
// collision logic still lives in OverworldScene, which borrows GetSprite().
class Player : public GameObject {
public:
    explicit Player(IDirect3DDevice9* device);
};
