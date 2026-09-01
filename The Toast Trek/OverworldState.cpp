#include "OverworldState.h"
#include "Cheats.h"
#include "Enemy.h"
#include "Font.h"
#include "Physics.h"
#include "Pochi.h"
#include "PochiBadge.h"
#include "Sprite.h"
#include "TileMap.h"
#include "UiFill.h"
#include "UnifiedMenu.h"
#include <algorithm>
#include <cstring>
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
        case MapId::Tarumt: return context.tarumtMap;
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


    std::unique_ptr<GameState> CreateMazeState();
    std::unique_ptr<GameState> CreateRuinsExteriorState();
    std::unique_ptr<GameState> CreateTarumtState();
    std::unique_ptr<GameState> CreateRuinsInteriorState();

    class OverworldState : public GameState {
    private:
        OverworldConfig config;
        TileMap* map;

        bool interactWasDown;
        bool menuWasDown;
        bool cheatClearWasDown;   // K - clear nearest boss
        bool cheatWarpWasDown;    // L - jump to this map's exit
        bool allClearedFired;
        bool exitsArmed;          // false until Pochi has stood clear of every exit trigger
        std::vector<Enemy*> bossEnemies;
        std::vector<bool> bossCleared;
        Font* interactPrompt;

        std::vector<Sprite*> itemSprites;
        std::vector<bool> itemCollected;
        IDirect3DTexture9* exclaimTex;   // "!" bubble, floated over Pochi's head

        PochiBadge* hud;   // top-left HP / DEF / ATK readout

        Font* levelUpFont;      // floating text over Pochi's head (win / stat-boost)
        int levelUpFrames;      // frames left to show it (~60/sec)
        const char* floatText;  // what that floating text says

        // Set on the Tarumt map: Pochi's stats are force-boosted to the
        // Mr Andrew "special level" (HP 99 / armor 50 / ATK 99) on entry and
        // restored to her real level on every way back out.
        Pochi* boostedStats;

        void LeaveBoostedMap(GameContext& context) {
            if (boostedStats != NULL && context.playerStats == boostedStats &&
                boostedStats->IsSpecialMode()) {
                boostedStats->SetSpecialMode(false);
            }
        }

        // Stash a forced spawn for the destination map (so Pochi lands on
        // the connecting seam, not the destination's default spawn). A y of
        // kCarryY keeps her current y; kNoSpawn means "no override".
        void StashSpawn(GameContext& context, const D3DXVECTOR2& s) {
            if (s.x <= OverworldConfig::kNoSpawn) return;
            D3DXVECTOR2 out = s;
            if (s.y == OverworldConfig::kCarryY && context.pochi != NULL)
                out.y = context.pochi->GetPosition().y;
            context.pendingSpawn = out;
            context.hasPendingSpawn = true;
        }

        Sprite* gateSprite;            // exit gate art, if config.gateTexture is set
        IDirect3DTexture9* whiteTex;   // 1x1 white - FillRect plates + the drawn-gate fallback

        bool HasGate() const { return config.gateWidth > 0.0f && config.gateHeight > 0.0f; }

        // The locked exit gate. Uses the supplied art if there is any,
        // otherwise draws a simple iron portcullis (frame + vertical bars)
        // filling the gate rect with plain sprite-brush quads.
        void DrawGate(GameContext& context) {
            if (gateSprite != NULL) { gateSprite->Draw(context.spriteBrush); return; }
            if (whiteTex == NULL) return;

            LPD3DXSPRITE b = context.spriteBrush;
            const float x = config.gateX, y = config.gateY;
            const float w = config.gateWidth, h = config.gateHeight;
            const D3DCOLOR iron     = D3DCOLOR_ARGB(255, 84, 88, 100);
            const D3DCOLOR ironDark = D3DCOLOR_ARGB(255, 44, 46, 56);

            ui::FillRect(b, whiteTex, x + 5.0f, y + 6.0f, w, h, D3DCOLOR_ARGB(90, 0, 0, 0)); // drop shadow
            ui::FillRect(b, whiteTex, x, y, w, 12.0f, iron);              // top rail
            ui::FillRect(b, whiteTex, x, y + h - 12.0f, w, 12.0f, iron);  // bottom rail

            const int bars = 5;
            for (int i = 0; i < bars; ++i) {
                const float bx = x + (w - 6.0f) * (i / (float)(bars - 1));
                ui::FillRect(b, whiteTex, bx, y, 6.0f, h, (i % 2) ? ironDark : iron);
            }
        }

    public:
        explicit OverworldState(OverworldConfig cfg)
            : config(std::move(cfg)), map(NULL), interactWasDown(false), menuWasDown(false),
              cheatClearWasDown(false), cheatWarpWasDown(false),
              allClearedFired(false), exitsArmed(false), interactPrompt(NULL),
              exclaimTex(NULL), hud(NULL),
              levelUpFont(NULL), levelUpFrames(0), floatText("Leveled Up!"),
              boostedStats(NULL),
              gateSprite(NULL), whiteTex(NULL) {}

        ~OverworldState() {
            // Safety net if this state is torn down (ClearAndPush on defeat)
            // without going through a normal exit: never leave Pochi boosted.
            if (boostedStats != NULL && boostedStats->IsSpecialMode()) {
                boostedStats->SetSpecialMode(false);
            }
            for (Enemy* enemy : bossEnemies) delete enemy;
            for (Sprite* item : itemSprites) delete item;
            delete interactPrompt;
            if (exclaimTex != NULL) exclaimTex->Release();
            delete hud;
            delete levelUpFont;
            delete gateSprite;
            if (whiteTex != NULL) whiteTex->Release();
        }

        void Initialize(GameContext& context) override {
            map = ResolveMap(context, config.mapId);

            if (whiteTex == NULL) whiteTex = ui::MakeWhiteTexture(context.device);

            // Reset Pochi's pose too - MainMenu (and a previous map) can
            // leave her mid-animation-frame from wherever she last was.
            // A pending spawn from the map she just left (placing her on the
            // connecting seam) wins over this map's own default spawn.
            if (context.pochi != NULL) {
                if (context.hasPendingSpawn) {
                    context.pochi->SetPosition(context.pendingSpawn.x, context.pendingSpawn.y);
                    context.pochi->CropToFrame(0);
                    context.hasPendingSpawn = false;
                }
                else if (config.computeSpawnPosition) {
                    D3DXVECTOR2 spawn = config.computeSpawnPosition(context.pochi->GetPosition());
                    context.pochi->SetPosition(spawn.x, spawn.y);
                    context.pochi->CropToFrame(0);
                }
            }

            // Don't let any exit fire until Pochi has first stepped clear of
            // every trigger - she may spawn right on a seam or in a doorway.
            exitsArmed = false;

            // If this map's bosses were already beaten earlier in the run,
            // it comes back cleared - no enemies to draw or fight, gate open,
            // and the once-only onAllCleared hook already spent.
            const bool preCleared = context.clearedMaps.count(config.mapId) != 0;

            bossEnemies.clear();
            bossCleared.assign(config.bosses.size(), preCleared);
            for (const BossSpawn& spawn : config.bosses) {
                bossEnemies.push_back(CreateBossEnemy(context.device, spawn.id, spawn.x, spawn.y));
            }
            allClearedFired = preCleared;

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

            // The "!" bubble floats over Pochi whenever she's next to an item
            // or a fightable enemy, so make it on any map that has either.
            if (exclaimTex != NULL) { exclaimTex->Release(); exclaimTex = NULL; }
            if (!config.items.empty() || !config.bosses.empty()) {
                exclaimTex = ui::LoadTexture(context.device,
                    "Assets/Item/exclamationPoint.png", 1254, 1254);
            }

            delete hud;
            hud = new PochiBadge(context.device);

            delete levelUpFont;
            // Wide rect: Font::Draw(x,y,...) only moves rect.left/top, so a
            // narrow rect inverts once Pochi walks past x = width.
            levelUpFont = new Font(context.device, 0.0f, 0.0f, 1400, 40, 15, "Arial");
            levelUpFrames = 0;
            floatText = "Leveled Up!";

            // Tarumt (Mr Andrew's arena): slam Pochi's stats up to the
            // "special level" so the 1000-HP secret boss is beatable, and
            // announce it over her head instead of the usual level-up text.
            // SetSpecialMode is cleared first so a stale flag (e.g. after a
            // Game Over -> retry) can't make the boost a no-op.
            boostedStats = NULL;
            if (config.mapId == MapId::Tarumt && !preCleared && map != NULL && context.playerStats != NULL) {
                boostedStats = context.playerStats;
                boostedStats->SetSpecialMode(false);
                boostedStats->SetSpecialMode(true);
                floatText = "Stats boosted to max!";
                levelUpFrames = 180;
            }

            // Exit gate: a supplied PNG if there is one, otherwise the drawn
            // iron portcullis (DrawGate stretches whiteTex into shape).
            delete gateSprite;
            gateSprite = NULL;
            if (HasGate() && !config.gateTexture.empty()) {
                gateSprite = new Sprite(context.device, config.gateTexture.c_str(),
                    config.gateTexWidth, config.gateTexHeight, 1, 1, 1,
                    config.gateX, config.gateY);
            }
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            if (context.pochi == NULL) return;

            // E opens the tab menu (Inventory / Status / Settings). Pass
            // `this` so the menu can redraw the frozen overworld behind it.
            if (JustPressed(context.keys, DIK_E, menuWasDown)) {
                manager.Push(CreateUnifiedMenuState(this));
                return;
            }

            // --- developer cheats (F5 toggles the whole switch) -----------
            if (Cheats::enabled) {
                // K: clear the nearest un-cleared boss without fighting.
                // (Letter keys, not F8/F9 - laptop OEM Fn rows swallow those
                // for airplane mode / brightness before the game sees them.)
                if (JustPressed(context.keys, DIK_K, cheatClearWasDown)) {
                    D3DXVECTOR2 pochiPos = context.pochi->GetPosition();
                    int best = -1;
                    float bestDistSq = 0.0f;
                    for (size_t i = 0; i < bossEnemies.size(); ++i) {
                        if (bossCleared[i]) continue;
                        D3DXVECTOR2 bp = bossEnemies[i]->GetSprite()->GetPosition();
                        float dsq = (pochiPos.x - bp.x) * (pochiPos.x - bp.x) +
                                    (pochiPos.y - bp.y) * (pochiPos.y - bp.y);
                        if (best < 0 || dsq < bestDistSq) { best = (int)i; bestDistSq = dsq; }
                    }
                    if (best >= 0) bossCleared[best] = true;
                    if (!config.bosses.empty() && AllBossesCleared()) {
                        context.clearedMaps.insert(config.mapId);
                    }
                    return;
                }
                // L: jump straight to this map's exit.
                if (JustPressed(context.keys, DIK_L, cheatWarpWasDown)) {
                    LeaveBoostedMap(context);
                    std::unique_ptr<GameState> next;
                    if (config.onReachRightEdge) { next = config.onReachRightEdge(); StashSpawn(context, config.rightEdgeSpawn); }
                    else if (config.onEnterDoorway) { next = config.onEnterDoorway(); StashSpawn(context, config.doorwaySpawn); }
                    if (next != NULL) manager.Push(std::move(next));
                    return;
                }
            }

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
                // Sequence lock: can't fight this one until the earlier ones
                // fall (god mode ignores the order).
                if (!Cheats::enabled && config.bossesInOrder && i > 0 && !bossCleared[i - 1]) continue;
                if (IsNear(pochiPos, bossEnemies[i]->GetSprite()->GetPosition(), kInteractRadius)) {
                    manager.Push(CreateBattleState(config.bosses[i].id));
                    return;
                }
            }
        }

        // True once every boss on this map is down (also true if there are none).
        bool AllBossesCleared() const {
            for (bool cleared : bossCleared) if (!cleared) return false;
            return true;
        }

        // The map's exit is sealed while it still has enemies to clear.
        // God mode (cheats) opens every seal - no gate, no "defeat them
        // first", edge/doorway exits fire straight away.
        bool ExitLocked() const {
            if (Cheats::enabled) return false;
            return config.requireBossesCleared && !bossCleared.empty() && !AllBossesCleared();
        }

        void Update(GameContext& context, GameStateManager& manager) override {
            // Map failed to resolve (e.g. Tarumt.tmx not added yet) - bounce
            // straight back instead of stranding the player on a black screen.
            if (map == NULL) { manager.Pop(); return; }

            // Pick up the result of whichever battle we just returned from -
            // BattleState sets this right before popping itself.
            if (context.lastBattleOutcome == BattleOutcome::Victory) {
                for (size_t i = 0; i < config.bosses.size(); ++i) {
                    if (config.bosses[i].id == context.lastBattleBoss) bossCleared[i] = true;
                }
                // Remember a fully-cleared map for the rest of the run, so
                // walking back into it later doesn't respawn its enemies.
                if (!config.bosses.empty() && AllBossesCleared()) {
                    context.clearedMaps.insert(config.mapId);
                }
                // Beating an enemy levels Pochi up (which also fully heals
                // her) and floats "Leveled Up!" over her head for a moment.
                // Mr Andrew (Tarumt) is outside the 3-level progression - his
                // fight runs on the temporary special-level boost instead.
                if (context.playerStats != NULL && config.mapId != MapId::Tarumt) {
                    const int lv = context.playerStats->GetLevel();
                    if (lv < 3) {
                        context.playerStats->SetLevel(lv + 1);
                        levelUpFrames = 150;
                        floatText = "Leveled Up!";
                    }
                }
            }
            context.lastBattleOutcome = BattleOutcome::None;

            if (levelUpFrames > 0) --levelUpFrames;

            // Every boss on this map down -> fire the once-only hook (the
            // final map uses it to roll the ending).
            if (!allClearedFired && config.onAllCleared && !bossCleared.empty()) {
                bool all = true;
                for (bool c : bossCleared) if (!c) { all = false; break; }
                if (all) {
                    allClearedFired = true;
                    LeaveBoostedMap(context);
                    // Land Pochi on the same seam a normal right-edge exit
                    // would use (e.g. Tarumt -> forest's top-left path).
                    StashSpawn(context, config.rightEdgeSpawn);
                    std::unique_ptr<GameState> next = config.onAllCleared();
                    if (next != NULL) { manager.Push(std::move(next)); return; }
                }
            }

            if (context.pochi == NULL) return;

            MoveInput input = ReadMoveInput(context.keys);
            // Cheat mode: walk 3x faster to cross maps quickly.
            const int moveSpeed = Cheats::enabled ? context.moveSpeed * 3 : context.moveSpeed;
            bool isMoving = false;
            if (input.left) {
                context.pochi->Move((float)-moveSpeed, 0.0f);
                context.pochi->AnimateWalk(LEFT);
                isMoving = true;
            }
            else if (input.right) {
                context.pochi->Move((float)moveSpeed, 0.0f);
                context.pochi->AnimateWalk(RIGHT);
                isMoving = true;
            }
            if (input.up) {
                context.pochi->Move(0.0f, (float)-moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            else if (input.down) {
                context.pochi->Move(0.0f, (float)moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            if (!isMoving) context.pochi->SetIdlePose();

            if (map == NULL) return;

            Physics::ClampToBounds(context.pochi, 0.0f, 0.0f,
                (float)map->GetWidthPixels(), (float)map->GetHeightPixels());

            // Cheat mode: no-clip - skip solid-tile resolution and the fence so
            // a tester can walk straight through walls (the edge / doorway
            // triggers below still fire).
            if (!Cheats::enabled) {
                Physics::ResolveCollisionShapes(context.pochi, map, kPochiFootWidthRatio, kPochiFootHeightRatio);

                // Invisible top/bottom fence: keep Pochi's feet inside the maze
                // band so she can't walk around it along the map's open edges.
                if (config.fenceBottom > config.fenceTop) {
                    AABB feet = Physics::GetFootBounds(context.pochi, kPochiFootWidthRatio, kPochiFootHeightRatio);
                    D3DXVECTOR2 p = context.pochi->GetPosition();
                    if (feet.top < config.fenceTop)       p.y += config.fenceTop - feet.top;
                    if (feet.bottom > config.fenceBottom) p.y -= feet.bottom - config.fenceBottom;
                    context.pochi->SetPosition(p.x, p.y);
                }

                // Closed exit gate: a solid wall at gateX until the map is cleared.
                if (HasGate() && ExitLocked()) {
                    AABB pb = Physics::GetBounds(context.pochi);
                    if (pb.right > config.gateX) {
                        D3DXVECTOR2 p = context.pochi->GetPosition();
                        context.pochi->SetPosition(p.x - (pb.right - config.gateX), p.y);
                    }
                }
            }

            // --- map exits ---------------------------------------------------
            const AABB pb = Physics::GetBounds(context.pochi);
            const D3DXVECTOR2 pcentre = context.pochi->GetPosition();
            const bool atRight = config.onReachRightEdge &&
                pb.right >= (float)map->GetWidthPixels() - 5.0f;
            const bool atLeft = config.onReachLeftEdge && pb.left <= 8.0f;
            const bool atDoor = config.onEnterDoorway &&
                IsNear(pcentre, config.doorwayPosition, config.doorwayRadius);

            // Arm exits only once Pochi is clear of all of them - and never
            // transition on the same frame the latch arms.
            if (!exitsArmed) {
                if (!atRight && !atLeft && !atDoor) exitsArmed = true;
                return;
            }

            // Forward exits (right edge / doorway) stay sealed while this
            // map still has bosses to beat; the L dev-warp bypasses this.
            const bool exitLocked = ExitLocked();

            if (atRight && !exitLocked) {
                LeaveBoostedMap(context);
                StashSpawn(context, config.rightEdgeSpawn);
                std::unique_ptr<GameState> next = config.onReachRightEdge();
                if (next != NULL) manager.Push(std::move(next));
                return;
            }

            if (atDoor && !exitLocked) {
                LeaveBoostedMap(context);
                StashSpawn(context, config.doorwaySpawn);
                std::unique_ptr<GameState> next = config.onEnterDoorway();
                if (next != NULL) manager.Push(std::move(next));
                return;
            }

            // Backtracking out the left edge - allowed even while sealed.
            if (atLeft) {
                LeaveBoostedMap(context);
                StashSpawn(context, config.leftEdgeSpawn);
                std::unique_ptr<GameState> next = config.onReachLeftEdge();
                if (next != NULL) manager.Push(std::move(next));
                return;
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

            // The exit gate stands until the map is cleared; Pochi draws in
            // front of it.
            if (HasGate() && ExitLocked()) DrawGate(context);

            if (context.pochi != NULL) context.pochi->Draw(context.spriteBrush);

            // Pochi's 50x30 frame is drawn 2x about its centre, so her
            // visible pose is centred at (pos + (25, 15)); her head is a
            // bit above that.
            if (context.pochi != NULL) {
                const D3DXVECTOR2 pp = context.pochi->GetPosition();
                const float headX = pp.x + 25.0f;
                const float headTop = pp.y - 12.0f;

                // "!" bubble over Pochi's head near an item or fightable enemy.
                if (exclaimTex != NULL) {
                    bool nearInteractable = false;
                    for (size_t i = 0; i < itemSprites.size() && !nearInteractable; ++i)
                        if (!itemCollected[i] && TouchingItem(context.pochi, itemSprites[i]))
                            nearInteractable = true;
                    for (size_t i = 0; i < bossEnemies.size() && !nearInteractable; ++i) {
                        if (bossCleared[i]) continue;
                        if (!Cheats::enabled && config.bossesInOrder && i > 0 && !bossCleared[i - 1]) continue;
                        if (IsNear(pp, bossEnemies[i]->GetSprite()->GetPosition(), kInteractRadius))
                            nearInteractable = true;
                    }
                    if (nearInteractable) {
                        const float sz = 40.0f;
                        ui::DrawTexture(context.spriteBrush, exclaimTex, 1254, 1254,
                            headX - sz * 0.5f, headTop - sz - 6.0f,
                            sz / 1254.0f, sz / 1254.0f);
                    }
                }

                // Floating "Leveled Up!" centred over Pochi's head. Drawn
                // through the shared brush (a NULL-sprite font draw is
                // unreliable inside Main's frame-long batch), with the
                // transform reset first so it doesn't inherit Pochi's 2x
                // transform from pochi->Draw().
                if (levelUpFrames > 0 && levelUpFont != NULL && floatText != NULL) {
                    D3DXMATRIX ident;
                    D3DXMatrixIdentity(&ident);
                    context.spriteBrush->SetTransform(&ident);

                    const float rise = (150 - levelUpFrames) * 0.10f;
                    // ~7px per glyph at this font size - centre the string on
                    // Pochi's head rather than a fixed offset (the boost text
                    // is much longer than "Leveled Up!").
                    const float tx = headX - (float)strlen(floatText) * 3.5f;
                    const float ty = headTop - 24.0f - rise;
                    levelUpFont->Draw(floatText, tx + 1.0f, ty + 1.0f, D3DCOLOR_XRGB(20, 15, 0), context.spriteBrush);
                    levelUpFont->Draw(floatText, tx, ty, D3DCOLOR_XRGB(255, 232, 120), context.spriteBrush);
                }
            }

            // Layers like the forest's leaf canopy draw last, in front of
            // both Pochi and any bosses.
            if (hasForeground) map->DrawOnlyLayers(context.spriteBrush, config.foregroundLayers);

            if (context.pochi != NULL && interactPrompt != NULL) {
                D3DXVECTOR2 pochiPos = context.pochi->GetPosition();

                // Same gold-with-drop-shadow look as the "Leveled Up!" text,
                // centred across the top of the screen.
                auto drawPrompt = [&](const char* text) {
                    const float tx = 640.0f - (float)strlen(text) * 5.2f;
                    const float ty = 26.0f;
                    interactPrompt->Draw(text, tx + 1.0f, ty + 1.0f, D3DCOLOR_XRGB(20, 15, 0), context.spriteBrush);
                    interactPrompt->Draw(text, tx, ty, D3DCOLOR_XRGB(255, 232, 120), context.spriteBrush);
                };

                // At a sealed exit: tell the player why they can't leave yet.
                bool shownExitLock = false;
                if (ExitLocked()) {
                    AABB pb = Physics::GetBounds(context.pochi);
                    const bool atRightEdge = config.onReachRightEdge &&
                        pb.right >= (float)map->GetWidthPixels() - 40.0f;
                    const bool atDoorway = config.onEnterDoorway &&
                        IsNear(pochiPos, config.doorwayPosition, config.doorwayRadius + 40.0f);
                    if (atRightEdge || atDoorway) {
                        drawPrompt("DEFEAT ALL ENEMIES FIRST");
                        shownExitLock = true;
                    }
                }

                for (size_t i = 0; !shownExitLock && i < bossEnemies.size(); ++i) {
                    if (bossCleared[i]) continue;
                    if (!IsNear(pochiPos, bossEnemies[i]->GetSprite()->GetPosition(), kInteractRadius)) continue;

                    if (!Cheats::enabled && config.bossesInOrder && i > 0 && !bossCleared[i - 1])
                        drawPrompt("DEFEAT THE OTHER ENEMY FIRST");
                    else
                        drawPrompt("PRESS F TO FIGHT");
                    break;
                }
            }

            if (hud != NULL && context.playerStats != NULL) {
                hud->Draw(context.spriteBrush, *context.playerStats);
            }
            // The "CHEAT MODE" indicator is drawn globally in Main.cpp so it
            // shows in every state, not just here.
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
            { BossId::SkullBones, 440.0f, 100.0f },
            { BossId::Goblin, 780.0f, 317.0f }
        };
        // The forest and maze art line up at the shared edge, so Pochi
        // keeps whatever Y she had when she walked off the forest's right
        // edge - only X resets, just inside the maze's left edge (far enough
        // in that the left-edge backtrack trigger doesn't fire on arrival) -
        // for a seamless crossing instead of snapping to one spot.
        config.computeSpawnPosition = [](const D3DXVECTOR2& current) {
            return D3DXVECTOR2(40.0f, current.y);
        };
        // Walk back into the left edge to return to the forest, landing at
        // the forest's right edge with the same Y.
        config.onReachLeftEdge = [] { return CreateForestState(); };
        config.leftEdgeSpawn = D3DXVECTOR2(1160.0f, OverworldConfig::kCarryY);
        // The Maze wall layer occupies pixel Y 32..688. Fence Pochi's feet
        // just inside that so she can't slip along the open grass above or
        // below the hedges and skip the whole maze.
        config.fenceTop = 40.0f;
        config.fenceBottom = 680.0f;
        // Pochi has to clear the maze - SkullBones then the Goblin - before
        // the right-edge exit to the ruins will open.
        config.requireBossesCleared = true;
        config.bossesInOrder = true;
        // A barred gate across the right-edge opening while the maze is
        // uncleared - stands in the fence band (y 40..680), stops Pochi at
        // x 1232. Drop a PNG in via config.gateTexture to replace the bars.
        config.gateX = 1232.0f;
        config.gateY = 40.0f;
        config.gateWidth = 34.0f;
        config.gateHeight = 640.0f;
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

        // The path leading off the forest's TOP-LEFT corner goes to the
        // hidden Tarumt area where the secret boss (Mr Andrew) waits. Nudge
        // the position/radius to sit on the actual path tile.
        config.doorwayPosition = D3DXVECTOR2(110.0f, 30.0f);
        config.doorwayRadius = 90.0f;
        config.onEnterDoorway = [] { return CreateTarumtState(); };
        return config;
    }

    // The hidden Tarumt arena - one enemy, Mr Andrew. Beating him (or just
    // walking back out the right edge) returns to the forest.
    OverworldConfig MakeTarumtConfig() {
        OverworldConfig config;
        config.mapId = MapId::Tarumt;
        config.foregroundLayers = { "Tree_leaf" }; // canopy draws in front of Pochi
        // Mr Andrew stands IN FRONT OF the campus gate (on Pochi's side of
        // it), not behind it. Nudge to line up with the gate art.
        config.bosses = { { BossId::MrAndrew, 640.0f, 470.0f } };
        // The forest's top-left path connects to Tarumt's BOTTOM-RIGHT, so
        // Pochi walks in from that corner. Kept clear of the right-edge exit
        // trigger (~x 1275) so arriving doesn't immediately bounce her back.
        config.computeSpawnPosition = [](const D3DXVECTOR2&) {
            return D3DXVECTOR2(1120.0f, 600.0f);
        };
        // Walk back to the bottom-right corner (right edge) to return to the
        // forest the way she came - or beat Mr Andrew and get sent back the
        // same way. Either way she lands on the forest's top-left path,
        // clear of the doorway that would just bring her straight back.
        config.onReachRightEdge = [] { return CreateForestState(); };
        config.onAllCleared     = [] { return CreateForestState(); };
        config.rightEdgeSpawn   = D3DXVECTOR2(110.0f, 150.0f);
        return config;
    }

    std::unique_ptr<GameState> CreateTarumtState() {
        return CreateOverworldState(MakeTarumtConfig());
    }

    // The maze (boxy garden hedges) and the ruins exterior (an organic
    // lake/forest scene) aren't drawn to line up at any edge, and the
    // ruins' upper-left is water - so Pochi enters from the LEFT edge
    // carrying her maze-exit Y (clamped to the dry lower-left band) and
    // then walks in, rather than snapping to a point mid-map. Tile
    // collision nudges her off the water if the clamped Y still lands wet.
    OverworldConfig MakeRuinsExteriorConfig() {
        OverworldConfig config;
        config.mapId = MapId::RuinsExterior;
        config.foregroundLayers = { "Tree_leaf" };
        config.computeSpawnPosition = [](const D3DXVECTOR2& current) {
            // A little way in from the left edge so the left-edge backtrack
            // trigger doesn't fire the instant she arrives from the maze.
            return D3DXVECTOR2(55.0f, std::clamp(current.y, 360.0f, 630.0f));
        };
        // The dark doorway opening between the two gargoyle statues at the
        // temple's entrance - walking up to it leads inside.
        config.doorwayPosition = D3DXVECTOR2(780.0f, 190.0f);
        config.doorwayRadius = 60.0f;
        config.onEnterDoorway = [] { return CreateRuinsInteriorState(); };
        // Backtrack out the left edge to the maze (re-enters it fresh - its
        // gate and bosses reset, since map progress isn't persisted).
        config.onReachLeftEdge = [] { return CreateMazeState(); };
        config.leftEdgeSpawn = D3DXVECTOR2(1160.0f, OverworldConfig::kCarryY);
        return config;
    }

    std::unique_ptr<GameState> CreateRuinsExteriorState() {
        return CreateOverworldState(MakeRuinsExteriorConfig());
    }

    // Maki (final boss) stands at the top of the main hall. Beating her is
    // the end of the journey - the ending screen rolls after the fight.
    OverworldConfig MakeRuinsInteriorConfig() {
        OverworldConfig config;
        config.mapId = MapId::RuinsInterior;
        config.bosses = {
            { BossId::Maki, 610.0f, 200.0f }
        };
        config.onAllCleared = [] { return CreateEndingState(); };
        config.computeSpawnPosition = [](const D3DXVECTOR2&) {
            // The entrance corridor is the open gap in the Walls_top layer,
            // cols 38-41 (x 608-672); its bottom row is the map's outer
            // boundary wall. Spawn centred in that gap and a few tiles up, so
            // Pochi's foot-box starts on clear floor instead of clipped into
            // the corridor wall / bottom edge.
            return D3DXVECTOR2(614.0f, 540.0f);
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
