#include "GameState.h"
#include "MainMenu.h"
#include "MenuSelectState.h"
#include <dinput.h>

namespace {
    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    // The title screen: "THE TOAST TREK" + Pochi + "PRESS ENTER TO CONTINUE"
    // Enter opens the New Game / Continue / Settings / Quit choice screen
    class MainMenuState : public GameState {
    private:
        MainMenu* menu;
        bool enterWasDown, endingWasDown, mouseWasDown;

    public:
        MainMenuState() : menu(NULL), enterWasDown(false), endingWasDown(false), mouseWasDown(true) {}
        ~MainMenuState() { delete menu; }

        void Initialize(GameContext& context) override {
            menu = new MainMenu(context.device, context.pochi);
            // Start "down" so a key/click still held from launch doesn't skip
            // this screen - it must be released and pressed again here
            enterWasDown = true;
            endingWasDown = true;
            mouseWasDown = true;
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            // Dev shortcut: jump straight to the ending screen
            if (JustPressed(context.keys, DIK_F10, endingWasDown)) {
                manager.Push(CreateEndingState());
                return;
            }

            const bool click = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;

            if (JustPressed(context.keys, DIK_RETURN, enterWasDown) || click) {
                manager.ClearAndPush(CreateMenuSelectState());
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
