#include "UnifiedMenu.h"
#include "GameStateManager.h"
#include "MainMenuScene.h"
#include "UiFill.h"
#include "Font.h"
#include "Inventory.h"
#include "Pochi.h"
#include "SoundManager.h"
#include "SaveGame.h"
#include "Keys.h"
#include <algorithm>
#include <string>

namespace {


    //hit-testing for UI elements
    bool IsPointInRect(float x, float y, float left, float top, float right, float bottom) {
        return x >= left && x <= right && y >= top && y <= bottom;
    }

    //UI tabs
    enum MenuTab { TAB_INVENTORY = 0, TAB_STATUS, TAB_SETTINGS, TAB_COUNT };
    const char* TAB_NAMES[TAB_COUNT] = { "INVENTORY", "STATUS", "SETTINGS" };

    //UI constants layout
    constexpr float panelLeft = 200.0f;
    constexpr float panelRight = 1080.0f;
    constexpr float panelTop = 80.0f;
    constexpr float panelBottom = 640.0f;

    constexpr float tabAreaLeft = panelLeft + 24.0f;
    constexpr float tabAreaWidth = (panelRight - panelLeft) - 48.0f;
    constexpr float tabTop = panelTop + 56.0f;
    constexpr float tabHeight = 40.0f;
    constexpr float dividerY = tabTop + tabHeight + 10.0f;

    constexpr float bodyTextX = panelLeft + 48.0f;
    constexpr float headingY = dividerY + 18.0f;
    constexpr float bodyY = dividerY + 62.0f;
    constexpr float rowHeight = 46.0f;

    constexpr float meterX = bodyTextX + 200.0f;
    constexpr float meterWidth = 300.0f;
    constexpr float meterHeight = 16.0f;

    //UI colour palette
    const D3DCOLOR dimBackground = D3DCOLOR_ARGB(200, 10, 10, 14);
    const D3DCOLOR panelColor = D3DCOLOR_ARGB(244, 30, 26, 22);
    const D3DCOLOR goldBorder = D3DCOLOR_ARGB(255, 216, 184, 128);
    const D3DCOLOR tabIdle = D3DCOLOR_ARGB(255, 46, 40, 34);
    const D3DCOLOR tabHover = D3DCOLOR_ARGB(255, 74, 62, 46);
    const D3DCOLOR tabActive = D3DCOLOR_ARGB(255, 120, 92, 52);
    const D3DCOLOR selectionBar = D3DCOLOR_ARGB(255, 70, 58, 40);
    const D3DCOLOR textColor = D3DCOLOR_XRGB(236, 230, 220);
    const D3DCOLOR textDim = D3DCOLOR_XRGB(160, 150, 138);
    const D3DCOLOR headingColor = D3DCOLOR_XRGB(245, 226, 184);

    //exit button
    constexpr float exitButtonRight = panelRight - 40.0f;
    constexpr float exitButtonLeft = exitButtonRight - 260.0f;
    constexpr float exitButtonTop = panelBottom - 76.0f;
    constexpr float exitButtonBottom = panelBottom - 38.0f;
    const D3DCOLOR exitButtonIdle = D3DCOLOR_ARGB(255, 58, 42, 36);
    const D3DCOLOR exitButtonHover = D3DCOLOR_ARGB(255, 120, 60, 48);

    //UI data - items displayed in inventory
    struct InventoryItem {
        ItemType type;
        const char* name;
        const char* description;
    };

    const InventoryItem items[3] = {
        { ItemType::HealthPotion, "Health Potion", "Restores 3 health." },
        { ItemType::Bone,         "Bone",          "Restores 2 armor." },
        { ItemType::Toast,        "Toast",         "Fully restores health and armor." },
    };

    float GetTabWidth() { return tabAreaWidth / (float)TAB_COUNT; }
    float GetRowTop(int index) { return bodyY + index * rowHeight - 8.0f; }
    float GetRowBottom(int index) { return GetRowTop(index) + rowHeight - 4.0f; }

