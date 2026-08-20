#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>

// One <tileset> entry from the .tmx: where its gid range starts, how many
// columns of tiles its source image has, and the loaded DirectX texture.
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
// size == mapWidthTiles * mapHeightTiles. A gid of 0 means "empty / no tile".
struct MapLayer {
    std::string name;
    std::vector<int> tileIds;
};

class TileMap {
private:
    IDirect3DDevice9* device;
    int mapWidthTiles, mapHeightTiles;
    int tileWidth, tileHeight;
    bool debugForceSingleTile;

    std::vector<TilesetInfo> tilesets;
    std::vector<MapLayer> layers;

    // Names of the layers that should block movement, set via
    // SetSolidLayers(). Any tile placed on one of these layers (gid != 0)
    // makes that grid cell solid; every other layer is purely visual.
    std::vector<std::string> solidLayerNames;

    // One flag per map cell (mapWidthTiles * mapHeightTiles, row-major),
    // rebuilt by BuildSolidGrid() whenever SetSolidLayers() is called.
    std::vector<bool> solidGrid;

    // Tiled uses the top 3 bits of a gid as flip flags (horizontal/vertical/
    // diagonal). Strip them off before looking the tile up in a tileset.
    static int StripFlipFlags(unsigned int gid);

    // Finds which tileset a (flip-stripped) gid belongs to (the tileset with
    // the largest firstGid that is still <= gid). Returns -1 for gid <= 0.
    int FindTilesetIndex(int gid) const;
    int FindLayerIndex(const std::string& layerName) const;

    // Draws exactly the given layer indices, in the order listed (callers
    // pass them in ascending/TMX order so tiles composite correctly).
    void DrawLayers(LPD3DXSPRITE sharedBrush, const std::vector<size_t>& layerIndices);
    void BuildSolidGrid();

public:
    TileMap(IDirect3DDevice9* d3dDevice, const char* tmxFilePath, const char* assetFolder = "");
    ~TileMap();

    void Draw(LPD3DXSPRITE sharedBrush);

    // Draws every layer EXCEPT the ones named, in TMX order. Pair with
    // DrawOnlyLayers() to draw a character between the map and a foreground
    // layer (e.g. a tree canopy) - unlike splitting by TMX order, this
    // doesn't care where the named layer(s) sit in the file, so reordering
    // layers in Tiled (e.g. moving Bushes) can't silently change what's
    // drawn behind vs. in front of the character.
    void DrawExcludingLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& excludeNames);

    // Draws only the named layers, in TMX order. See DrawExcludingLayers().
    void DrawOnlyLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& includeNames);

    int GetWidthPixels() const;
    int GetHeightPixels() const;
    int GetTileWidth() const { return tileWidth; }
    int GetTileHeight() const { return tileHeight; }
    int GetWidthTiles() const { return mapWidthTiles; }
    int GetHeightTiles() const { return mapHeightTiles; }
    bool HasLayer(const std::string& layerName) const;

    // Choose which layers block movement, e.g. {"Tree", "Rock", "Bushes"}.
    // Any tile placed on one of these layers is solid; everything else
    // (ground, grass, decals, the leaf canopy, etc.) is walked over/under
    // freely, with no shapes to author in Tiled. Call once after
    // construction - each call replaces the previous list.
    void SetSolidLayers(const std::vector<std::string>& layerNames);

    // True if the tile at (tileX, tileY) is solid - i.e. it has a non-empty
    // gid on one of the layers passed to SetSolidLayers(). Out-of-range
    // coordinates are always non-solid.
    bool IsTileSolid(int tileX, int tileY) const;

    // TEMPORARY DIAGNOSTIC: when true, Draw() ignores the CSV data entirely
    // and draws ONE known-solid tile (gid 738, in "exterior") everywhere,
    // for every cell, on ONE layer only. If the checkerboard STILL appears
    // with this on, the bug is in the draw pipeline itself (every call is
    // affected the same way, regardless of which tile). If it goes away,
    // the bug is data-dependent (something about switching between
    // different tiles/gids). Remove once diagnosed.
    void SetDebugForceSingleTile(bool enabled);

    // Pops up a MessageBox listing every tileset (firstGid/columns/image)
    // and layer (name/tile count) that got parsed. Not called automatically
    // anymore - call it yourself if you ever need to sanity-check parsing again.
    void PrintDiagnosticReport() const;
};