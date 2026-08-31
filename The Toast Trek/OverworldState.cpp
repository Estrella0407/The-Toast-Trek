#include "OverworldState.h"
#include "Enemy.h"
#include "Font.h"
#include "Physics.h"
#include "PochiBadge.h"
#include "Sprite.h"
#include "TileMap.h"
#include <dinput.h>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    bool IsKeyDown(BYTE* keys, int key) {
        return keys != NULL && (keys[key] & 0x80) != 0;
    }

    // Arrow keys and WASD both drive movement.
    struct MoveInput {
        bool left, right, up, down;
    };

    MoveInput ReadMoveInput(BYTE* keys) {
        MoveInput input;
        input.left = IsKeyDown(keys, DIK_LEFT) || IsKeyDown(keys, DIK_A);
        input.right = IsKeyDown(keys, DIK_RIGHT) || IsKeyDown(keys, DIK_D);
        input.up = IsKeyDown(keys, DIK_UP) || IsKeyDown(keys, DIK_W);
        input.down = IsKeyDown(keys, DIK_DOWN) || IsKeyDown(keys, DIK_S);
        return input;
    }

    TileMap* ResolveMap(GameContext& context, MapId id) {
        switch (id) {
        case MapId::Maze: return context.mazeMap;
        case MapId::RuinsExterior: return context.ruinsExteriorMap;
        case MapId::RuinsInterior: return context.ruinsInteriorMap;
        case MapId::Forest:
        default: return context.forestMap;
        }
    }

    // Pochi's sprite canvas (scaled ~100x60) is much bigger than her actual
    // standing pose, so colliding tiles against the full canvas made gaps
    // that look easily walkable (a maze corridor, two rocks either side of
    // a path) feel blocked. Collide a smaller box at her feet instead.
    constexpr float kPochiFootWidthRatio = 0.5f;
    constexpr float kPochiFootHeightRatio = 0.6f;

    constexpr float kInteractRadius = 90.0f;

    bool IsNear(const D3DXVECTOR2& a, const D3DXVECTOR2& b, float radius) {
        float dx = a.x - b.x;
        float dy = a.y - b.y;
        return (dx * dx + dy * dy) <= (radius * radius);
    }

    // Pochi's foot box overlapping an item's sprite bounds - the same
    // footprint used for tile collision, so "standing on it" lines up with
    // where she visually is.
    bool TouchingItem(Sprite* pochi, Sprite* item) {
        if (pochi == NULL || item == NULL) return false;
        return Physics::CheckAABBCollision(
            Physics::GetFootBounds(pochi, kPochiFootWidthRatio, kPochiFootHeightRatio),
            Physics::GetBounds(item));
    }

    const char* ItemLabel(ItemType type) {
        switch (type) {
        case ItemType::HealthPotion: return "Health Potion  -  restores health";
        case ItemType::Bone:         return "Bone  -  restores armor";
        case ItemType::Toast:        return "Toast";
        }
        return "Item";
    }

    std::unique_ptr<GameState> CreateMazeState();
    std::unique_ptr<GameState> CreateRuinsExteriorState();
    std::unique_ptr<GameState> CreateRuinsInteriorState();

    class OverworldState : public GameState {
    private:
        OverworldConfig config;
        TileMap* map;

        bool interactWasDown;
        std::vector<Enemy*> bossEnemies;
        std::vector<bool> bossCleared;
        Font* interactPrompt;

        std::vector<Sprite*> itemSprites;
        std::vector<bool> itemCollected;
        Sprite* exclaim;   // "!" bubble shown above an un-collected item Pochi is standing on
        Font* itemPrompt;

        PochiBadge* hud;   // top-left HP / DEF / ATK readout

    public:
        explicit OverworldState(OverworldConfig cfg)
            : config(std::move(cfg)), map(NULL), interactWasDown(false), interactPrompt(NULL),
              exclaim(NULL), itemPrompt(NULL), hud(NULL) {}

        ~OverworldState() {
            for (Enemy* enemy : bossEnemies) delete enemy;
            for (Sprite* item : itemSprites) delete item;
            delete interactPrompt;
            delete exclaim;
            delete itemPrompt;
            delete hud;
        }

        void Initialize(GameContext& context) override {
            map = ResolveMap(context, config.mapId);

            // Reset Pochi's pose too - MainMenu (and a previous map) can
            // leave her mid-animation-frame from wherever she last was.
            if (context.pochi != NULL && config.computeSpawnPosition) {
                D3DXVECTOR2 spawn = config.computeSpawnPosition(context.pochi->GetPosition());
                context.pochi->SetPosition(spawn.x, spawn.y);
                context.pochi->CropToFrame(0);
            }

            bossEnemies.clear();
            bossCleared.assign(config.bosses.size(), false);
            for (const BossSpawn& spawn : config.bosses) {
                bossEnemies.push_back(CreateBossEnemy(context.device, spawn.id, spawn.x, spawn.y));
            }

            if (!config.bosses.empty()) {
                interactPrompt = new Font(context.device, 0.0f, 20.0f, 1280, 40, 20, "Arial");
            }

            for (Sprite* item : itemSprites) delete item;
            itemSprites.clear();
            itemCollected.assign(config.items.size(), false);
            for (const ItemSpawn& spawn : config.items) {
                Sprite* item = new Sprite(context.device, spawn.texture.c_str(),
                    spawn.texWidth, spawn.texHeight, 1, 1, 1, spawn.x, spawn.y);
                item->SetScale(spawn.scale);
                itemSprites.push_back(item);
            }

            if (!config.items.empty()) {
                // exclamationPoint.png is a 1254x1254 source - scale it right down.
                exclaim = new Sprite(context.device, "Assets/Item/exclamationPoint.png",
                    1254, 1254, 1, 1, 1, 0.0f, 0.0f);
                exclaim->SetScale(0.03f);
                itemPrompt = new Font(context.device, 0.0f, 0.0f, 360, 30, 18, "Arial");
            }

            delete hud;
            hud = new PochiBadge(context.device);
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            if (context.pochi == NULL) return;
            if (!JustPressed(context.keys, DIK_F, interactWasDown)) return;

            D3DXVECTOR2 pochiPos = context.pochi->GetPosition();

            // Items first: standing on one and pressing F picks it up (and
            // consumes this press, so it never also starts a fight).
            for (size_t i = 0; i < itemSprites.size(); ++i) {
                if (itemCollected[i]) continue;
                if (TouchingItem(context.pochi, itemSprites[i])) {
                    itemCollected[i] = true;
                    if (context.inventory != NULL) context.inventory->Add(config.items[i].type);
                    return;
                }
            }

            for (size_t i = 0; i < bossEnemies.size(); ++i) {
                if (bossCleared[i]) continue;
                if (IsNear(pochiPos, bossEnemies[i]->GetSprite()->GetPosition(), kInteractRadius)) {
                    manager.Push(CreateBattleState(config.bosses[i].id));
                    return;
                }
            }
        }

        void Update(GameContext& context, GameStateManager& manager) override {
            // Pick up the result of whichever battle we just returned from -
            // BattleState sets this right before popping itself.
            if (context.lastBattleOutcome == BattleOutcome::Victory) {
                for (size_t i = 0; i < config.bosses.size(); ++i) {
                    if (config.bosses[i].id == context.lastBattleBoss) bossCleared[i] = true;
                }
            }
            context.lastBattleOutcome = BattleOutcome::None;

            if (context.pochi == NULL) return;

            MoveInput input = ReadMoveInput(context.keys);
            bool isMoving = false;
            if (input.left) {
                context.pochi->Move((float)-context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(LEFT);
                isMoving = true;
            }
            else if (input.right) {
                context.pochi->Move((float)context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(RIGHT);
                isMoving = true;
            }
            if (input.up) {
                context.pochi->Move(0.0f, (float)-context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            else if (input.down) {
                context.pochi->Move(0.0f, (float)context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            if (!isMoving) context.pochi->SetIdlePose();

            if (map == NULL) return;

            Physics::ClampToBounds(context.pochi, 0.0f, 0.0f,
                (float)map->GetWidthPixels(), (float)map->GetHeightPixels());
            Physics::ResolveCollisionShapes(context.pochi, map, kPochiFootWidthRatio, kPochiFootHeightRatio);

            if (config.onReachRightEdge) {
                AABB pochiBounds = Physics::GetBounds(context.pochi);
                if (pochiBounds.right >= (float)map->GetWidthPixels() - 5.0f) {
                    std::unique_ptr<GameState> next = config.onReachRightEdge();
                    if (next != NULL) manager.Push(std::move(next));
                }
            }

            if (config.onEnterDoorway &&
                IsNear(context.pochi->GetPosition(), config.doorwayPosition, config.doorwayRadius)) {
                std::unique_ptr<GameState> next = config.onEnterDoorway();
                if (next != NULL) manager.Push(std::move(next));
            }
        }

        void Render(GameContext& context) override {
            if (map == NULL) return;

            bool hasForeground = !config.foregroundLayers.empty();
            if (hasForeground) map->DrawExcludingLayers(context.spriteBrush, config.foregroundLayers);
            else map->Draw(context.spriteBrush);

            // Items sit on the ground - drawn before Pochi/bosses so they
            // walk over them.
            for (size_t i = 0; i < itemSprites.size(); ++i) {
                if (!itemCollected[i]) itemSprites[i]->Draw(context.spriteBrush);
            }

            for (size_t i = 0; i < bossEnemies.size(); ++i) {
                if (!bossCleared[i]) bossEnemies[i]->Render(context.spriteBrush);
            }

            if (context.pochi != NULL) context.pochi->Draw(context.spriteBrush);

            // Layers like the forest's leaf canopy draw last, in front of
            // both Pochi and any bosses.
            if (hasForeground) map->DrawOnlyLayers(context.spriteBrush, config.foregroundLayers);

            // "!" bubble + label for an un-collected item Pochi is standing on.
            if (context.pochi != NULL && itemPrompt != NULL) {
                for (size_t i = 0; i < itemSprites.size(); ++i) {
                    if (itemCollected[i]) continue;
                    if (!TouchingItem(context.pochi, itemSprites[i])) continue;

                    D3DXVECTOR2 itemPos = itemSprites[i]->GetPosition();
                    if (exclaim != NULL) {
                        exclaim->SetPosition(itemPos.x, itemPos.y - 44.0f);
                        exclaim->Draw(context.spriteBrush);
                    }
                    itemPrompt->Draw(ItemLabel(config.items[i].type),
                        itemPos.x - 110.0f, itemPos.y - 26.0f, D3DCOLOR_XRGB(255, 255, 255));
                    itemPrompt->Draw("Press F to pick up",
                        itemPos.x - 110.0f, itemPos.y - 4.0f, D3DCOLOR_XRGB(255, 255, 255));
                    break;
                }
            }

            if (context.pochi != NULL && interactPrompt != NULL) {
                D3DXVECTOR2 pochiPos = context.pochi->GetPosition();
                for (size_t i = 0; i < bossEnemies.size(); ++i) {
                    if (!bossCleared[i] &&
                        IsNear(pochiPos, bossEnemies[i]->GetSprite()->GetPosition(), kInteractRadius)) {
                        interactPrompt->Draw("PRESS F TO FIGHT", D3DCOLOR_XRGB(255, 255, 255));
                        break;
                    }
                }
            }

            if (hud != NULL && context.playerStats != NULL) {
                hud->Draw(context.spriteBrush, *context.playerStats);
            }
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

    // Boss coordinates place each boss (a ~100px-ish canvas) centered in
    // the room it was circled in on a render of the maze: SkullBones in
    // the top-left room, Goblin in the open pocket mid-map. Nudge these if
    // they don't line up with the final art.
    OverworldConfig MakeMazeConfig() {
        OverworldConfig config;
        config.mapId = MapId::Maze;
        config.bosses = {
            { BossId::SkullBones, 140.0f, 35.0f },
            { BossId::Goblin, 690.0f, 317.0f }
        };
        // The forest and maze art line up at the shared edge, so Pochi
        // keeps whatever Y she had when she walked off the forest's right
        // edge - only X resets, to the maze's left edge - for a seamless
        // crossing instead of snapping to one spot.
        config.computeSpawnPosition = [](const D3DXVECTOR2& current) {
            return D3DXVECTOR2(10.0f, current.y);
        };
        config.onReachRightEdge = [] { return CreateRuinsExteriorState(); };
        return config;
    }

    std::unique_ptr<GameState> CreateMazeState() {
        return CreateOverworldState(MakeMazeConfig());
    }

    OverworldConfig MakeForestConfig() {
        OverworldConfig config;
        config.mapId = MapId::Forest;
        config.foregroundLayers = { "Tree_Leaf" }; // only the leaf canopy draws in front of Pochi
        config.computeSpawnPosition = [](const D3DXVECTOR2&) {
            return D3DXVECTOR2(100.0f, 380.0f);
        };
        // A couple of pickups along the walk to the maze, so the tutorial's
        // "stand on it and press F" has something to land on. Nudge these if
        // they end up inside a tree once the final art is in.
        config.items = {
            { ItemType::HealthPotion, "Assets/Item/heathPotion.png", 18, 20, 380.0f, 392.0f, 2.0f },
            { ItemType::Bone,         "Assets/Item/bone.png",        32, 32, 680.0f, 360.0f, 1.5f },
        };
        config.onReachRightEdge = [] { return CreateMazeState(); };
        return config;
    }

    // The maze (boxy garden hedges) and the ruins exterior (an organic
    // lake/forest scene) aren't drawn to line up at any edge the way the
    // forest and maze are, and the ruins' left edge is partly water - so
    // this spawns Pochi at a fixed, known-dry spot on the entrance path
    // instead of carrying her Y across like the earlier crossing does.
    OverworldConfig MakeRuinsExteriorConfig() {
        OverworldConfig config;
        config.mapId = MapId::RuinsExterior;
        config.foregroundLayers = { "Tree_leaf" }; // matches this map's actual layer name (lowercase 'leaf')
        config.computeSpawnPosition = [](const D3DXVECTOR2&) {
            return D3DXVECTOR2(320.0f, 528.0f); // verified clear of Tree/House/Bricks/Statues/Columns and the lake
        };
        // The dark doorway opening between the two gargoyle statues at the
        // temple's entrance - walking up to it leads inside.
        config.doorwayPosition = D3DXVECTOR2(780.0f, 190.0f);
        config.doorwayRadius = 60.0f;
        config.onEnterDoorway = [] { return CreateRuinsInteriorState(); };
        return config;
    }

    std::unique_ptr<GameState> CreateRuinsExteriorState() {
        return CreateOverworldState(MakeRuinsExteriorConfig());
    }

    // Maki (final boss) stands at the top of the main hall, in front of
    // the large gargoyle statue - a dead end for now, since there's
    // nowhere further to go once she's here.
    OverworldConfig MakeRuinsInteriorConfig() {
        OverworldConfig config;
        config.mapId = MapId::RuinsInterior;
        config.bosses = {
            { BossId::Maki, 610.0f, 270.0f }
        };
        config.computeSpawnPosition = [](const D3DXVECTOR2&) {
            // The entrance corridor is only 4 tiles wide (cols 38-41) and its
            // bottom row (y>=704) is the map's outer boundary wall - ClampToBounds
            // would otherwise press Pochi's foot-box straight into that wall, so
            // this sits a bit further up the corridor instead of right at the seam.
            return D3DXVECTOR2(590.0f, 620.0f);
        };
        return config;
    }

    std::unique_ptr<GameState> CreateRuinsInteriorState() {
        return CreateOverworldState(MakeRuinsInteriorConfig());
    }
}

std::unique_ptr<GameState> CreateOverworldState(OverworldConfig config) {
    return std::make_unique<OverworldState>(std::move(config));
}

std::unique_ptr<GameState> CreateForestState() {
    return CreateOverworldState(MakeForestConfig());
}