    class UnifiedMenuScene : public GameScene {
    private:
        GameScene* backdrop;
        int tab;
        int sel;

        IDirect3DTexture9* whiteTexture;
        Font* titleFont;
        Font* tabFont;
        Font* headingFont;
        Font* bodyFont;
        Font* hintFont;

        bool eKeyWasDown;
        bool escapeKeyWasDown;
        bool aKeyWasDown;
        bool dKeyWasDown;
        bool qKeyWasDown;
        bool leftKeyWasDown;
        bool rightKeyWasDown;
        bool upKeyWasDown;
        bool downKeyWasDown;
        bool enterKeyWasDown;
        bool mouseWasDown;
        float previousMouseX;
        float previousMouseY;

        //sound feedback for UI interactions
        void PlayUISound(GameContext& context) {
            if (context.sound != NULL) {
                context.sound->PlaySfx("click", 0.5f, 1.0f);
            }
        }

        void SaveSettingsToFile(GameContext& context) {
            if (context.sound == NULL) return;
            save::SaveSettings({ context.sound->GetMasterVolume(),
                                 context.sound->GetMusicVolume(),
                                 context.sound->GetSFXVolume(),
                                 context.sound->IsMuted() });
        }

        // Save where we are, then go back to the title screen.
        void ExitToMainMenu(GameContext& context, GameStateManager& manager) {
            PersistSettings(context);
            SaveCurrentRun(context);   // so "Continue" resumes right here
            manager.ClearAndPush(CreateMainMenuScene());
        }

        int GetRowCount() const {
            switch (currentTab) {
            case TAB_INVENTORY: return 3;
            case TAB_SETTINGS:  return 4;
            default:            return 0;
            }
        }

        //filled rectangles and borders
        void DrawFilledRect(LPD3DXSPRITE sprite, float x, float y, float width, float height, D3DCOLOR color) {
            ui::FillRect(sprite, whiteTexture, x, y, width, height, color);
        }

        void DrawBorder(LPD3DXSPRITE sprite, float left, float top, float right, float bottom, float thickness, D3DCOLOR color) {
            DrawFilledRect(sprite, left, top, right - left, thickness, color);
            DrawFilledRect(sprite, left, bottom - thickness, right - left, thickness, color);
            DrawFilledRect(sprite, left, top, thickness, bottom - top, color);
            DrawFilledRect(sprite, right - thickness, top, thickness, bottom - top, color);
        }

        //item usage
        void UseSelectedItem(GameContext& context) {
            if (context.inventory == NULL || context.pochi == NULL) return;
            const ItemType type = kItems[sel].type;
            if (context.inventory->GetCount(type) <= 0) return;
            if (!context.inventory->Consume(type)) return;

            if (context.sound != NULL) {
                context.sound->PlaySfx("click", 0.7f, 1.2f);
            }

            if (type == ItemType::HealthPotion) {
                context.pochi->Heal(3);
            }
            else if (type == ItemType::Bone) {
                context.pochi->RecoverArmor(2);
            }
            else {
                context.pochi->Heal(context.pochi->GetMaxHealth());
                context.pochi->RecoverArmor(context.pochi->GetMaxArmor());
            }
        }

        //control volume
        void AdjustVolume(GameContext& context, int direction) {
            if (context.sound == NULL) return;
            const float step = 0.1f * dir;
            SoundManager* s = context.sound;
            switch (sel) {
            case 0: s->SetMasterVolume(s->GetMasterVolume() + step); break;
            case 1: s->SetMusicVolume(s->GetMusicVolume() + step); break;
            case 2: s->SetSFXVolume(s->GetSFXVolume() + step); break;
            case 3: if (dir != 0) s->ToggleMute(); break;
            }

            switch (selectedRow) {
                case 0: 
                    soundManager->SetMasterVolume(soundManager->GetMasterVolume() + stepSize); 
                    break;

                case 1: 
                    soundManager->SetMusicVolume(soundManager->GetMusicVolume() + stepSize); 
                    break;

                case 2: 
                    soundManager->SetSFXVolume(soundManager->GetSFXVolume() + stepSize);
                    break;

                default: 
                    if (direction != 0) soundManager->ToggleMute(); 
                    break;

            }
        }

