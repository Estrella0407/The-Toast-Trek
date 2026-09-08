#include "MapLibrary.h"
#include "TileMap.h"

MapLibrary::MapLibrary()
    : forest(nullptr), maze(nullptr), ruinsExterior(nullptr),
      ruinsInterior(nullptr), tarumt(nullptr)
{
}

MapLibrary::~MapLibrary()
{
    delete forest;
    delete maze;
    delete ruinsExterior;
    delete ruinsInterior;
    delete tarumt;
}

void MapLibrary::Load(IDirect3DDevice9* device)
{
    forest = new TileMap(device, "Assets/TileMap/Forest.tmx", "Assets/TileMap/");
    forest->SetSolidLayers({ "Tree", "Rock" });

    maze = new TileMap(device, "Assets/TileMap/Maze.tmx", "Assets/TileMap/");
    maze->SetSolidLayers({ "Maze" });

    ruinsExterior = new TileMap(device, "Assets/TileMap/Ruined_Temple_Exterior.tmx", "Assets/TileMap/");
    ruinsExterior->SetSolidLayers({ "Tree", "House", "Bricks", "Statues", "Columns" });
    ruinsExterior->SetWalkableLayers({ "Ground", "Grass", "Spots", "Grass_details", "Site", "House_platform" });

    ruinsInterior = new TileMap(device, "Assets/TileMap/Ruined_Temple_Interior.tmx", "Assets/TileMap/");
    // Decorative_objects1/2 are floor clutter (pots, rubble, banner poles) -
    // they were blocking the path up to Maki, so only the real walls +
    // the dragon statue are solid
    ruinsInterior->SetSolidLayers({ "Walls_back", "Walls_top", "Statue" });

    // Secret-boss area, reached from the forest's top-left
    tarumt = new TileMap(device, "Assets/TileMap/Tarumt.tmx", "Assets/TileMap/");
    tarumt->SetSolidLayers({ "Tree", "Structure1", "Structure2", "Building" });
}
