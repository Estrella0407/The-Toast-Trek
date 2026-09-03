#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>

// One <tileset> entry from the .tmx: where its gid range starts, how many
// columns of tiles its source image has, and the loaded DirectX texture
struct TilesetInfo {
    int firstGid;
    int columns;
    int tileWidth;
    int tileHeight;
    int imageWidth;
    int imageHeight;
    std::string imageFile;
    LPDIRECT3DTEXTURE9 texture;
};

// One <layer> entry: its name and one gid (tile id) per tile, row-major,
// size == mapWidthTiles * mapHeightTiles. A gid of 0 means "empty / no tile"
struct MapLayer {
    std::string name;
    std::vector<int> tileIds;
};

class TileMap {
private:
    IDirect3DDevice9* device;
    int mapWidthTiles, mapHeightTiles;
    int tileWidth, tileHeight;

    std::vector<TilesetInfo> tilesets;
    std::vector<MapLayer> layers;

    // Names of the layers that should block movement, set via SetSolidLayers()
    // Any tile placed on one of these layers (gid != 0)  makes that grid cell solid
    // Every other layer is purely visual
    std::vector<std::string> solidLayerNames;

    // Names of the layers that define "land", set via SetWalkableLayers()
    // Empty (the default) disables this check entirely
    std::vector<std::string> walkableLayerNames;

    // One flag per map cell (mapWidthTiles * mapHeightTiles, row-major)
    // Rebuilt by BuildSolidGrid() whenever SetSolidLayers() or SetWalkableLayers() is called
    std::vector<bool> solidGrid;

    // Tiled uses the top 3 bits of a gid as flip flags (horizontal/vertical/ Diagonal)
    // Strip = off before looking the tile up in a tileset
    static int StripFlipFlags(unsigned int gid);

    // Finds which tileset a (flip-stripped) gid belongs to
    // (the tileset with the largest firstGid that is still <= gid)
    // Returns -1 for gid <= 0
    int FindTilesetIndex(int gid) const;

    // Draws exactly the given layer indices, in the order listed
    // (callers pass them in ascending/TMX order so tiles composite correctly)
    void DrawLayers(LPD3DXSPRITE sharedBrush, const std::vector<size_t>& layerIndices);
    void BuildSolidGrid();

public:
    TileMap(IDirect3DDevice9* d3dDevice, const char* tmxFilePath, const char* assetFolder = "");
    ~TileMap();

    void Draw(LPD3DXSPRITE sharedBrush);

    // Draws every layer EXCEPT the ones named, in TMX order
    // Draw a character between the map and a foreground layer (with DrawOnlyLayers())
    void DrawExcludingLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& excludeNames);

    // Draws only the named layers, in TMX order
    void DrawOnlyLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& includeNames);

    int GetWidthPixels() const;
    int GetHeightPixels() const;
    int GetTileWidth() const { return tileWidth; }
    int GetTileHeight() const { return tileHeight; }

    // Choose which layers block movement
    void SetSolidLayers(const std::vector<std::string>& layerNames);

    void SetWalkableLayers(const std::vector<std::string>& layerNames);

    // True if the tile at (tileX, tileY) is solid
    bool IsTileSolid(int tileX, int tileY) const;
};