        void SetVolumeValue(GameContext& context, int volumeIndex, float value) {
            if (context.sound == NULL) return;
            value = std::clamp(value, 0.0f, 1.0f);

            switch (volumeIndex) {
                case 0:
                    context.sound->SetMasterVolume(value);
                    break;

                case 1:
                    context.sound->SetMusicVolume(value);
                    break;

                case 2:
                    context.sound->SetSFXVolume(value);
                    break;

                default:
                    break;

            }

        }

        void RenderInventoryTab(LPD3DXSPRITE sprite, GameContext& context) {
            headingFont->Draw("Items Pochi is carrying", bodyTextX, headingY, headingColor, sprite);

            for (int i = 0; i < 3; ++i) {
                const float y = bodyY + i * rowHeight;

                if (i == selectedRow) {
                    DrawFilledRect(sprite, panelLeft + 24.0f, GetRowTop(i),
                        panelRight - panelLeft - 48.0f, rowHeight - 4.0f, selectionBar);
                }

                const int count = context.inventory ? context.inventory->GetCount(items[i].type) : 0;
                std::string itemLine = std::string(items[i].name) + "   x" + std::to_string(count);

                bodyFont->Draw(itemLine.c_str(), bodyTextX, y,
                    count > 0 ? textColor : textDim, sprite);
                bodyFont->Draw(items[i].description, bodyTextX + 320.0f, y, textDim, sprite);
            }

            hintFont->Draw("Enter / click: use the selected item",
                bodyTextX, bodyY + 3 * rowHeight + 12.0f, textDim, sprite);
        }

        void RenderStatus(LPD3DXSPRITE b, GameContext& context) {
            headFont->Draw("Pochi", kBodyX, kHeadingY, kHeading, b);
            const Pochi* p = context.pochi;
            auto Line = [&](const char* label, const std::string& val, int row) {
                bodyFont->Draw(label, kBodyX, kBodyY + row * kRowH, kTextDim, b);
                bodyFont->Draw(val.c_str(), kBodyX + 220.0f, kBodyY + row * kRowH, kText, b);
                };

            if (player != NULL) {
                DrawStatLine("Level", std::to_string(player->GetLevel()), 0);
                DrawStatLine("Health", std::to_string(player->GetHealth()) + " / " + std::to_string(player->GetMaxHealth()), 1);
                DrawStatLine("Armor", std::to_string(player->GetArmor()) + " / " + std::to_string(player->GetMaxArmor()), 2);
                DrawStatLine("Attack", std::to_string(player->GetAttackDamage()), 3);
            }

            bodyFont->Draw("Goal: help Pochi find his way back home.",
                bodyTextX, bodyY + 5 * rowHeight, textDim, sprite);
        }

