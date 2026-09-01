#include "UnifiedMenu.h"
#include "UiFill.h"
#include "Font.h"
#include "Inventory.h"
#include "Pochi.h"
#include "SoundManage.h"
#include <dinput.h>
#include <algorithm>
#include <string>

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

    enum Tab { TAB_INVENTORY = 0, TAB_STATUS, TAB_SETTINGS, TAB_COUNT };
    const char* kTabNames[TAB_COUNT] = { "INVENTORY", "STATUS", "SETTINGS" };

    // Panel + layout.
    constexpr float kPanelL = 200.0f, kPanelR = 1080.0f;
    constexpr float kPanelT = 80.0f, kPanelB = 640.0f;
    constexpr float kTabAreaL = kPanelL + 24.0f;
    constexpr float kTabAreaW = (kPanelR - kPanelL) - 48.0f;
    constexpr float kTabT = kPanelT + 56.0f;
    constexpr float kTabH = 40.0f;
    constexpr float kDividerY = kTabT + kTabH + 10.0f;
    constexpr float kBodyX = kPanelL + 48.0f;
    constexpr float kHeadingY = kDividerY + 18.0f;
    constexpr float kBodyY = kDividerY + 62.0f;
    constexpr float kRowH = 46.0f;

    constexpr float kMeterX = kBodyX + 200.0f;
    constexpr float kMeterW = 300.0f;
    constexpr float kMeterH = 16.0f;

    const D3DCOLOR kDim = D3DCOLOR_ARGB(200, 10, 10, 14);
    const D3DCOLOR kPanel = D3DCOLOR_ARGB(244, 30, 26, 22);
    const D3DCOLOR kGold = D3DCOLOR_ARGB(255, 216, 184, 128);
    const D3DCOLOR kTabIdle = D3DCOLOR_ARGB(255, 46, 40, 34);
    const D3DCOLOR kTabHover = D3DCOLOR_ARGB(255, 74, 62, 46);
    const D3DCOLOR kTabActive = D3DCOLOR_ARGB(255, 120, 92, 52);
    const D3DCOLOR kSelBar = D3DCOLOR_ARGB(255, 70, 58, 40);
    const D3DCOLOR kText = D3DCOLOR_XRGB(236, 230, 220);
    const D3DCOLOR kTextDim = D3DCOLOR_XRGB(160, 150, 138);
    const D3DCOLOR kHeading = D3DCOLOR_XRGB(245, 226, 184);

    struct ItemRow { ItemType type; const char* name; const char* desc; };
    const ItemRow kItems[3] = {
        { ItemType::HealthPotion, "Health Potion", "Restores 3 health." },
        { ItemType::Bone,         "Bone",          "Restores 2 armor." },
        { ItemType::Toast,        "Toast",         "Fully restores health and armor." },
    };

    float TabWidth() { return kTabAreaW / (float)TAB_COUNT; }
    float RowTop(int i) { return kBodyY + i * kRowH - 8.0f; }
    float RowBottom(int i) { return RowTop(i) + kRowH - 4.0f; }

    class UnifiedMenuState : public GameState {
    private:
        GameState* backdrop;
        int tab;
        int sel;

        IDirect3DTexture9* whiteTex;
        Font* titleFont;
        Font* tabFont;
        Font* headFont;
        Font* bodyFont;
        Font* hintFont;

        bool eWasDown, escWasDown, aWasDown, dWasDown;
        bool leftWasDown, rightWasDown, upWasDown, downWasDown, enterWasDown;
        bool mouseWasDown;
        float prevMouseX, prevMouseY;

        int RowCount() const {
            switch (tab) {
            case TAB_INVENTORY: return 3;
            case TAB_SETTINGS:  return 4;   // master, music, sfx, mute
            default:            return 0;   // status: nothing selectable
            }
        }

        void Fill(LPD3DXSPRITE b, float x, float y, float w, float h, D3DCOLOR c) {
            ui::FillRect(b, whiteTex, x, y, w, h, c);
        }
        void Border(LPD3DXSPRITE b, float l, float t, float r, float bot, float px, D3DCOLOR c) {
            Fill(b, l, t, r - l, px, c);
            Fill(b, l, bot - px, r - l, px, c);
            Fill(b, l, t, px, bot - t, c);
            Fill(b, r - px, t, px, bot - t, c);
        }

        void UseSelectedItem(GameContext& context) {
            if (context.inventory == NULL || context.playerStats == NULL) return;
            const ItemType type = kItems[sel].type;
            if (context.inventory->GetCount(type) <= 0) return;
            if (!context.inventory->Consume(type)) return;

            if (type == ItemType::HealthPotion) {
                context.playerStats->Heal(3);
            }
            else if (type == ItemType::Bone) {
                context.playerStats->RecoverArmor(2);
            }
            else {
                context.playerStats->Heal(context.playerStats->GetMaxHealth());
                context.playerStats->RecoverArmor(context.playerStats->GetMaxArmor());
            }
            if (context.sound != NULL) context.sound->PlaySfx("click");
        }

        void NudgeVolume(GameContext& context, int dir) {
            if (context.sound == NULL) return;
            const float step = 0.1f * dir;
            SoundManage* s = context.sound;
            switch (sel) {
            case 0: s->SetMasterVolume(s->GetMasterVolume() + step); break;
            case 1: s->SetMusicVolume(s->GetMusicVolume() + step); break;
            case 2: s->SetSFXVolume(s->GetSFXVolume() + step); break;
            case 3: if (dir != 0) s->ToggleMute(); break;
            }
        }

        void SetVolume(GameContext& context, int which, float value) {
            if (context.sound == NULL) return;
            value = std::clamp(value, 0.0f, 1.0f);
            if (which == 0) context.sound->SetMasterVolume(value);
            else if (which == 1) context.sound->SetMusicVolume(value);
            else if (which == 2) context.sound->SetSFXVolume(value);
        }

        // --- rendering per tab ---------------------------------------
        void RenderInventory(LPD3DXSPRITE b, GameContext& context) {
            headFont->Draw("Items Pochi is carrying", kBodyX, kHeadingY, kHeading, b);
            for (int i = 0; i < 3; ++i) {
                const float y = kBodyY + i * kRowH;
                if (i == sel) Fill(b, kPanelL + 24.0f, RowTop(i), kPanelR - kPanelL - 48.0f, kRowH - 4.0f, kSelBar);
                const int count = context.inventory ? context.inventory->GetCount(kItems[i].type) : 0;
                std::string line = std::string(kItems[i].name) + "   x" + std::to_string(count);
                bodyFont->Draw(line.c_str(), kBodyX, y, count > 0 ? kText : kTextDim, b);
                bodyFont->Draw(kItems[i].desc, kBodyX + 320.0f, y, kTextDim, b);
            }
            hintFont->Draw("Enter / click: use the selected item",
                kBodyX, kBodyY + 3 * kRowH + 12.0f, kTextDim, b);
        }

        void RenderStatus(LPD3DXSPRITE b, GameContext& context) {
            headFont->Draw("Pochi", kBodyX, kHeadingY, kHeading, b);
            const Pochi* p = context.playerStats;
            auto line = [&](const char* label, const std::string& val, int row) {
                bodyFont->Draw(label, kBodyX, kBodyY + row * kRowH, kTextDim, b);
                bodyFont->Draw(val.c_str(), kBodyX + 220.0f, kBodyY + row * kRowH, kText, b);
                };
            if (p != NULL) {
                line("Level", std::to_string(p->GetLevel()), 0);
                line("Health", std::to_string(p->GetHealth()) + " / " + std::to_string(p->GetMaxHealth()), 1);
                line("Armor", std::to_string(p->GetArmor()) + " / " + std::to_string(p->GetMaxArmor()), 2);
                line("Attack", std::to_string(p->GetAttackDamage()), 3);
            }
            bodyFont->Draw("Goal: help Pochi find his way back home.",
                kBodyX, kBodyY + 5 * kRowH, kTextDim, b);
        }

        void RenderSettings(LPD3DXSPRITE b, GameContext& context) {
            headFont->Draw("Sound", kBodyX, kHeadingY, kHeading, b);
            SoundManage* s = context.sound;
            if (s == NULL) {
                bodyFont->Draw("Audio unavailable.", kBodyX, kBodyY, kTextDim, b);
                return;
            }
            const char* labels[3] = { "Master", "Music", "SFX" };
            const float vols[3] = { s->GetMasterVolume(), s->GetMusicVolume(), s->GetSFXVolume() };
            for (int i = 0; i < 3; ++i) {
                const float y = kBodyY + i * kRowH;
                if (i == sel) Fill(b, kPanelL + 24.0f, RowTop(i), kPanelR - kPanelL - 48.0f, kRowH - 4.0f, kSelBar);
                bodyFont->Draw(labels[i], kBodyX, y, kText, b);
                Fill(b, kMeterX, y + 6.0f, kMeterW, kMeterH, D3DCOLOR_ARGB(255, 16, 14, 12));
                Fill(b, kMeterX, y + 6.0f, kMeterW * std::clamp(vols[i], 0.0f, 1.0f), kMeterH, kGold);
                bodyFont->Draw((std::to_string((int)(vols[i] * 100 + 0.5f)) + "%").c_str(),
                    kMeterX + kMeterW + 16.0f, y, kText, b);
            }
            const float my = kBodyY + 3 * kRowH;
            if (sel == 3) Fill(b, kPanelL + 24.0f, RowTop(3), kPanelR - kPanelL - 48.0f, kRowH - 4.0f, kSelBar);
            bodyFont->Draw("Mute", kBodyX, my, kText, b);
            bodyFont->Draw(s->IsMuted() ? "ON" : "OFF", kMeterX, my, kText, b);
            hintFont->Draw("Left / Right or drag the bar: adjust    Enter / click: toggle mute",
                kBodyX, kBodyY + 4 * kRowH + 12.0f, kTextDim, b);
        }

        // --- mouse -------------------------------------------------------
        void HandleMouse(GameContext& context) {
            const float mx = context.mouseX, my = context.mouseY;
            const bool moved = (mx != prevMouseX || my != prevMouseY);
            prevMouseX = mx; prevMouseY = my;
            const bool click = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;

            // Tabs.
            for (int i = 0; i < TAB_COUNT; ++i) {
                const float l = kTabAreaL + i * TabWidth();
                if (InRect(mx, my, l, kTabT, l + TabWidth(), kTabT + kTabH)) {
                    if (click && tab != i) { tab = i; sel = 0; }
                }
            }

            const int rows = RowCount();
            for (int i = 0; i < rows; ++i) {
                if (!InRect(mx, my, kPanelL + 24.0f, RowTop(i), kPanelR - 24.0f, RowBottom(i))) continue;
                if (moved) sel = i;
                if (!click) continue;

                if (tab == TAB_INVENTORY) {
                    sel = i;
                    UseSelectedItem(context);
                }
                else if (tab == TAB_SETTINGS) {
                    sel = i;
                    if (i <= 2) {
                        const float y = kBodyY + i * kRowH;
                        if (InRect(mx, my, kMeterX, y, kMeterX + kMeterW, y + kMeterH + 8.0f)) {
                            SetVolume(context, i, (mx - kMeterX) / kMeterW);
                        }
                    }
                    else if (context.sound != NULL) {
                        context.sound->ToggleMute();
                    }
                }
            }

            // Drag inside a volume bar even without re-clicking on the row.
            if (context.mouseLeftDown && tab == TAB_SETTINGS && sel <= 2) {
                const float y = kBodyY + sel * kRowH;
                if (InRect(mx, my, kMeterX, y - 4.0f, kMeterX + kMeterW, y + kMeterH + 10.0f)) {
                    SetVolume(context, sel, (mx - kMeterX) / kMeterW);
                }
            }
        }

    public:
        explicit UnifiedMenuState(GameState* under)
            : backdrop(under), tab(0), sel(0), whiteTex(NULL),
              titleFont(NULL), tabFont(NULL), headFont(NULL), bodyFont(NULL), hintFont(NULL),
              eWasDown(true), escWasDown(false), aWasDown(false), dWasDown(false),
              leftWasDown(false), rightWasDown(false), upWasDown(false), downWasDown(false),
              enterWasDown(false), mouseWasDown(true), prevMouseX(-1.0f), prevMouseY(-1.0f) {}

        ~UnifiedMenuState() override {
            if (whiteTex != NULL) whiteTex->Release();
            delete titleFont;
            delete tabFont;
            delete headFont;
            delete bodyFont;
            delete hintFont;
        }

        void Initialize(GameContext& context) override {
            tab = 0; sel = 0;
            eWasDown = true;            // the E that opened the menu is still held
            mouseWasDown = true;        // and the mouse button might be too
            escWasDown = aWasDown = dWasDown = false;
            leftWasDown = rightWasDown = upWasDown = downWasDown = enterWasDown = false;
            prevMouseX = context.mouseX; prevMouseY = context.mouseY;

            whiteTex = ui::MakeWhiteTexture(context.device);
            titleFont = new Font(context.device, 0.0f, 0.0f, 400, 40, 30, "Arial");
            tabFont = new Font(context.device, 0.0f, 0.0f, 300, 30, 20, "Arial");
            headFont = new Font(context.device, 0.0f, 0.0f, 600, 30, 22, "Arial");
            bodyFont = new Font(context.device, 0.0f, 0.0f, 700, 30, 19, "Arial");
            hintFont = new Font(context.device, 0.0f, 0.0f, 900, 28, 16, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* k = context.keys;

            if (JustPressed(k, DIK_E, eWasDown) || JustPressed(k, DIK_ESCAPE, escWasDown)) {
                manager.Pop();
                return;
            }

            if (JustPressed(k, DIK_A, aWasDown)) { tab = (tab - 1 + TAB_COUNT) % TAB_COUNT; sel = 0; }
            if (JustPressed(k, DIK_D, dWasDown)) { tab = (tab + 1) % TAB_COUNT; sel = 0; }

            const bool onMeter = (tab == TAB_SETTINGS && sel <= 2);
            if (JustPressed(k, DIK_LEFT, leftWasDown)) {
                if (onMeter) NudgeVolume(context, -1);
                else { tab = (tab - 1 + TAB_COUNT) % TAB_COUNT; sel = 0; }
            }
            if (JustPressed(k, DIK_RIGHT, rightWasDown)) {
                if (onMeter) NudgeVolume(context, +1);
                else { tab = (tab + 1) % TAB_COUNT; sel = 0; }
            }

            const int rows = RowCount();
            if (rows > 0) {
                if (JustPressed(k, DIK_UP, upWasDown))     sel = (sel - 1 + rows) % rows;
                if (JustPressed(k, DIK_DOWN, downWasDown)) sel = (sel + 1) % rows;
            }
            else {
                upWasDown = k != NULL && (k[DIK_UP] & 0x80) != 0;
                downWasDown = k != NULL && (k[DIK_DOWN] & 0x80) != 0;
            }

            if (JustPressed(k, DIK_RETURN, enterWasDown)) {
                if (tab == TAB_INVENTORY) UseSelectedItem(context);
                else if (tab == TAB_SETTINGS && sel == 3) NudgeVolume(context, 1);
            }

            HandleMouse(context);
        }

        void Update(GameContext&, GameStateManager&) override {}

        void Render(GameContext& context) override {
            LPD3DXSPRITE b = context.spriteBrush;

            // The frozen world behind the menu.
            if (backdrop != NULL) backdrop->Render(context);

            Fill(b, 0.0f, 0.0f, 1280.0f, 720.0f, kDim);
            Fill(b, kPanelL, kPanelT, kPanelR - kPanelL, kPanelB - kPanelT, kPanel);
            Border(b, kPanelL, kPanelT, kPanelR, kPanelB, 3.0f, kGold);

            titleFont->Draw("MENU", kPanelL + 40.0f, kPanelT + 14.0f, kHeading, b);

            const float tabW = TabWidth();
            const float mx = context.mouseX, my = context.mouseY;
            for (int i = 0; i < TAB_COUNT; ++i) {
                const float tx = kTabAreaL + i * tabW;
                const bool hover = InRect(mx, my, tx, kTabT, tx + tabW, kTabT + kTabH);
                Fill(b, tx + 3.0f, kTabT, tabW - 6.0f, kTabH,
                     i == tab ? kTabActive : (hover ? kTabHover : kTabIdle));
                tabFont->Draw(kTabNames[i], tx + 28.0f, kTabT + 8.0f,
                              i == tab ? kHeading : kTextDim, b);
            }
            Fill(b, kTabAreaL, kDividerY, kTabAreaW, 2.0f, kGold);

            if (tab == TAB_INVENTORY) RenderInventory(b, context);
            else if (tab == TAB_STATUS) RenderStatus(b, context);
            else RenderSettings(b, context);

            hintFont->Draw("A / D: Tabs      Up / Down: Select      Enter: Use      E / Esc: Close",
                           kPanelL + 40.0f, kPanelB - 32.0f, kTextDim, b);
        }

        D3DCOLOR ClearColor() const override {
            return backdrop != NULL ? backdrop->ClearColor() : D3DCOLOR_XRGB(0, 0, 0);
        }
    };

} // namespace

std::unique_ptr<GameState> CreateUnifiedMenuState(GameState* backdrop) {
    return std::make_unique<UnifiedMenuState>(backdrop);
}
