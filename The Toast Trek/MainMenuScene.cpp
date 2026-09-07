#include "GameStateManager.h"
#include "Pochi.h"
#include "Sprite.h"
#include "Font.h"
#include "MenuSelectScene.h"
#include "MainMenuScene.h"
#include "EndingScene.h"
#include "Keys.h"

namespace {

    // The title screen: "THE TOAST TREK" + animated Pochi + "PRESS ENTER TO
    // CONTINUE". Enter (or a click) opens the New Game / Continue / Settings /
    // Quit choice screen.
    class MainMenuScene : public GameScene {
    private:
        Sprite* pochi;          // borrowed from GameContext (Pochi loads it)
        Font* titleFont;
        Font* promptFont;
        int animCounter;
        int animDelay;

        bool enterWasDown, endingWasDown, mouseWasDown;

    public:
        MainMenuScene()
            : pochi(NULL), titleFont(NULL), promptFont(NULL),
              animCounter(0), animDelay(8),
              enterWasDown(false), endingWasDown(false), mouseWasDown(true) {}

        ~MainMenuScene() override {
            delete titleFont;
            delete promptFont;
        }

        void Initialize(GameContext& context) override {
            titleFont = new Font(context.device, 0.0f, 180.0f, 1280, 80, 48, "Arial");
            promptFont = new Font(context.device, 0.0f, 480.0f, 1280, 60, 24, "Arial");

            pochi = context.pochi != NULL ? context.pochi->GetSprite() : NULL;
            if (pochi != NULL) {
                pochi->SetPosition(615.0f, 320.0f);   // centre pose for the menu
                pochi->CropToFrame(0);
            }
            animCounter = 0;

            // Start "down" so a key/click still held from launch doesn't skip
            // this screen - it must be released and pressed again here
            enterWasDown = true;
            endingWasDown = true;
            mouseWasDown = true;
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            // Dev shortcut: jump straight to the ending screen
            if (JustPressed(context.keys, F10_KEY, endingWasDown)) {
                manager.Push(CreateEndingScene());
                return;
            }

            const bool click = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;

            if (JustPressed(context.keys, RETURN_KEY, enterWasDown) || click) {
                manager.ClearAndPush(CreateMenuSelectScene());
            }
        }

        void Update(GameContext&, GameStateManager&) override {
            if (pochi == NULL) return;
            if (++animCounter >= animDelay) {
                animCounter = 0;
                pochi->NextFrame();
            }
        }

        void Render(GameContext& context) override {
            LPD3DXSPRITE brush = context.spriteBrush;

            if (pochi != NULL && brush != NULL) {
                pochi->Draw(brush);
                // Sprite::Draw leaves its scale/translate matrix on the brush -
                // reset it or the text below inherits it and vanishes.
                D3DXMATRIX identity;
                D3DXMatrixIdentity(&identity);
                brush->SetTransform(&identity);
            }

            if (titleFont != NULL)
                titleFont->Draw("THE TOAST TREK", D3DCOLOR_XRGB(35, 35, 35), brush);
            if (promptFont != NULL)
                promptFont->Draw("PRESS ENTER TO CONTINUE", D3DCOLOR_XRGB(70, 70, 70), brush);
            // The "CHEAT MODE" indicator is drawn globally by GameStateManager
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(245, 245, 245); }
    };
}

std::unique_ptr<GameScene> CreateMainMenuScene() {
    return std::make_unique<MainMenuScene>();
}
