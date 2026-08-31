#include "GameState.h"
#include "Font.h"
#include "MainMenu.h"
#include "Physics.h"
#include "Sprite.h"
#include "TileMap.h"
#include "BattleState.h"
#include "Inventory.h"
#include "Pochi.h"
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

    // Arrow keys and WASD both drive movement, everywhere movement happens.
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

    // Pochi's sprite canvas (scaled ~100x60) is much bigger than her actual
    // standing pose, so colliding tiles against the full canvas made gaps
    // that look easily walkable (a maze corridor, two rocks either side of
    // a path) feel blocked. Collide a smaller box at her feet instead.
    constexpr float kPochiFootWidthRatio = 0.5f;
    constexpr float kPochiFootHeightRatio = 0.6f;

    void ResolveWorldCollisions(GameContext& context, TileMap* map) {
        if (context.pochi == NULL || map == NULL) return;

        Physics::ClampToBounds(context.pochi, 0.0f, 0.0f,
            (float)map->GetWidthPixels(), (float)map->GetHeightPixels());

        Physics::ResolveCollisionShapes(context.pochi, map, kPochiFootWidthRatio, kPochiFootHeightRatio);
    }

    class TutorialState;
    std::unique_ptr<GameState> CreateTutorialState();

    class MazeState;

    class Level1State;
    std::unique_ptr<GameState> CreateLevel1State();

    class MainMenuState : public GameState {
    private:
        MainMenu* menu;
        bool enterWasDown;

    public:
        MainMenuState() : menu(NULL), enterWasDown(false) {}
        ~MainMenuState() { delete menu; }

        void Initialize(GameContext& context) override {
			if (context.playerStats != NULL) context.playerStats->SetLevel(1);
            menu = new MainMenu(context.device, context.pochi);
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            if (JustPressed(context.keys, DIK_RETURN, enterWasDown)) {
                manager.Push(CreateTutorialState());
            }
        }

        void Update(GameContext&, GameStateManager&) override {
            if (menu != NULL) menu->Update();
        }

        void Render(GameContext& context) override {
            if (menu != NULL) menu->Draw(context.spriteBrush);
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(245, 245, 245); }
    };

    class GameOverState : public GameState {
    private:
        Font* titleFont;
        Font* promptFont;
        bool retryWasDown;
        bool menuWasDown;

    public:
        GameOverState() : titleFont(NULL), promptFont(NULL), retryWasDown(false), menuWasDown(false) {}
        ~GameOverState() { delete titleFont; delete promptFont; }

        void Initialize(GameContext& context) override {
            titleFont = new Font(context.device, 0.0f, 250.0f, 1280, 70, 48, "Arial");
            promptFont = new Font(context.device, 0.0f, 350.0f, 1280, 60, 22, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            if (JustPressed(context.keys, DIK_R, retryWasDown)) manager.Pop();
            if (JustPressed(context.keys, DIK_M, menuWasDown)) manager.ClearAndPush(CreateMainMenuState());
        }

        void Update(GameContext&, GameStateManager&) override {}

        void Render(GameContext&) override {
            if (titleFont != NULL) titleFont->Draw("GAME OVER", D3DCOLOR_XRGB(255, 255, 255));
            if (promptFont != NULL) promptFont->Draw("R: RETRY    M: RETURN TO MAIN MENU", D3DCOLOR_XRGB(220, 220, 220));
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(35, 35, 35); }
    };

    class TutorialState : public GameState {
    private:
        bool gameOverWasDown;

    public:
        TutorialState() : gameOverWasDown(false) {}

        void Initialize(GameContext& context) override {
            // MainMenu parks the shared Pochi sprite at its menu pose; reset
            // it to the tutorial's spawn point every time this state starts.
            if (context.pochi != NULL) {
                context.pochi->SetPosition(250.0f, 360.0f);
                context.pochi->CropToFrame(0);
            }
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            // Debug shortcut straight to a fight, bypassing the walk to the maze.
            if (JustPressed(context.keys,DIK_P, gameOverWasDown)) {
                manager.Push(CreateBattleState(BossId::MrAndrew));
                return;
            }
        }

        void Update(GameContext& context, GameStateManager& manager) override {
            if (context.pochi == NULL) return;

            D3DXVECTOR2 beforeMove = context.pochi->GetPosition();
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

            ResolveWorldCollisions(context, context.forestMap);

            // Walking off the right edge of the forest leads into the maze.
            if (context.forestMap != NULL) {
                AABB pochiBounds = Physics::GetBounds(context.pochi);
                if (pochiBounds.right >= (float)context.forestMap->GetWidthPixels() - 5.0f) {
                    manager.Push(CreateMazeState());
                }
            }
        }

        void Render(GameContext& context) override {
            // Only the leaf canopy draws in front of Pochi
            if (context.forestMap != NULL) context.forestMap->DrawExcludingLayers(context.spriteBrush, { "Tree_Leaf" });
            if (context.pochi != NULL) context.pochi->Draw(context.spriteBrush);
            if (context.forestMap != NULL) context.forestMap->DrawOnlyLayers(context.spriteBrush, { "Tree_Leaf" });
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

    class MazeState : public GameState {
    private:
        bool interactWasDown;
		bool pickupWasDown;
        bool level1Cleared;
        bool level2Cleared;
		bool healthPotionPickedUp;
		bool bonePickedUp;
        Enemy* skullBonesEnemy;
        Enemy* goblinEnemy;
		Sprite* healthPotion;
		Sprite* bone;
        Font* interactPrompt;
		Font* itemPrompt;

        static constexpr float kMazeEntranceX = 10.0f;
        static constexpr float kSkullBonesX = 140.0f;
        static constexpr float kSkullBonesY = 35.0f;
        static constexpr float kGoblin1X = 760.0f;
        static constexpr float kGoblin1Y = 315.0f;
        static constexpr float kInteractRadius = 90.0f;
		static constexpr float kHealthPotionX = 528.0f;
		static constexpr float kHealthPotionY = 360.0f;
		static constexpr float kBoneX = 1104.0f;
		static constexpr float kBoneY = 97.0f;

        static bool IsNear(const D3DXVECTOR2& a, const D3DXVECTOR2& b, float radius) {
            float dx = a.x - b.x;
            float dy = a.y - b.y;
            return (dx * dx + dy * dy) <= (radius * radius);
        }

        bool IsNearActiveBoss(const D3DXVECTOR2& pochiPos) const {
            if (!level1Cleared && skullBonesEnemy != NULL &&
                IsNear(pochiPos, skullBonesEnemy->GetSprite()->GetPosition(), kInteractRadius)) return true;
            if (!level2Cleared && goblinEnemy != NULL &&
                IsNear(pochiPos, goblinEnemy->GetSprite()->GetPosition(), kInteractRadius)) return true;
            return false;
        }

		bool IsTouchingItem(Sprite* pochiSprite, Sprite* item) const {
			if (pochiSprite == NULL || item == NULL) return false;
			return Physics::CheckAABBCollision(
				Physics::GetFootBounds(pochiSprite, kPochiFootWidthRatio, kPochiFootHeightRatio),
				Physics::GetBounds(item));
		}

    public:
        MazeState()
            : interactWasDown(false), pickupWasDown(false), level1Cleared(false), level2Cleared(false),
			healthPotionPickedUp(false), bonePickedUp(false), skullBonesEnemy(NULL), goblinEnemy(NULL),
			healthPotion(NULL), bone(NULL), interactPrompt(NULL), itemPrompt(NULL) {}

        ~MazeState() {
            delete skullBonesEnemy;
            delete goblinEnemy;
			delete healthPotion;
			delete bone;
            delete interactPrompt;
			delete itemPrompt;
        }

        void Initialize(GameContext& context) override {
            // Keep Pochi's Y from the forest exit and only reset X to the
            // maze's left edge, so crossing the seam between the two maps
            // is seamless instead of snapping to a fixed spawn point.
            if (context.pochi != NULL) {
                D3DXVECTOR2 currentPosition = context.pochi->GetPosition();
                context.pochi->SetPosition(kMazeEntranceX, currentPosition.y);
                context.pochi->CropToFrame(0);
            }

            skullBonesEnemy = CreateBossEnemy(context.device, BossId::SkullBones, kSkullBonesX, kSkullBonesY);
            goblinEnemy = CreateBossEnemy(context.device, BossId::Goblin, kGoblin1X, kGoblin1Y);
			healthPotion = new Sprite(context.device, "Assets/Item/heathPotion.png", 18, 20, 1, 1, 1,
				kHealthPotionX, kHealthPotionY);
			bone = new Sprite(context.device, "Assets/Item/bone.png", 32, 32, 1, 1, 1,
				kBoneX, kBoneY);
			healthPotion->SetScale(2.0f);
			bone->SetScale(1.5f);

            interactPrompt = new Font(context.device, 0.0f, 20.0f, 1280, 40, 20, "Arial");
			itemPrompt = new Font(context.device, 0.0f, 0.0f, 250, 30, 18, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            if (context.pochi == NULL) return;

			if (JustPressed(context.keys, DIK_E, pickupWasDown)) {
				if (!healthPotionPickedUp && IsTouchingItem(context.pochi, healthPotion)) {
					healthPotionPickedUp = true;
					if (context.inventory != NULL) context.inventory->Add(ItemType::HealthPotion);
					return;
				}
				if (!bonePickedUp && IsTouchingItem(context.pochi, bone)) {
					bonePickedUp = true;
					if (context.inventory != NULL) context.inventory->Add(ItemType::Bone);
					return;
				}
			}
            if (!JustPressed(context.keys, DIK_F, interactWasDown)) return;

            D3DXVECTOR2 pochiPos = context.pochi->GetPosition();

            if (!level1Cleared && skullBonesEnemy != NULL &&
                IsNear(pochiPos, skullBonesEnemy->GetSprite()->GetPosition(), kInteractRadius)) {
				// Skull Bone is the level-1 encounter.
				if (context.playerStats != NULL && context.playerStats->GetLevel() != 1) {
					context.playerStats->SetLevel(1);
				}
                manager.Push(CreateBattleState(BossId::SkullBones));
                return;
            }

            if (!level2Cleared && goblinEnemy != NULL &&
                IsNear(pochiPos, goblinEnemy->GetSprite()->GetPosition(), kInteractRadius)) {
				// The maze permits reaching Goblin before Skull Bone. Always use
				// the intended level-2 stats for this encounter so one 3-damage
				// special projectile cannot incorrectly end a level-1 battle.
				if (context.playerStats != NULL && context.playerStats->GetLevel() < 2) {
					context.playerStats->SetLevel(2);
				}
                manager.Push(CreateBattleState(BossId::Goblin));
                return;
            }
        }

        void Update(GameContext& context, GameStateManager&) override {
            // Pick up the result of whichever battle we just returned from -
            // BattleState sets this right before popping itself.
            if (context.lastBattleOutcome == BattleOutcome::Victory) {
				if (context.lastBattleBoss == BossId::SkullBones) {
					level1Cleared = true;
					if (context.playerStats != NULL) context.playerStats->SetLevel(2);
				}
				else if (context.lastBattleBoss == BossId::Goblin) {
					level2Cleared = true;
					if (context.playerStats != NULL) context.playerStats->SetLevel(3);
				}
            }
			else if (context.lastBattleOutcome == BattleOutcome::Defeat && context.playerStats != NULL) {
				context.playerStats->RestoreFull();
			}
			if (context.lastBattleOutcome != BattleOutcome::None) interactWasDown = false;
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

            ResolveWorldCollisions(context, context.mazeMap);
        }

        void Render(GameContext& context) override {
            if (context.mazeMap != NULL) context.mazeMap->Draw(context.spriteBrush);

            if (!level1Cleared && skullBonesEnemy != NULL) skullBonesEnemy->Render(context.spriteBrush);
            if (!level2Cleared && goblinEnemy != NULL) goblinEnemy->Render(context.spriteBrush);
			if (!healthPotionPickedUp && healthPotion != NULL) healthPotion->Draw(context.spriteBrush);
			if (!bonePickedUp && bone != NULL) bone->Draw(context.spriteBrush);

            if (context.pochi != NULL) context.pochi->Draw(context.spriteBrush);

			if (context.pochi != NULL && itemPrompt != NULL) {
				Sprite* nearbyItem = NULL;
				const char* itemName = NULL;
				if (!healthPotionPickedUp && IsTouchingItem(context.pochi, healthPotion)) {
					nearbyItem = healthPotion;
					itemName = "Health Potion - recover health 3";
				}
				else if (!bonePickedUp && IsTouchingItem(context.pochi, bone)) {
					nearbyItem = bone;
					itemName = "Bone - recover armor 2";
				}
				if (nearbyItem != NULL) {
					D3DXVECTOR2 position = nearbyItem->GetPosition();
					itemPrompt->Draw(itemName, position.x - 90.0f, position.y - 48.0f,
						D3DCOLOR_XRGB(255, 255, 255));
					itemPrompt->Draw("Press E to pick up", position.x - 90.0f, position.y - 25.0f,
						D3DCOLOR_XRGB(255, 255, 255));
				}
			}

            if (context.pochi != NULL && interactPrompt != NULL &&
                IsNearActiveBoss(context.pochi->GetPosition())) {
                interactPrompt->Draw("PRESS F TO FIGHT", D3DCOLOR_XRGB(255, 255, 255));
            }
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

    class Level1State : public GameState {
    private:
        bool gameOverWasDown;

    public:
        Level1State() : gameOverWasDown(false) {}

        void Initialize(GameContext& context) override {
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            // Temporary trigger until Level 1 has a real death condition.
            if (JustPressed(context.keys, DIK_F1, gameOverWasDown)) {
                manager.Push(std::make_unique<GameOverState>());
            }
        }

        void Update(GameContext& context, GameStateManager&) override {
            if (context.pochi == NULL) return;

            D3DXVECTOR2 beforeMove = context.pochi->GetPosition();
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
            ResolveWorldCollisions(context, context.forestMap);
        }

        void Render(GameContext& context) override {

        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

    // Ready-to-use placeholder for later levels. Add a menu/level-exit call that
    // pushes one of these states when those levels are implemented.
    class PlaceholderLevelState : public GameState {
    private:
        Font* label;
    public:
        PlaceholderLevelState() : label(NULL) {}
        ~PlaceholderLevelState() { delete label; }
        void Initialize(GameContext& context) override {
            label = new Font(context.device, 0.0f, 320.0f, 1280, 60, 32, "Arial");
        }
        void HandleInput(GameContext&, GameStateManager&) override {}
        void Update(GameContext&, GameStateManager&) override {}
        void Render(GameContext&) override {
            if (label != NULL) label->Draw("NEXT LEVEL - COMING SOON", D3DCOLOR_XRGB(255, 255, 255));
        }
        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(20, 40, 70); }
    };

    std::unique_ptr<GameState> CreateTutorialState() {
        return std::make_unique<TutorialState>();
    }

    std::unique_ptr<GameState> CreateLevel1State() {
        return std::make_unique<Level1State>();
    }
}

GameStateManager::GameStateManager(GameContext& gameContext)
    : context(gameContext), pendingPopCount(0), clearRequested(false) {
}

void GameStateManager::Push(std::unique_ptr<GameState> state) {
    if (state != NULL) pendingPushes.push_back(std::move(state));
}

void GameStateManager::Pop() {
    ++pendingPopCount;
}

void GameStateManager::ClearAndPush(std::unique_ptr<GameState> state) {
    clearRequested = true;
    pendingPopCount = 0;
    pendingPushes.clear();
    if (state != NULL) pendingPushes.push_back(std::move(state));
}

void GameStateManager::ApplyPendingChanges() {
    if (clearRequested) stateStack.clear();
    clearRequested = false;
    while (pendingPopCount > 0 && !stateStack.empty()) {
        stateStack.pop_back();
        --pendingPopCount;
    }
    pendingPopCount = 0;
    for (size_t i = 0; i < pendingPushes.size(); ++i) {
        pendingPushes[i]->Initialize(context);
        stateStack.push_back(std::move(pendingPushes[i]));
    }
    pendingPushes.clear();
}

void GameStateManager::HandleInput() {
    if (!stateStack.empty()) stateStack.back()->HandleInput(context, *this);
}

void GameStateManager::Update() {
    if (!stateStack.empty()) stateStack.back()->Update(context, *this);
}

void GameStateManager::Render() {
    if (!stateStack.empty()) stateStack.back()->Render(context);
}

D3DCOLOR GameStateManager::ClearColor() const {
    return stateStack.empty() ? D3DCOLOR_XRGB(0, 0, 0) : stateStack.back()->ClearColor();
}

std::unique_ptr<GameState> CreateMainMenuState() {
    return std::make_unique<MainMenuState>();
}

std::unique_ptr<GameState> CreateMazeState() {
    return std::make_unique<MazeState>();
}

std::unique_ptr<GameState> CreateBattleState(BossId bossId) {
    return std::make_unique<BattleState>(bossId);
}
