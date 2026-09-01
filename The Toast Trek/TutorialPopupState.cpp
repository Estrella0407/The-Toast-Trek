#include "TutorialPopupState.h"
#include "Font.h"
#include "Sprite.h"
#include "TileMap.h"
#include "UiFill.h"
#include <d3dx9.h>
#include <dinput.h>
#include <string>
#include <vector>

namespace {

    // Edge-detect on a group of keys: true on the frame any of them goes
    // from up to down. Matches the JustPressed pattern used across the
    // other states, extended to "any of these keys".
    bool JustPressedAny(BYTE* keys, const int* codes, int count, bool& wasDown) {
        bool isDown = false;
        if (keys != NULL) {
            for (int i = 0; i < count; ++i) {
                if ((keys[codes[i]] & 0x80) != 0) { isDown = true; break; }
            }
        }
        const bool pressed = isDown && !wasDown;
        wasDown = isDown;
        return pressed;
    }

    struct TutorialPage {
        const char* heading;
        std::vector<const char*> lines;
    };

    // Authored, fixed-width lines (no runtime word-wrap - Font only draws a
    // single line per call). Keep each line short enough to sit inside the
    // panel at font size 21.
    std::vector<TutorialPage> BuildPages() {
        return {
            { "Lost in the Woods", {
                "Pochi wandered too far from home, and now every",
                "part of the forest looks the same.",
                "",
                "Help him find his way back." } },

            { "Moving Around", {
                "Use  W  A  S  D  to walk.",
                "",
                "(The arrow keys work too.)" } },

            { "Interacting", {
                "Press  F  to interact with the world.",
                "",
                "Walk up to something and an on-screen prompt",
                "will tell you when F does something - picking",
                "up an item, or starting a fight." } },

            { "Your Pack", {
                "Press  E  to open your pack.",
                "",
                "Check what Pochi is carrying and how he's",
                "holding up." } },

            { "Picking Things Up", {
                "Things are lying around the forest.",
                "",
                "Stand over one until its label appears, then",
                "press  F  to take it.",
                "",
                "Potions restore health; bones restore armor." } },

            { "Barred Paths", {
                "Some ways out are chained shut by an iron gate.",
                "",
                "A gate like that won't lift until every enemy in",
                "the area has been beaten - and they must be",
                "faced in order.",
                "",
                "Clear the room, the gate opens, the path is yours." } },

            { "Fights", {
                "When a fight starts, click a button:",
                "",
                "  FIGHT  - attack",
                "  ACT    - Act Cute / Roll on Ground / Bark",
                "  ITEM   - use something from your pack",
                "  MERCY  - slip away",
                "",
                "You can also press  1  2  3  4  for those.",
                "",
                "Press Enter to begin your walk home." } },
        };
    }

    class TutorialPopupState : public GameState {
    private:
        std::vector<TutorialPage> pages;
        int pageIndex;

        // Assets/UI/white.png (a 1x1 white image), drawn stretched through
        // the shared sprite brush for every solid rectangle. Using ID3DXLine
        // here instead corrupts the active ID3DXSprite batch and makes this
        // panel - and every font drawn after it - render invisibly.
        IDirect3DTexture9* whiteTex;

        Font* headingFont;
        Font* bodyFont;
        Font* footerFont;

        bool prevWasDown;
        bool nextWasDown;
        bool advanceWasDown;

        // Panel rectangle (centred in the 1280x720 back buffer).
        static constexpr float kPanelL = 220.0f;
        static constexpr float kPanelR = 1060.0f;
        static constexpr float kPanelT = 110.0f;
        static constexpr float kPanelB = 610.0f;
        static constexpr float kTextMargin = 44.0f;

        void FillRect(LPD3DXSPRITE brush, float x, float y, float w, float h, D3DCOLOR color) {
            if (brush == NULL || whiteTex == NULL) return;
            D3DXVECTOR2 scale(w, h);
            D3DXVECTOR2 translate(x, y);
            D3DXMATRIX transform;
            D3DXMatrixTransformation2D(&transform, NULL, 0.0f, &scale, NULL, 0.0f, &translate);
            brush->SetTransform(&transform);
            RECT src = { 0, 0, 1, 1 };
            brush->Draw(whiteTex, &src, NULL, NULL, color);
        }

    public:
        TutorialPopupState()
            : pageIndex(0), whiteTex(NULL), headingFont(NULL), bodyFont(NULL), footerFont(NULL),
              prevWasDown(false), nextWasDown(false), advanceWasDown(true) {}

        ~TutorialPopupState() override {
            if (whiteTex != NULL) whiteTex->Release();
            delete headingFont;
            delete bodyFont;
            delete footerFont;
        }

