#pragma once
#include <d3d9.h>

class TileMap;

// Loads and owns every overworld TileMap for the run, with the solid /
// walkable layer setup each one needs. Scenes borrow the pointers; the
// library frees them.
class MapLibrary {
private:
    TileMap* forest;
    TileMap* maze;
    TileMap* ruinsExterior;
    TileMap* ruinsInterior;
    TileMap* tarumt;

public:
    MapLibrary();
    ~MapLibrary();

    void Load(IDirect3DDevice9* device);

    TileMap* Forest() const { return forest; }
    TileMap* Maze() const { return maze; }
    TileMap* RuinsExterior() const { return ruinsExterior; }
    TileMap* RuinsInterior() const { return ruinsInterior; }
    TileMap* Tarumt() const { return tarumt; }
};
