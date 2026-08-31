#include "TileMap.h"
#include <Windows.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <cstdio>

TileMap::TileMap(IDirect3DDevice9* d3dDevice, const char* tmxFilePath, const char* assetFolder) {
    device = d3dDevice;
    debugForceSingleTile = false;
    mapWidthTiles = 0;
    mapHeightTiles = 0;
    tileWidth = 16;
    tileHeight = 16;

    std::ifstream file(tmxFilePath);
    if (!file.is_open()) {
        std::string msg = "TileMap could not open the .tmx file:\n" + std::string(tmxFilePath) + "\n\nCheck the path.";
        MessageBoxA(NULL, msg.c_str(), "TileMap Error", MB_OK | MB_ICONERROR);
        return;
    }

    std::stringstream fileBuffer;
    fileBuffer << file.rdbuf();
    std::string xml = fileBuffer.str();
    file.close();

    // Map dimensions
    {
        std::regex mapRegex("<map[^>]*\\bwidth=\"(\\d+)\"[^>]*\\bheight=\"(\\d+)\"[^>]*\\btilewidth=\"(\\d+)\"[^>]*\\btileheight=\"(\\d+)\"");
        std::smatch match;
        if (std::regex_search(xml, match, mapRegex)) {
            mapWidthTiles = std::stoi(match[1]);
            mapHeightTiles = std::stoi(match[2]);
            tileWidth = std::stoi(match[3]);
            tileHeight = std::stoi(match[4]);
        }
    }

    // Tilesets
    {
        std::regex tilesetRegex("<tileset\\s+firstgid=\"(\\d+)\"[\\s\\S]*?tilewidth=\"(\\d+)\"[\\s\\S]*?tileheight=\"(\\d+)\"[\\s\\S]*?columns=\"(\\d+)\"[\\s\\S]*?<image\\s+source=\"([^\"]+)\"[^>]*width=\"(\\d+)\"[^>]*height=\"(\\d+)\"[^>]*/>[\\s\\S]*?</tileset>");
        auto begin = std::sregex_iterator(xml.begin(), xml.end(), tilesetRegex);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            std::smatch match = *it;
            TilesetInfo info;

            info.firstGid = std::stoi(match[1]);
            info.tileWidth = std::stoi(match[2]);
            info.tileHeight = std::stoi(match[3]);
            info.columns = std::stoi(match[4]);
            info.imageFile = std::string(assetFolder) + match[5].str();
            info.imageWidth = std::stoi(match[6]);
            info.imageHeight = std::stoi(match[7]);
            info.texture = NULL;

            HRESULT hr = D3DXCreateTextureFromFileEx(d3dDevice, info.imageFile.c_str(), info.imageWidth, info.imageHeight, 1, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &info.texture);

            if (FAILED(hr) || info.texture == NULL) {
                char buf[16];
                sprintf_s(buf, "%08X", (unsigned int)hr);
                std::string msg = "TileMap failed to load tileset image:\n" + info.imageFile + "\n\nHRESULT: 0x" + std::string(buf);
                MessageBoxA(NULL, msg.c_str(), "TileMap Error", MB_OK | MB_ICONERROR);
            }
            tilesets.push_back(info);
        }
    }

    // Parse map layers
    {
        std::regex layerRegex("<layer id=\"\\d+\" name=\"([^\"]+)\" width=\"\\d+\" height=\"\\d+\">\\s*<data encoding=\"csv\">\\s*([\\s\\S]*?)\\s*</data>\\s*</layer>");
        auto begin = std::sregex_iterator(xml.begin(), xml.end(), layerRegex);
        auto end = std::sregex_iterator();

        for (auto it = begin; it != end; ++it) {
            std::smatch match = *it;
            MapLayer layer;
            layer.name = match[1].str();
            std::string csv = match[2].str();
            std::stringstream ss(csv);
            std::string token;

            layer.tileIds.reserve((size_t)mapWidthTiles * (size_t)mapHeightTiles);

            while (std::getline(ss, token, ',')) {
                size_t start = token.find_first_not_of(" \t\r\n");
                if (start == std::string::npos) continue;

                size_t stop = token.find_last_not_of(" \t\r\n");
                token = token.substr(start, stop - start + 1);
                if (token.empty()) continue;

                try { layer.tileIds.push_back((int)std::stoul(token)); }
                catch (...) { layer.tileIds.push_back(0); }
            }
            layers.push_back(layer);
        }
    }

    // No solid layers until SetSolidLayers() is called - nothing blocks
    // movement by default.
    solidGrid.assign((size_t)mapWidthTiles * (size_t)mapHeightTiles, false);
}

TileMap::~TileMap() {
    for (size_t i = 0; i < tilesets.size(); ++i) {
        if (tilesets[i].texture != NULL) {
            tilesets[i].texture->Release();
            tilesets[i].texture = NULL;
        }
    }
}