        void Initialize(GameContext& context) override {
            pages = BuildPages();
            pageIndex = 0;

            // The Enter that opened this popup (from the main menu) is still
            // held for the first frame or two - start "down" so it must be
            // released before it counts as an advance.
            prevWasDown = false;
            nextWasDown = false;
            advanceWasDown = true;

            whiteTex = ui::MakeWhiteTexture(context.device);

            const int innerWidth = (int)(kPanelR - kPanelL - 2.0f * kTextMargin);
            headingFont = new Font(context.device, kPanelL + kTextMargin, kPanelT + 34.0f,
                                   innerWidth, 44, 30, "Arial");
            // Tall rects so per-line Draw() never leaves rect.bottom < rect.top.
            bodyFont = new Font(context.device, kPanelL + kTextMargin, kPanelT + 110.0f,
                                innerWidth, 420, 21, "Arial");
            footerFont = new Font(context.device, kPanelL + kTextMargin, kPanelB - 46.0f,
                                  innerWidth, 40, 16, "Arial");
        }

        void HandleInput(GameContext& context, GameStateManager& manager) override {
            const int prevKeys[] = { DIK_A, DIK_LEFT };
            const int nextKeys[] = { DIK_D, DIK_RIGHT };
            const int advanceKeys[] = { DIK_RETURN, DIK_SPACE };

            const int lastPage = (int)pages.size() - 1;

            if (JustPressedAny(context.keys, prevKeys, 2, prevWasDown)) {
                if (pageIndex > 0) --pageIndex;
            }
            if (JustPressedAny(context.keys, nextKeys, 2, nextWasDown)) {
                if (pageIndex < lastPage) ++pageIndex;
            }
            if (JustPressedAny(context.keys, advanceKeys, 2, advanceWasDown)) {
                if (pageIndex < lastPage) ++pageIndex;
                else manager.Pop();
            }
        }

        void Update(GameContext&, GameStateManager&) override {
            // Nothing animates; the forest underneath is frozen by design.
        }

        void Render(GameContext& context) override {
            LPD3DXSPRITE brush = context.spriteBrush;

            // 1. Static snapshot of the forest, drawn the same way
            //    OverworldState draws it (Pochi between the map and its leaf
            //    canopy).
            const std::vector<std::string> leaf = { "Tree_Leaf" };
            if (context.forestMap != NULL) context.forestMap->DrawExcludingLayers(brush, leaf);
            if (context.pochi != NULL) context.pochi->Draw(brush);
            if (context.forestMap != NULL) context.forestMap->DrawOnlyLayers(brush, leaf);

            // 2. Dim the whole screen, then an opaque-ish panel with a thin
            //    border. All plain sprite-brush quads - no ID3DXLine.
            FillRect(brush, 0.0f, 0.0f, 1280.0f, 720.0f, D3DCOLOR_ARGB(200, 10, 10, 14));
            FillRect(brush, kPanelL, kPanelT, kPanelR - kPanelL, kPanelB - kPanelT,
                     D3DCOLOR_ARGB(245, 26, 22, 20));

            const D3DCOLOR gold = D3DCOLOR_ARGB(255, 216, 184, 128);
            const float bw = 3.0f;
            FillRect(brush, kPanelL, kPanelT, kPanelR - kPanelL, bw, gold);              // top
            FillRect(brush, kPanelL, kPanelB - bw, kPanelR - kPanelL, bw, gold);         // bottom
            FillRect(brush, kPanelL, kPanelT, bw, kPanelB - kPanelT, gold);              // left
            FillRect(brush, kPanelR - bw, kPanelT, bw, kPanelB - kPanelT, gold);         // right

            // Leave the brush transform clean for anything that draws after.
            D3DXMATRIX identity;
            D3DXMatrixIdentity(&identity);
            brush->SetTransform(&identity);

            // 3. Page contents - drawn into the shared, already-open batch
            //    (passing `brush`) so the glyphs actually land.
            const TutorialPage& page = pages[pageIndex];
            if (headingFont != NULL) {
                headingFont->Draw(page.heading, kPanelL + kTextMargin, kPanelT + 34.0f,
                                  D3DCOLOR_XRGB(245, 226, 184), brush);
            }
            if (bodyFont != NULL) {
                float y = kPanelT + 110.0f;
                for (const char* line : page.lines) {
                    bodyFont->Draw(line, kPanelL + kTextMargin, y,
                                   D3DCOLOR_XRGB(232, 230, 226), brush);
                    y += 30.0f;
                }
            }

            // 4. Footer: navigation hint + page counter.
            if (footerFont != NULL) {
                const int lastPage = (int)pages.size() - 1;
                std::string footer = "A / Left        Page " + std::to_string(pageIndex + 1) +
                                     " of " + std::to_string((int)pages.size());
                if (pageIndex < lastPage) footer += "        D / Right        Enter: Next";
                else footer += "        Enter: Start";
                footerFont->Draw(footer.c_str(), kPanelL + kTextMargin, kPanelB - 46.0f,
                                 D3DCOLOR_XRGB(170, 168, 164), brush);
            }
        }

        D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(0, 0, 0); }
    };

} // namespace

std::unique_ptr<GameState> CreateForestIntroPopup() {
    return std::make_unique<TutorialPopupState>();
}
