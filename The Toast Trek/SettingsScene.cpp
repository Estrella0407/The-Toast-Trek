#include "SettingsScene.h"
#include "SaveGame.h"
#include "SoundManager.h"
#include "UiFill.h"
#include "Font.h"
#include "Keys.h"
#include <algorithm>
#include <string>

namespace {


    bool InRect(float x, float y, float l, float t, float r, float b) {
        return x >= l && x <= r && y >= t && y <= b;
    }

    constexpr float kPanelL = 360.0f, kPanelR = 920.0f;
    constexpr float kPanelT = 160.0f, kPanelB = 520.0f;
    constexpr float kBodyX = kPanelL + 48.0f;
    constexpr float kRowH = 56.0f;
    constexpr float kFirstRowY = kPanelT + 110.0f;
    constexpr float kMeterX = kBodyX + 150.0f;
    constexpr float kMeterW = 260.0f;
    constexpr float kMeterH = 16.0f;

    const D3DCOLOR kDim = D3DCOLOR_ARGB(200, 10, 10, 14);
    const D3DCOLOR kPanel = D3DCOLOR_ARGB(244, 30, 26, 22);
    const D3DCOLOR kGold = D3DCOLOR_ARGB(255, 216, 184, 128);
    const D3DCOLOR kSelBar = D3DCOLOR_ARGB(255, 70, 58, 40);
    const D3DCOLOR kText = D3DCOLOR_XRGB(236, 230, 220);
    const D3DCOLOR kTextDim = D3DCOLOR_XRGB(160, 150, 138);
    const D3DCOLOR kHeading = D3DCOLOR_XRGB(245, 226, 184);

    class SettingsScene : public GameScene {
    private:
        GameScene* backdrop;
        int sel;   // 0 Master, 1 Music, 2 SFX, 3 Mute

        IDirect3DTexture9* whiteTex;
        Font* titleFont;
        Font* rowFont;

        bool escWasDown, eWasDown, upWasDown, downWasDown, leftWasDown, rightWasDown, enterWasDown;
        bool mouseWasDown;

        void Fill(LPD3DXSPRITE b, float x, float y, float w, float h, D3DCOLOR c) {
            ui::FillRect(b, whiteTex, x, y, w, h, c);
        }

        void SetVolume(GameContext& context, int which, float value) {
            SoundManager* s = context.sound;
            if (s == NULL) return;
            value = std::clamp(value, 0.0f, 1.0f);
            if (which == 0) s->SetMasterVolume(value);
            else if (which == 1) s->SetMusicVolume(value);
            else if (which == 2) s->SetSFXVolume(value);
        }

        void Persist(GameContext& context) {
            SoundManager* s = context.sound;
            if (s == NULL) return;
            save::SaveSettings({ s->GetMasterVolume(), s->GetMusicVolume(),
                                 s->GetSFXVolume(), s->IsMuted() });
        }

        void Adjust(GameContext& context, int dir) {
            SoundManager* s = context.sound;
            if (s == NULL) return;
            const float step = 0.1f * dir;
            switch (sel) {
            case 0: s->SetMasterVolume(s->GetMasterVolume() + step); break;
            case 1: s->SetMusicVolume(s->GetMusicVolume() + step); break;
            case 2: s->SetSFXVolume(s->GetSFXVolume() + step); break;
            case 3: if (dir != 0) s->ToggleMute(); break;
            }
        }

    public:
        explicit SettingsScene(GameScene* under)
            : backdrop(under), sel(0), whiteTex(NULL),
              titleFont(NULL), rowFont(NULL),
              escWasDown(true), eWasDown(true), upWasDown(false), downWasDown(false),
              leftWasDown(false), rightWasDown(false), enterWasDown(false), mouseWasDown(true) {}

        ~SettingsScene() override {
            if (whiteTex != NULL) whiteTex->Release();
            delete titleFont;
            delete rowFont;
        }

