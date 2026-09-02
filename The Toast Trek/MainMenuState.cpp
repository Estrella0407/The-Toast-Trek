#include "GameState.h"
#include "MainMenu.h"
#include "OverworldState.h"
#include "TutorialPopupState.h"
#include "Cheats.h"
#include <dinput.h>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    // The forest intro plays only the first time the player starts a run
    // the main menu is reachable again later (retry / return to menu), and
    // it shouldn't replay the tutorial then
    bool s_forestIntroShown = false;

    class MainMenuState : public GameState {
    private:
        MainMenu* menu;
        bool enterWasDown;
        bool endingWasDown;

    public:
        MainMenuState() : menu(NULL), enterWasDown(false), endingWasDown(false) {}
        ~MainMenuState() { delete menu; }

        void Initialize(GameContext& context) override {
            menu = new MainMenu(context.device, context.pochi);
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            // Dev shortcut: jump straight to the ending screen. Remove when
            // no longer needed
            if (JustPressed(context.keys, DIK_F10, endingWasDown)) {
                manager.Push(CreateEndingState());
                return;
            }

            if (JustPressed(context.keys, DIK_RETURN, enterWasDown)) {
                ResetRunProgress(context);   // Fresh run - level 1, empty pack, maps re-locked
                // queued in order: the forest initializes first, then the
                // popup lands on top of it. Closing the popup reveals the
                // already-running forest underneath
                manager.Push(CreateForestState());
                // Cheat mode skips the tutorial popup entirely (and leaves
                // S_forestIntroShown alone, so a later non-cheat run still
                // gets it)
                if (!s_forestIntroShown && !Cheats::enabled) {
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
            // The "CHEAT MODE" indicator is drawn globally in Main.cpp
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(245, 245, 245); }
    };
}

std::unique_ptr<GameState> CreateMainMenuState() {
    return std::make_unique<MainMenuState>();
}
