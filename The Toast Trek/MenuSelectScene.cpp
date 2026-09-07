#include "MenuSelectScene.h"
#include "GameStateManager.h"
#include "MainMenuScene.h"
#include "OverworldScene.h"
#include "ForestScene.h"
#include "TutorialPopupScene.h"
#include "SettingsScene.h"
#include "BallPitScene.h"
#include "SaveGame.h"
#include "Cheats.h"
#include "Font.h"
#include "Button.h"
#include "Sprite.h"
#include "Pochi.h"
#include "Inventory.h"
#include "SoundManager.h"
#include <Windows.h>
#include <dinput.h>

namespace {

    // The forest intro plays only the first time a run starts
    bool s_forestIntroShown = false;

    enum { OPT_NEW = 0, OPT_CONTINUE, OPT_PHYSICS, OPT_SETTINGS, OPT_QUIT, OPT_COUNT };
    const char* kOptions[OPT_COUNT] = { "New Game", "Continue", "Physics Demo", "Settings", "Quit" };

    // A column of buttons, centred under the title
    constexpr int kBtnX = 470;
    constexpr int kBtnW = 340;
    constexpr int kBtnH = 44;
    constexpr int kFirstRowY = 340;
    constexpr int kRowStep = 52;

    class MenuSelectScene : public GameScene {
    private:
        int sel;
        bool hasSave;

        Font* titleFont;
        Button* buttons[OPT_COUNT];

        bool enterWasDown, escWasDown, upWasDown, downWasDown;

        // Step the keyboard cursor, skipping a disabled row
        void MoveSel(int dir) {
            for (int n = 0; n < OPT_COUNT; ++n) {
                sel = (sel + dir + OPT_COUNT) % OPT_COUNT;
                if (!(sel == OPT_CONTINUE && !hasSave)) return;
            }
        }

        void Activate(int option, GameContext& context, GameStateManager& manager) {
            if (option == OPT_CONTINUE && !hasSave) return;
            if (context.sound != NULL) context.sound->PlaySfx("click");
            switch (option) {
            case OPT_NEW:      StartNewRun(context, manager); break;
            case OPT_CONTINUE: ContinueRun(context, manager); break;
            case OPT_PHYSICS:  manager.Push(CreateBallPitScene()); break;
            case OPT_SETTINGS: manager.Push(CreateSettingsScene(this)); break;
            case OPT_QUIT:     PostQuitMessage(0); break;
            }
        }

        void StartNewRun(GameContext& context, GameStateManager& manager) {
            save::ClearProgress();
            ResetRunProgress(context);
            manager.ClearAndPush(std::make_unique<ForestScene>());
            if (!s_forestIntroShown && !Cheats::enabled) {
                manager.Push(CreateForestIntroPopup());
                s_forestIntroShown = true;
            }
        }

        void ContinueRun(GameContext& context, GameStateManager& manager) {
            const save::Progress p = save::LoadProgress();
            if (!p.valid) return;

            ResetRunProgress(context);   // Clean slate, then layer the save on top
            if (context.pochi != NULL) context.pochi->SetLevel(p.level);
            if (context.inventory != NULL) {
                context.inventory->SetCount(ItemType::HealthPotion, p.potions);
                context.inventory->SetCount(ItemType::Bone, p.bones);
                context.inventory->SetCount(ItemType::Toast, p.toast);
            }
            context.clearedMaps.clear();
            for (int m : p.clearedMaps) context.clearedMaps.insert((MapId)m);
            context.collectedItems.clear();
            for (int k : p.collectedItems) context.collectedItems.insert(k);
            context.clearedBosses.clear();
            for (int k : p.clearedBosses) context.clearedBosses.insert(k);

            // Drop Pochi back on the exact spot the save was taken
            if (p.px != 0.0f || p.py != 0.0f) {
                context.pendingSpawn = D3DXVECTOR2(p.px, p.py);
                context.hasPendingSpawn = true;
            }

            manager.ClearAndPush(CreateOverworldSceneForMap((MapId)p.mapId));
        }

    public:
        MenuSelectScene()
            : sel(0), hasSave(false), titleFont(NULL),
              enterWasDown(true), escWasDown(true), upWasDown(false), downWasDown(false) {
            for (int i = 0; i < OPT_COUNT; ++i) buttons[i] = NULL;
        }

        ~MenuSelectScene() override {
            delete titleFont;
            for (int i = 0; i < OPT_COUNT; ++i) delete buttons[i];
        }

        void Initialize(GameContext& context) override {
            hasSave = save::HasProgress();
            sel = hasSave ? OPT_CONTINUE : OPT_NEW;

            // Whatever opened this screen (Enter or a click) may still be held
            enterWasDown = context.keys != NULL && (context.keys[DIK_RETURN] & 0x80) != 0;
            escWasDown = context.keys != NULL && (context.keys[DIK_ESCAPE] & 0x80) != 0;

            // Same placement as the title screen (MainMenuScene.cpp)
            titleFont = new Font(context.device, 0.0f, 180.0f, 1280, 80, 48, "Arial");

            for (int i = 0; i < OPT_COUNT; ++i) {
                buttons[i] = new Button(context.device, kOptions[i],
                                        kBtnX, kFirstRowY + i * kRowStep, kBtnW, kBtnH, 22);
            }
            buttons[OPT_CONTINUE]->SetEnabled(hasSave);

            // Prime each button's click edge with the current mouse state so a
            // click still held from the previous screen doesn't fall through
            for (int i = 0; i < OPT_COUNT; ++i) {
                buttons[i]->Update(context.mouseX, context.mouseY, context.mouseLeftDown);
            }
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.ClearAndPush(CreateMainMenuScene());
                return;
            }

            if (JustPressed(k, DIK_UP, upWasDown))   MoveSel(-1);
            if (JustPressed(k, DIK_DOWN, downWasDown)) MoveSel(+1);

            // Mouse: hovering a button moves the cursor onto it, a click fires it
            for (int i = 0; i < OPT_COUNT; ++i) {
                const bool clicked =
                    buttons[i]->Update(context.mouseX, context.mouseY, context.mouseLeftDown);
                if (buttons[i]->IsHovered()) sel = i;
                if (clicked) { Activate(i, context, manager); return; }
            }

            if (JustPressed(k, DIK_RETURN, enterWasDown)) Activate(sel, context, manager);
        }

        void Update(GameContext&, GameStateManager&) override {
            for (int i = 0; i < OPT_COUNT; ++i) buttons[i]->SetSelected(i == sel);
        }

        void Render(GameContext& context) override {
            LPD3DXSPRITE b = context.spriteBrush;

            if (context.pochi != NULL) {
                context.pochi->GetSprite()->Draw(b);
                // Sprite::Draw leaves a scale/translate matrix on the brush -
                // reset it or the buttons' text/lines inherit it and vanish.
                D3DXMATRIX identity;
                D3DXMatrixIdentity(&identity);
                b->SetTransform(&identity);
            }

            for (int i = 0; i < OPT_COUNT; ++i) buttons[i]->Render(b);

            // Title through the shared brush, drawn last
            titleFont->Draw("THE TOAST TREK", D3DCOLOR_XRGB(35, 35, 35), b);
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(245, 245, 245); }
    };

} // Namespace

std::unique_ptr<GameScene> CreateMenuSelectScene() {
    return std::make_unique<MenuSelectScene>();
}