        void RenderSettings(LPD3DXSPRITE b, GameContext& context) {
            headFont->Draw("Sound", kBodyX, kHeadingY, kHeading, b);
            SoundManager* s = context.sound;
            if (s == NULL) {
                bodyFont->Draw("Audio unavailable.", kBodyX, kBodyY, kTextDim, b);
                return;
            }

            const char* volumeLabels[3] = { "Master", "Music", "SFX" };
            const float volumes[3] = {
                soundManager->GetMasterVolume(),
                soundManager->GetMusicVolume(),
                soundManager->GetSFXVolume()
            };

            for (int i = 0; i < 3; ++i) {
                const float y = bodyY + i * rowHeight;

                if (i == selectedRow) {
                    DrawFilledRect(sprite, panelLeft + 24.0f, GetRowTop(i),
                        panelRight - panelLeft - 48.0f, rowHeight - 4.0f, selectionBar);
                }

                bodyFont->Draw(volumeLabels[i], bodyTextX, y, textColor, sprite);

                DrawFilledRect(sprite, meterX, y + 6.0f, meterWidth, meterHeight,
                    D3DCOLOR_ARGB(255, 16, 14, 12));

                DrawFilledRect(sprite, meterX, y + 6.0f,
                    meterWidth * std::clamp(volumes[i], 0.0f, 1.0f), meterHeight, goldBorder);

                std::string percentageText = std::to_string((int)(volumes[i] * 100 + 0.5f)) + "%";
                bodyFont->Draw(percentageText.c_str(), meterX + meterWidth + 16.0f, y, textColor, sprite);
            }

            const float muteY = bodyY + 3 * rowHeight;
            if (selectedRow == 3) {
                DrawFilledRect(sprite, panelLeft + 24.0f, GetRowTop(3),
                    panelRight - panelLeft - 48.0f, rowHeight - 4.0f, selectionBar);
            }

            bodyFont->Draw("Mute", bodyTextX, muteY, textColor, sprite);
            bodyFont->Draw(soundManager->IsMuted() ? "ON" : "OFF", meterX, muteY, textColor, sprite);

            hintFont->Draw("Left / Right or drag the bar: adjust    Enter / click: toggle mute",
                bodyTextX, bodyY + 4 * rowHeight + 12.0f, textDim, sprite);
        }

        //nouse input handling
        void HandleMouseInput(GameContext& context, GameStateManager& manager) {
            const float mouseX = context.mouseX;
            const float mouseY = context.mouseY;
            const bool mouseMoved = (mouseX != previousMouseX || mouseY != previousMouseY);

            previousMouseX = mouseX;
            previousMouseY = mouseY;

            const bool mouseClicked = context.mouseLeftDown && !mouseWasDown;
            mouseWasDown = context.mouseLeftDown;

            if (mouseClicked && IsPointInRect(mouseX, mouseY, exitButtonLeft, exitButtonTop,
                exitButtonRight, exitButtonBottom)) {
                ReturnToMainMenu(context, manager);
                return;
            }

            for (int i = 0; i < TAB_COUNT; ++i) {
                const float tabLeft = tabAreaLeft + i * GetTabWidth();
                if (IsPointInRect(mouseX, mouseY, tabLeft, tabTop, tabLeft + GetTabWidth(), tabTop + tabHeight)) {
                    if (mouseClicked && currentTab != i) {
                        currentTab = i;
                        selectedRow = 0;
                        PlayUISound(context);
                    }
                }
            }

            const int rowCount = GetRowCount();
            for (int i = 0; i < rowCount; ++i) {
                if (!IsPointInRect(mouseX, mouseY, panelLeft + 24.0f, GetRowTop(i),
                    panelRight - 24.0f, GetRowBottom(i))) continue;

                if (mouseMoved) selectedRow = i;
                if (!mouseClicked) continue;

                if (currentTab == TAB_INVENTORY) {
                    selectedRow = i;
                    UseSelectedItem(context);
                }
                else if (currentTab == TAB_SETTINGS) {
                    selectedRow = i;
                    if (i <= 2) {
                        const float y = bodyY + i * rowHeight;
                        if (IsPointInRect(mouseX, mouseY, meterX, y, meterX + meterWidth, y + meterHeight + 8.0f)) {
                            SetVolumeValue(context, i, (mouseX - meterX) / meterWidth);
                        }
                    }
                    else if (context.sound != NULL) {
                        context.sound->ToggleMute();
                        PlayUISound(context);
                    }
                }
            }

            if (context.mouseLeftDown && currentTab == TAB_SETTINGS && selectedRow <= 2) {
                const float y = bodyY + selectedRow * rowHeight;
                if (IsPointInRect(mouseX, mouseY, meterX, y - 4.0f, meterX + meterWidth, y + meterHeight + 10.0f)) {
                    SetVolumeValue(context, selectedRow, (mouseX - meterX) / meterWidth);
                }
            }
        }