        void Initialize(GameContext& context) override {
            sel = 0;
            escWasDown = KeyDown(context.keys, ESCAPE_KEY);
            eWasDown = KeyDown(context.keys, E_KEY);
            enterWasDown = KeyDown(context.keys, RETURN_KEY);
            mouseWasDown = context.mouseLeftDown;

            whiteTex = ui::MakeWhiteTexture(context.device);
            titleFont = new Font(context.device, 0.0f, 0.0f, 400, 40, 26, "Arial");
            rowFont = new Font(context.device, 0.0f, 0.0f, 400, 30, 20, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (JustPressed(k, ESCAPE_KEY, escWasDown) || JustPressed(k, E_KEY, eWasDown)) {
                Persist(context);
                manager.Pop();
                return;
            }
            if (JustPressed(k, UP_KEY, upWasDown))    sel = (sel + 3) % 4;
            if (JustPressed(k, DOWN_KEY, downWasDown)) sel = (sel + 1) % 4;
            if (JustPressed(k, LEFT_KEY, leftWasDown))  Adjust(context, -1);
            if (JustPressed(k, RIGHT_KEY, rightWasDown)) Adjust(context, +1);
            if (JustPressed(k, RETURN_KEY, enterWasDown) && sel == 3) Adjust(context, +1);

            // --- Mouse -------------------------------------------------
            const float mx = context.mouseX, my = context.mouseY;
            const bool click = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;

            for (int i = 0; i < 4; ++i) {
                const float y = kFirstRowY + i * kRowH;
                if (InRect(mx, my, kPanelL + 20.0f, y - 10.0f, kPanelR - 20.0f, y + kRowH - 12.0f))
                    sel = i;
            }
            // Click / drag on a volume bar sets that volume.
            for (int i = 0; i < 3; ++i) {
                const float y = kFirstRowY + i * kRowH;
                if (context.mouseLeftDown &&
                    InRect(mx, my, kMeterX, y - 2.0f, kMeterX + kMeterW, y + kMeterH + 14.0f)) {
                    sel = i;
                    SetVolume(context, i, (mx - kMeterX) / kMeterW);
                }
            }
            // Click the Mute row toggles it.
            if (click) {
                const float y = kFirstRowY + 3 * kRowH;
                if (InRect(mx, my, kPanelL + 20.0f, y - 10.0f, kPanelR - 20.0f, y + kRowH - 12.0f)) {
                    if (context.sound != NULL) context.sound->ToggleMute();
                }
            }
        }

        void Update(GameContext&, GameStateManager&) override {}

        void Render(GameContext& context) override {
            LPD3DXSPRITE b = context.spriteBrush;
            if (backdrop != NULL) backdrop->Render(context);

            Fill(b, 0.0f, 0.0f, 1280.0f, 720.0f, kDim);
            Fill(b, kPanelL, kPanelT, kPanelR - kPanelL, kPanelB - kPanelT, kPanel);
            // Gold border
            const float bw = 3.0f;
            Fill(b, kPanelL, kPanelT, kPanelR - kPanelL, bw, kGold);
            Fill(b, kPanelL, kPanelB - bw, kPanelR - kPanelL, bw, kGold);
            Fill(b, kPanelL, kPanelT, bw, kPanelB - kPanelT, kGold);
            Fill(b, kPanelR - bw, kPanelT, bw, kPanelB - kPanelT, kGold);

            titleFont->Draw("SOUND SETTINGS", kBodyX, kPanelT + 30.0f, kHeading, b);

            SoundManager* s = context.sound;
            const char* labels[3] = { "Master", "Music", "SFX" };
            const float vols[3] = {
                s ? s->GetMasterVolume() : 0.0f,
                s ? s->GetMusicVolume() : 0.0f,
                s ? s->GetSFXVolume() : 0.0f,
            };

            for (int i = 0; i < 3; ++i) {
                const float y = kFirstRowY + i * kRowH;
                if (i == sel) Fill(b, kPanelL + 20.0f, y - 8.0f, kPanelR - kPanelL - 40.0f, kRowH - 8.0f, kSelBar);
                rowFont->Draw(labels[i], kBodyX, y, kText, b);
                Fill(b, kMeterX, y + 6.0f, kMeterW, kMeterH, D3DCOLOR_ARGB(255, 16, 14, 12));
                Fill(b, kMeterX, y + 6.0f, kMeterW * std::clamp(vols[i], 0.0f, 1.0f), kMeterH, kGold);
                rowFont->Draw((std::to_string((int)(vols[i] * 100 + 0.5f)) + "%").c_str(),
                              kMeterX + kMeterW + 16.0f, y, kText, b);
            }

            const float my = kFirstRowY + 3 * kRowH;
            if (sel == 3) Fill(b, kPanelL + 20.0f, my - 8.0f, kPanelR - kPanelL - 40.0f, kRowH - 8.0f, kSelBar);
            rowFont->Draw("Mute", kBodyX, my, kText, b);
            rowFont->Draw(s && s->IsMuted() ? "ON" : "OFF", kMeterX, my, kText, b);
        }

        D3DCOLOR ClearColor() const override {
            return backdrop != NULL ? backdrop->ClearColor() : D3DCOLOR_XRGB(245, 245, 245);
        }
    };

} // namespace

std::unique_ptr<GameScene> CreateSettingsScene(GameScene* backdrop) {
    return std::make_unique<SettingsScene>(backdrop);
}
