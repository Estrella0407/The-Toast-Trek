#include "MenuSelectState.h"
#include "GameState.h"
#include "OverworldState.h"
#include "TutorialPopupState.h"
#include "SettingsState.h"
#include "SaveGame.h"
#include "Cheats.h"
#include "Font.h"
#include "Sprite.h"
#include "Pochi.h"
#include "Inventory.h"
#include "SoundManage.h"
#include "UiFill.h"
#include <Windows.h>
#include <dinput.h>

namespace {

    bool JustPressed(BYTE* keys, int key, bool& wasDown) {
        const bool isDown = keys != NULL && (keys[key] & 0x80) != 0;
        const bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    bool InRect(float x, float y, float l, float t, float r, float b) {
        return x >= l && x <= r && y >= t && y <= b;
    }

    // The forest intro plays only the first time a run starts
    bool s_forestIntroShown = false;

    enum { OPT_NEW = 0, OPT_CONTINUE, OPT_SETTINGS, OPT_QUIT, OPT_COUNT };
    const char* kOptions[OPT_COUNT] = { "New Game", "Continue", "Settings", "Quit" };

    constexpr float kPanelL = 470.0f, kPanelR = 810.0f;
    constexpr float kPanelT = 300.0f;
    constexpr float kFirstRowY = kPanelT + 40.0f;
    constexpr float kRowH = 50.0f;
    constexpr float kPanelB = kFirstRowY + 4 * kRowH + 8.0f;

    const D3DCOLOR kPanel = D3DCOLOR_ARGB(250, 30, 26, 22);
    const D3DCOLOR kGold = D3DCOLOR_ARGB(255, 216, 184, 128);
    const D3DCOLOR kSelBar = D3DCOLOR_ARGB(255, 74, 60, 42);
    const D3DCOLOR kTextDim = D3DCOLOR_XRGB(170, 162, 150);
    const D3DCOLOR kTextOff = D3DCOLOR_XRGB(96, 92, 86);

    class MenuSelectState : public GameState {
    private:
        int sel;
        bool hasSave;

        IDirect3DTexture9* whiteTex;
        Font* titleFont;
        Font* rowFont;

        bool enterWasDown, escWasDown, upWasDown, downWasDown, mouseWasDown;

        // Clickable bounds of option row i
        static void RowBounds(int i, float& t, float& b) {
            const float y = kFirstRowY + i * kRowH;
            t = y - 12.0f;
            b = y + kRowH - 20.0f;
        }

        void Activate(int option, GameContext& context, GameStateManager& manager) {
            if (option == OPT_CONTINUE && !hasSave) return;
            if (context.sound != NULL) context.sound->PlaySfx("click");
            switch (option) {
            case OPT_NEW:      StartNewRun(context, manager); break;
            case OPT_CONTINUE: ContinueRun(context, manager); break;
            case OPT_SETTINGS: manager.Push(CreateSettingsState(this)); break;
            case OPT_QUIT:     PostQuitMessage(0); break;
            }
        }

        void StartNewRun(GameContext& context, GameStateManager& manager) {
            save::ClearProgress();
            ResetRunProgress(context);
            manager.ClearAndPush(CreateForestState());
            if (!s_forestIntroShown && !Cheats::enabled) {
                manager.Push(CreateForestIntroPopup());
                s_forestIntroShown = true;
            }
        }

        void ContinueRun(GameContext& context, GameStateManager& manager) {
            const save::Progress p = save::LoadProgress();
            if (!p.valid) return;

            ResetRunProgress(context);   // Clean slate, then layer the save on top
            if (context.playerStats != NULL) context.playerStats->SetLevel(p.level);
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

            manager.ClearAndPush(CreateOverworldStateForMap((MapId)p.mapId));
        }

    public:
        MenuSelectState()
            : sel(0), hasSave(false), whiteTex(NULL),
              titleFont(NULL), rowFont(NULL),
              enterWasDown(true), escWasDown(true), upWasDown(false), downWasDown(false),
              mouseWasDown(true) {}

        ~MenuSelectState() override {
            if (whiteTex != NULL) whiteTex->Release();
            delete titleFont;
            delete rowFont;
        }

        void Initialize(GameContext& context) override {
            hasSave = save::HasProgress();
            sel = hasSave ? OPT_CONTINUE : OPT_NEW;

            // Whatever opened this screen (Enter or a click) may still be held
            enterWasDown = context.keys != NULL && (context.keys[DIK_RETURN] & 0x80) != 0;
            escWasDown = context.keys != NULL && (context.keys[DIK_ESCAPE] & 0x80) != 0;
            mouseWasDown = context.mouseLeftDown;

            whiteTex = ui::MakeWhiteTexture(context.device);
            // Same placement as the title screen (MainMenu.cpp)
            titleFont = new Font(context.device, 0.0f, 180.0f, 1280, 80, 48, "Arial");
            rowFont = new Font(context.device, 0.0f, 0.0f, 320, 40, 24, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.ClearAndPush(CreateMainMenuState());
                return;
            }

            if (JustPressed(k, DIK_UP, upWasDown))   sel = (sel + OPT_COUNT - 1) % OPT_COUNT;
            if (JustPressed(k, DIK_DOWN, downWasDown)) sel = (sel + 1) % OPT_COUNT;

            // Mouse: hover highlights a row, click activates it
            const float mx = context.mouseX, my = context.mouseY;
            const bool click = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;
            for (int i = 0; i < OPT_COUNT; ++i) {
                if (i == OPT_CONTINUE && !hasSave) continue;
                float rt, rb;
                RowBounds(i, rt, rb);
                if (!InRect(mx, my, kPanelL + 6.0f, rt, kPanelR - 6.0f, rb)) continue;
                sel = i;
                if (click) { Activate(i, context, manager); return; }
            }

            if (JustPressed(k, DIK_RETURN, enterWasDown)) Activate(sel, context, manager);
        }

        void Update(GameContext&, GameStateManager&) override {}

        void Render(GameContext& context) override {
            LPD3DXSPRITE b = context.spriteBrush;

            if (context.pochi != NULL) context.pochi->Draw(b);

            // Choice panel
            ui::FillRect(b, whiteTex, kPanelL, kPanelT, kPanelR - kPanelL, kPanelB - kPanelT, kPanel);
            const float bw = 3.0f;
            ui::FillRect(b, whiteTex, kPanelL, kPanelT, kPanelR - kPanelL, bw, kGold);
            ui::FillRect(b, whiteTex, kPanelL, kPanelB - bw, kPanelR - kPanelL, bw, kGold);
            ui::FillRect(b, whiteTex, kPanelL, kPanelT, bw, kPanelB - kPanelT, kGold);
            ui::FillRect(b, whiteTex, kPanelR - bw, kPanelT, bw, kPanelB - kPanelT, kGold);

            for (int i = 0; i < OPT_COUNT; ++i) {
                const float y = kFirstRowY + i * kRowH;
                if (i == sel) ui::FillRect(b, whiteTex, kPanelL + 14.0f, y - 8.0f,
                                           kPanelR - kPanelL - 28.0f, kRowH - 10.0f, kSelBar);
                D3DCOLOR c = (i == sel) ? kGold : kTextDim;
                if (i == OPT_CONTINUE && !hasSave) c = kTextOff;
                rowFont->Draw(kOptions[i], kPanelL + 44.0f, y, c, b);
            }

            // Title through the shared brush, drawn last
            titleFont->Draw("THE TOAST TREK", D3DCOLOR_XRGB(35, 35, 35), b);
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(245, 245, 245); }
    };

} // Namespace

std::unique_ptr<GameState> CreateMenuSelectState() {
    return std::make_unique<MenuSelectState>();
}
