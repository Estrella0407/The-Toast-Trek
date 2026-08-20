#include "GameState.h"
#include "Font.h"
#include "MainMenu.h"
#include "Physics.h"
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

    void ResolveWorldCollisions(GameContext& context) {
        if (context.pochi == NULL || context.forestMap == NULL) return;

        Physics::ClampToBounds(context.pochi, 0.0f, 0.0f,
            (float)context.forestMap->GetWidthPixels(), (float)context.forestMap->GetHeightPixels());

        Physics::ResolveCollisionShapes(context.pochi, context.forestMap);
    }

    class TutorialState;
    std::unique_ptr<GameState> CreateTutorialState();

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
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {

        }

        void Update(GameContext& context, GameStateManager&) override {
            if (context.pochi == NULL) return;

            D3DXVECTOR2 beforeMove = context.pochi->GetPosition();
            bool isMoving = false;
            if (context.keys[DIK_LEFT] & 0x80) {
                context.pochi->Move((float)-context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(LEFT);
                isMoving = true;
            }
            else if (context.keys[DIK_RIGHT] & 0x80) {
                context.pochi->Move((float)context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(RIGHT);
                isMoving = true;
            }
            if (context.keys[DIK_UP] & 0x80) {
                context.pochi->Move(0.0f, (float)-context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            else if (context.keys[DIK_DOWN] & 0x80) {
                context.pochi->Move(0.0f, (float)context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            if (!isMoving) context.pochi->SetIdlePose();

            ResolveWorldCollisions(context);
        }

        void Render(GameContext& context) override {
            // Only the leaf canopy draws in front of Pochi
            if (context.forestMap != NULL) context.forestMap->DrawExcludingLayers(context.spriteBrush, { "Tree_Leaf" });
            if (context.pochi != NULL) context.pochi->Draw(context.spriteBrush);
            if (context.forestMap != NULL) context.forestMap->DrawOnlyLayers(context.spriteBrush, { "Tree_Leaf" });
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
            bool isMoving = false;
            if (context.keys[DIK_LEFT] & 0x80) {
                context.pochi->Move((float)-context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(LEFT);
                isMoving = true;
            }
            else if (context.keys[DIK_RIGHT] & 0x80) {
                context.pochi->Move((float)context.moveSpeed, 0.0f);
                context.pochi->AnimateWalk(RIGHT);
                isMoving = true;
            }
            if (context.keys[DIK_UP] & 0x80) {
                context.pochi->Move(0.0f, (float)-context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            else if (context.keys[DIK_DOWN] & 0x80) {
                context.pochi->Move(0.0f, (float)context.moveSpeed);
                context.pochi->AnimateWalk();
                isMoving = true;
            }
            if (!isMoving) context.pochi->SetIdlePose();
            ResolveWorldCollisions(context);
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