    public:
        explicit UnifiedMenuScene(GameScene* under)
            : backdrop(under), tab(0), sel(0), whiteTex(NULL),
              titleFont(NULL), tabFont(NULL), headFont(NULL), bodyFont(NULL), hintFont(NULL),
              eWasDown(true), escWasDown(false), aWasDown(false), dWasDown(false), qWasDown(false),
              leftWasDown(false), rightWasDown(false), upWasDown(false), downWasDown(false),
              enterWasDown(false), mouseWasDown(true), prevMouseX(-1.0f), prevMouseY(-1.0f) {}

        ~UnifiedMenuScene() override {
            if (whiteTex != NULL) whiteTex->Release();
            delete titleFont;
            delete tabFont;
            delete headingFont;
            delete bodyFont;
            delete hintFont;
        }

        void Initialize(GameContext& context) override {
            currentTab = 0;
            selectedRow = 0;
            eKeyWasDown = true;
            mouseWasDown = true;
            escapeKeyWasDown = aKeyWasDown = dKeyWasDown = qKeyWasDown = false;
            leftKeyWasDown = rightKeyWasDown = upKeyWasDown = downKeyWasDown = enterKeyWasDown = false;
            previousMouseX = context.mouseX;
            previousMouseY = context.mouseY;

            whiteTexture = ui::MakeWhiteTexture(context.device);
            titleFont = new Font(context.device, 0.0f, 0.0f, 400, 40, 30, "Arial");
            tabFont = new Font(context.device, 0.0f, 0.0f, 300, 30, 20, "Arial");
            headingFont = new Font(context.device, 0.0f, 0.0f, 600, 30, 22, "Arial");
            bodyFont = new Font(context.device, 0.0f, 0.0f, 700, 30, 19, "Arial");
            hintFont = new Font(context.device, 0.0f, 0.0f, 900, 28, 16, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            BYTE* keys = context.keys;

            if (JustPressed(k, E_KEY, eWasDown) || JustPressed(k, ESCAPE_KEY, escWasDown)) {
                PersistSettings(context);
                manager.Pop();
                return;
            }
            if (JustPressed(k, Q_KEY, qWasDown)) {
                ExitToMainMenu(context, manager);
                return;
            }

            if (JustPressed(keys, DIK_A, aKeyWasDown)) {
                currentTab = (currentTab - 1 + TAB_COUNT) % TAB_COUNT;
                selectedRow = 0;
                PlayUISound(context);
            }

            if (JustPressed(keys, DIK_D, dKeyWasDown)) {
                currentTab = (currentTab + 1) % TAB_COUNT;
                selectedRow = 0;
                PlayUISound(context);
            }

            if (JustPressed(k, A_KEY, aWasDown)) { tab = (tab - 1 + TAB_COUNT) % TAB_COUNT; sel = 0; }
            if (JustPressed(k, D_KEY, dWasDown)) { tab = (tab + 1) % TAB_COUNT; sel = 0; }

            const bool onMeter = (tab == TAB_SETTINGS && sel <= 2);
            if (JustPressed(k, LEFT_KEY, leftWasDown)) {
                if (onMeter) NudgeVolume(context, -1);
                else { tab = (tab - 1 + TAB_COUNT) % TAB_COUNT; sel = 0; }
            }
            if (JustPressed(k, RIGHT_KEY, rightWasDown)) {
                if (onMeter) NudgeVolume(context, +1);
                else { tab = (tab + 1) % TAB_COUNT; sel = 0; }
            }

            const int rows = RowCount();
            if (rows > 0) {
                if (JustPressed(k, UP_KEY, upWasDown))     sel = (sel - 1 + rows) % rows;
                if (JustPressed(k, DOWN_KEY, downWasDown)) sel = (sel + 1) % rows;
            }
            else {
                upWasDown = KeyDown(k, UP_KEY);
                downWasDown = KeyDown(k, DOWN_KEY);
            }

            if (JustPressed(k, RETURN_KEY, enterWasDown)) {
                if (tab == TAB_INVENTORY) UseSelectedItem(context);
                else if (tab == TAB_SETTINGS && sel == 3) NudgeVolume(context, 1);
            }

            HandleMouseInput(context, manager);
        }

        void Update(GameContext&, GameStateManager&) override {}

        void Render(GameContext& context) override {
            LPD3DXSPRITE sprite = context.spriteBrush;

            if (backdrop != NULL) backdrop->Render(context);

            DrawFilledRect(sprite, 0.0f, 0.0f, 1280.0f, 720.0f, dimBackground);
            DrawFilledRect(sprite, panelLeft, panelTop, panelRight - panelLeft, panelBottom - panelTop, panelColor);
            DrawBorder(sprite, panelLeft, panelTop, panelRight, panelBottom, 3.0f, goldBorder);

            titleFont->Draw("MENU", panelLeft + 40.0f, panelTop + 14.0f, headingColor, sprite);

            const float tabWidth = GetTabWidth();
            const float mouseX = context.mouseX;
            const float mouseY = context.mouseY;

            for (int i = 0; i < TAB_COUNT; ++i) {
                const float tabLeft = tabAreaLeft + i * tabWidth;
                const bool isHovering = IsPointInRect(mouseX, mouseY, tabLeft, tabTop,
                    tabLeft + tabWidth, tabTop + tabHeight);

                DrawFilledRect(sprite, tabLeft + 3.0f, tabTop, tabWidth - 6.0f, tabHeight,
                    (i == currentTab) ? tabActive : (isHovering ? tabHover : tabIdle));

                tabFont->Draw(TAB_NAMES[i], tabLeft + 28.0f, tabTop + 8.0f,
                    (i == currentTab) ? headingColor : textDim, sprite);
            }

            DrawFilledRect(sprite, tabAreaLeft, dividerY, tabAreaWidth, 2.0f, goldBorder);

            if (currentTab == TAB_INVENTORY) {
                RenderInventoryTab(sprite, context);
            }
            else if (currentTab == TAB_STATUS) {
                RenderStatusTab(sprite, context);
            }
            else {
                RenderSettingsTab(sprite, context);
            }

            const bool isExitHovering = IsPointInRect(mouseX, mouseY, exitButtonLeft, exitButtonTop,
                exitButtonRight, exitButtonBottom);

            DrawFilledRect(sprite, exitButtonLeft, exitButtonTop,
                exitButtonRight - exitButtonLeft, exitButtonBottom - exitButtonTop,
                isExitHovering ? exitButtonHover : exitButtonIdle);

            DrawBorder(sprite, exitButtonLeft, exitButtonTop, exitButtonRight, exitButtonBottom, 2.0f, goldBorder);

            tabFont->Draw("EXIT TO MAIN MENU", exitButtonLeft + 24.0f, exitButtonTop + 8.0f,
                isExitHovering ? headingColor : textColor, sprite);

            hintFont->Draw("A / D: Tabs    Up / Down: Select    Enter: Use    Q: Main Menu    E / Esc: Close",
                panelLeft + 40.0f, panelBottom - 32.0f, textDim, sprite);
        }

        D3DCOLOR ClearColor() const override {
            return backdrop != NULL ? backdrop->ClearColor() : D3DCOLOR_XRGB(0, 0, 0);
        }
    };

} // Namespace

std::unique_ptr<GameScene> CreateUnifiedMenuScene(GameScene* backdrop) {
    return std::make_unique<UnifiedMenuScene>(backdrop);
}