void TileMap::BuildSolidGrid() {
    size_t cellCount = (size_t)mapWidthTiles * (size_t)mapHeightTiles;
    solidGrid.assign(cellCount, false);

    for (size_t layerIndex = 0; layerIndex < layers.size(); ++layerIndex) {
        const MapLayer& layer = layers[layerIndex];

        bool isSolidLayer = false;
        for (size_t i = 0; i < solidLayerNames.size(); ++i) {
            if (solidLayerNames[i] == layer.name) {
                isSolidLayer = true;
                break;
            }
        }
        if (!isSolidLayer) continue;

        for (size_t index = 0; index < layer.tileIds.size() && index < solidGrid.size(); ++index) {
            if (layer.tileIds[index] != 0) {
                solidGrid[index] = true;
            }
        }
    }

    // Inverse pass: anywhere NONE of the walkable layers has a tile becomes
    // solid too (e.g. a lake that's only distinguishable by land tiles not
    // being drawn there). Skipped entirely when no walkable layers are set.
    if (!walkableLayerNames.empty()) {
        std::vector<bool> hasLand(cellCount, false);

        for (size_t layerIndex = 0; layerIndex < layers.size(); ++layerIndex) {
            const MapLayer& layer = layers[layerIndex];

            bool isWalkableLayer = false;
            for (size_t i = 0; i < walkableLayerNames.size(); ++i) {
                if (walkableLayerNames[i] == layer.name) {
                    isWalkableLayer = true;
                    break;
                }
            }
            if (!isWalkableLayer) continue;

            for (size_t index = 0; index < layer.tileIds.size() && index < hasLand.size(); ++index) {
                if (layer.tileIds[index] != 0) {
                    hasLand[index] = true;
                }
            }
        }

        for (size_t index = 0; index < cellCount; ++index) {
            if (!hasLand[index]) solidGrid[index] = true;
        }
    }
}

void TileMap::SetSolidLayers(const std::vector<std::string>& layerNames) {
    solidLayerNames = layerNames;
    BuildSolidGrid();
}

void TileMap::SetWalkableLayers(const std::vector<std::string>& layerNames) {
    walkableLayerNames = layerNames;
    BuildSolidGrid();
}

bool TileMap::IsTileSolid(int tileX, int tileY) const {
    if (tileX < 0 || tileY < 0 || tileX >= mapWidthTiles || tileY >= mapHeightTiles) return false;

    size_t index = (size_t)tileY * (size_t)mapWidthTiles + (size_t)tileX;
    if (index >= solidGrid.size()) return false;

    return solidGrid[index];
}

int TileMap::StripFlipFlags(unsigned int gid) {
    const unsigned int FLIP_HORIZONTAL = 0x80000000;
    const unsigned int FLIP_VERTICAL = 0x40000000;
    const unsigned int FLIP_DIAGONAL = 0x20000000;
    return (int)(gid & ~(FLIP_HORIZONTAL | FLIP_VERTICAL | FLIP_DIAGONAL));
}

int TileMap::FindTilesetIndex(int gid) const {
    if (gid <= 0) return -1;

    int bestIndex = -1;
    int bestFirstGid = -1;

    for (size_t i = 0; i < tilesets.size(); ++i) {
        if (tilesets[i].firstGid <= gid && tilesets[i].firstGid > bestFirstGid) {
            bestFirstGid = tilesets[i].firstGid;
            bestIndex = (int)i;
        }
    }
    return bestIndex;
}

int TileMap::FindLayerIndex(const std::string& layerName) const {
    for (size_t i = 0; i < layers.size(); ++i) {
        if (layers[i].name == layerName) return (int)i;
    }
    return -1;
}

void TileMap::DrawLayers(LPD3DXSPRITE sharedBrush, const std::vector<size_t>& layerIndices) {
    if (sharedBrush == NULL) return;

    if (device != NULL) {
        device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
        device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
        device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
    }

    D3DXMATRIX identity;
    D3DXMatrixIdentity(&identity);
    sharedBrush->SetTransform(&identity);

    for (size_t i = 0; i < layerIndices.size(); ++i) {
        size_t L = layerIndices[i];
        if (L >= layers.size()) continue;

        const MapLayer& layer = layers[L];

        for (int y = 0; y < mapHeightTiles; ++y) {
            for (int x = 0; x < mapWidthTiles; ++x) {
                int index = y * mapWidthTiles + x;
                if (index < 0 || index >= (int)layer.tileIds.size()) continue;

                unsigned int rawGid = (unsigned int)layer.tileIds[index];
                if (rawGid == 0) continue;

                int gid = StripFlipFlags(rawGid);
                int tilesetIndex = FindTilesetIndex(gid);
                if (tilesetIndex < 0) continue;

                const TilesetInfo& ts = tilesets[tilesetIndex];
                if (ts.texture == NULL) continue;

                int localId = gid - ts.firstGid;
                int col = localId % ts.columns;
                int row = localId / ts.columns;

                RECT srcRect;
                srcRect.left = col * ts.tileWidth;
                srcRect.top = row * ts.tileHeight;
                srcRect.right = srcRect.left + ts.tileWidth;
                srcRect.bottom = srcRect.top + ts.tileHeight;

                D3DXVECTOR3 worldPos((float)(x * tileWidth), (float)(y * tileHeight), 0.0f);
                sharedBrush->Draw(ts.texture, &srcRect, NULL, &worldPos, D3DCOLOR_XRGB(255, 255, 255));
            }
        }
    }
}

