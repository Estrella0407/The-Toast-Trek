#include "GameState.h"
#include "MainMenu.h"
#include "OverworldState.h"
#include "TutorialPopupState.h"
#include <dinput.h>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    // The forest intro plays only the first time the player starts a run.
    // The main menu is reachable again later (retry / return to menu), and
    // it shouldn't replay the tutorial then.
    bool s_forestIntroShown = false;

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
                // Queued in order: the forest initializes first, then the
                // popup lands on top of it. Closing the popup reveals the
                // already-running forest underneath.
                manager.Push(CreateForestState());
                if (!s_forestIntroShown) {
                    manager.Push(CreateForestIntroPopup());
                    s_forestIntroShown = true;
                }
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
}

std::unique_ptr<GameState> CreateMainMenuState() {
    return std::make_unique<MainMenuState>();
}
