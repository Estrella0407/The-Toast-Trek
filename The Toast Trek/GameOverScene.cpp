#include "GameOverScene.h"
#include "GameStateManager.h"
#include "MainMenuScene.h"
#include "ForestScene.h"
#include "Font.h"
#include "SoundManager.h"
#include <dinput.h>
#include <cmath>

namespace {

// The lose screen. Only ever the single entry on the stack; both choices
// rebuild the run with ClearAndPush.
class GameOverScene : public GameScene {
public:
    explicit GameOverScene(SoundManager* soundMgr) : soundManage(soundMgr) {}
    ~GameOverScene() override { delete titleFont; delete statsFont; delete promptFont; }

    void Initialize(GameContext& context) override {
        titleFont = new Font(context.device, 0.0f, 180.0f, 1280, 80, 56, "Arial");
        statsFont = new Font(context.device, 0.0f, 300.0f, 1280, 50, 26, "Arial");
        promptFont = new Font(context.device, 0.0f, 500.0f, 1280, 60, 22, "Arial");
        if (soundManage) soundManage->PlaySfx("gameover", 0.8f);
    }

    void HandleInput(GameContext& context, GameStateManager& manager) override {
        if (JustPressed(context.keys, DIK_R, retryWasDown)) {
            ResetRunProgress(context);   // Fresh Pochi, empty pack, every map locked again
            manager.ClearAndPush(std::make_unique<ForestScene>());
        }
        if (JustPressed(context.keys, DIK_M, menuWasDown)) {
            manager.ClearAndPush(CreateMainMenuScene());
        }
    }

    void Update(GameContext& context, GameStateManager& manager) override {
        flashTimer += 0.016f;   // ~60 fps
        if (soundManage) soundManage->Update();
    }

    void Render(GameContext& context) override {
        LPD3DXSPRITE brush = context.spriteBrush;
        if (titleFont) {
            D3DCOLOR color = (fmod(flashTimer, 0.6f) > 0.3f)
                ? D3DCOLOR_XRGB(200, 50, 50) : D3DCOLOR_XRGB(255, 70, 70);
            titleFont->Draw("GAME OVER", color, brush);
        }
        if (statsFont)
            statsFont->Draw("Pochi has run out of health.", D3DCOLOR_XRGB(210, 200, 200), brush);
        if (promptFont)
            promptFont->Draw("R: RETRY FROM THE START     M: MAIN MENU",
                D3DCOLOR_XRGB(200, 200, 200), brush);
    }

    D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(35, 10, 10); }

private:
    Font* titleFont = nullptr;
    Font* statsFont = nullptr;
    Font* promptFont = nullptr;
    bool retryWasDown = false;
    bool menuWasDown = false;
    SoundManager* soundManage = nullptr;
    float flashTimer = 0.0f;   // Drives the title colour flash
};

} // namespace

std::unique_ptr<GameScene> CreateGameOverScene(SoundManager* sound) {
    return std::make_unique<GameOverScene>(sound);
}