void TileMap::Draw(LPD3DXSPRITE sharedBrush) {
    if (sharedBrush == NULL) return;

    if (debugForceSingleTile) {
        const int debugGid = 738;
        int tilesetIndex = FindTilesetIndex(debugGid);

        if (tilesetIndex >= 0 && tilesets[tilesetIndex].texture != NULL) {
            if (device != NULL) {
                device->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
                device->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
                device->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);
            }
            D3DXMATRIX identity;
            D3DXMatrixIdentity(&identity);
            sharedBrush->SetTransform(&identity);

            const TilesetInfo& ts = tilesets[tilesetIndex];
            int localId = debugGid - ts.firstGid;
            int col = localId % ts.columns;
            int row = localId / ts.columns;

            RECT srcRect;
            srcRect.left = col * ts.tileWidth;
            srcRect.top = row * ts.tileHeight;
            srcRect.right = srcRect.left + ts.tileWidth;
            srcRect.bottom = srcRect.top + ts.tileHeight;

            for (int y = 0; y < mapHeightTiles; ++y) {
                for (int x = 0; x < mapWidthTiles; ++x) {
                    D3DXVECTOR3 worldPos((float)(x * tileWidth), (float)(y * tileHeight), 0.0f);
                    sharedBrush->Draw(ts.texture, &srcRect, NULL, &worldPos, D3DCOLOR_XRGB(255, 255, 255));
                }
            }
        }
        return;
    }

    std::vector<size_t> allLayers;
    allLayers.reserve(layers.size());
    for (size_t i = 0; i < layers.size(); ++i) allLayers.push_back(i);
    DrawLayers(sharedBrush, allLayers);
}

void TileMap::DrawExcludingLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& excludeNames) {
    std::vector<size_t> indices;
    for (size_t i = 0; i < layers.size(); ++i) {
        bool excluded = false;
        for (size_t n = 0; n < excludeNames.size(); ++n) {
            if (layers[i].name == excludeNames[n]) { excluded = true; break; }
        }
        if (!excluded) indices.push_back(i);
    }
    DrawLayers(sharedBrush, indices);
}

void TileMap::DrawOnlyLayers(LPD3DXSPRITE sharedBrush, const std::vector<std::string>& includeNames) {
    std::vector<size_t> indices;
    for (size_t i = 0; i < layers.size(); ++i) {
        for (size_t n = 0; n < includeNames.size(); ++n) {
            if (layers[i].name == includeNames[n]) { indices.push_back(i); break; }
        }
    }
    DrawLayers(sharedBrush, indices);
}

int TileMap::GetWidthPixels() const {
    return mapWidthTiles * tileWidth;
}

int TileMap::GetHeightPixels() const {
    return mapHeightTiles * tileHeight;
}

bool TileMap::HasLayer(const std::string& layerName) const {
    return FindLayerIndex(layerName) >= 0;
}

void TileMap::SetDebugForceSingleTile(bool enabled) {
    debugForceSingleTile = enabled;
}

void TileMap::PrintDiagnosticReport() const {
    std::string report = "TileMap parsed:\n\n";
    report += "Map: " + std::to_string(mapWidthTiles) + "x" + std::to_string(mapHeightTiles) + " tiles\n";
    report += "Tile size: " + std::to_string(tileWidth) + "x" + std::to_string(tileHeight) + "\n\n";
    report += "Tilesets (" + std::to_string(tilesets.size()) + "):\n";

    for (size_t i = 0; i < tilesets.size(); ++i) {
        report += "  firstGid=" + std::to_string(tilesets[i].firstGid) + " columns=" + std::to_string(tilesets[i].columns) + " image=" + tilesets[i].imageFile;
        if (tilesets[i].texture == NULL) {
            report += " [FAILED TO LOAD]";
        }
        report += "\n";
    }

    report += "\nLayers (" + std::to_string(layers.size()) + "):\n";
    for (size_t i = 0; i < layers.size(); ++i) {
        report += "  \"" + layers[i].name + "\" - " + std::to_string(layers[i].tileIds.size()) + " tiles\n";
    }

    int solidCount = 0;
    for (size_t i = 0; i < solidGrid.size(); ++i) {
        if (solidGrid[i]) ++solidCount;
    }
    report += "\nSolid layers: ";
    if (solidLayerNames.empty()) {
        report += "(none set)";
    }
    else {
        for (size_t i = 0; i < solidLayerNames.size(); ++i) {
            if (i > 0) report += ", ";
            report += solidLayerNames[i];
        }
    }
    report += "\nSolid tile cells: " + std::to_string(solidCount);

    MessageBoxA(NULL, report.c_str(), "TileMap Diagnostic", MB_OK | MB_ICONINFORMATION);
}