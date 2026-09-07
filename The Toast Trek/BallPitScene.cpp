#include "BallPitScene.h"
#include "GameStateManager.h"
#include "Ball.h"
#include "PhysicsManager.h"
#include "Font.h"
#include "UiFill.h"
#include "SoundManager.h"
#include <dinput.h>
#include <cmath>
#include <cstdio>
#include <memory>

namespace {

// Arena rectangle (leaves a strip at the top for the HUD text)
constexpr float kL = 70.0f, kT = 150.0f, kR = 1210.0f, kB = 690.0f;

// Per fixed 1/60 s step
constexpr float kAccel        = 0.55f;   // push per frame while a key is held
constexpr float kMaxSpeed     = 12.0f;
constexpr float kFriction      = 0.990f;  // gentle drag so bounces carry
constexpr float kWallBounce    = 0.92f;   // restitution against the walls
constexpr float kBallBounce    = 0.98f;   // restitution ball-to-ball

const D3DCOLOR kTintA = D3DCOLOR_XRGB(150, 200, 255);   // heavy ball (WASD)
const D3DCOLOR kTintB = D3DCOLOR_XRGB(255, 190, 120);   // light ball (arrows)
const D3DCOLOR kArenaFill = D3DCOLOR_ARGB(255, 18, 20, 28);
const D3DCOLOR kArenaEdge = D3DCOLOR_ARGB(255, 120, 130, 160);

float Length(const D3DXVECTOR2& v) { return sqrtf(v.x * v.x + v.y * v.y); }

D3DXVECTOR2 ReadDir(const BYTE* keys, int up, int down, int left, int right) {
    D3DXVECTOR2 d(0.0f, 0.0f);
    if (GameScene::IsKeyDown(keys, left))  d.x -= 1.0f;
    if (GameScene::IsKeyDown(keys, right)) d.x += 1.0f;
    if (GameScene::IsKeyDown(keys, up))    d.y -= 1.0f;
    if (GameScene::IsKeyDown(keys, down))  d.y += 1.0f;
    const float len = Length(d);
    if (len > 0.0f) d /= len;               // keep diagonals the same speed
    return d;
}

class BallPitScene : public GameScene {
public:
    ~BallPitScene() override {
        if (ballTex)  ballTex->Release();
        if (whiteTex) whiteTex->Release();
        delete hudFont;
    }

    void Initialize(GameContext& context) override {
        ballTex  = ui::LoadTexture(context.device, "Assets/Characters/football.png", 1330, 1183);
        whiteTex = ui::MakeWhiteTexture(context.device);
        hudFont  = new Font(context.device, 0.0f, 0.0f, 1280, 40, 20, "Arial");

        // Heavy + big vs light + small, so the mass term is visible
        a = std::make_unique<Ball>(ballTex, 360.0f, 420.0f, 54.0f, 4.0f);
        b = std::make_unique<Ball>(ballTex, 900.0f, 420.0f, 32.0f, 1.0f);

        // Whatever opened this screen may still be held
        escWasDown = GameScene::IsKeyDown(context.keys, DIK_ESCAPE);
        eWasDown   = GameScene::IsKeyDown(context.keys, DIK_E);
    }

    void HandleInput(GameContext& context, GameStateManager& manager) override {
        if (JustPressed(context.keys, DIK_ESCAPE, escWasDown) ||
            JustPressed(context.keys, DIK_E, eWasDown)) {
            manager.Pop();               // back to the menu
        }
    }

    void Update(GameContext& context, GameStateManager&) override {
        const BYTE* k = context.keys;

        a->Drive(ReadDir(k, DIK_W, DIK_S, DIK_A, DIK_D),           kAccel, kMaxSpeed);
        b->Drive(ReadDir(k, DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT), kAccel, kMaxSpeed);

        a->Step(kFriction);
        b->Step(kFriction);

        BounceWalls(*a);
        BounceWalls(*b);

        // Ball-to-ball: non-axis-aligned elastic resolution
        const bool overlapping = PhysicsManager::CirclesOverlap(
            a->GetPosition(), a->Radius(), b->GetPosition(), b->Radius());
        if (overlapping) {
            D3DXVECTOR2 pa = a->GetPosition(), pb = b->GetPosition();
            D3DXVECTOR2 va = a->Body().velocity, vb = b->Body().velocity;
            PhysicsManager::ResolveCircleCollision(
                pa, va, a->Mass(), a->Radius(),
                pb, vb, b->Mass(), b->Radius(), kBallBounce);
            a->SetPosition(pa.x, pa.y); a->Body().velocity = va;
            b->SetPosition(pb.x, pb.y); b->Body().velocity = vb;

            if (!wasOverlapping && context.sound) context.sound->PlaySfx("click");
        }
        wasOverlapping = overlapping;
    }

    void Render(GameContext& context) override {
        LPD3DXSPRITE brush = context.spriteBrush;

        // Arena
        ui::FillRect(brush, whiteTex, kL, kT, kR - kL, kB - kT, kArenaFill);
        const float bw = 3.0f;
        ui::FillRect(brush, whiteTex, kL, kT, kR - kL, bw, kArenaEdge);
        ui::FillRect(brush, whiteTex, kL, kB - bw, kR - kL, bw, kArenaEdge);
        ui::FillRect(brush, whiteTex, kL, kT, bw, kB - kT, kArenaEdge);
        ui::FillRect(brush, whiteTex, kR - bw, kT, bw, kB - kT, kArenaEdge);

        a->Render(brush, kTintA);
        b->Render(brush, kTintB);

        hudFont->Draw("PHYSICS DEMO - elastic collision resolved along the contact normal",
                      80.0f, 34.0f, D3DCOLOR_XRGB(230, 230, 235), brush);
        hudFont->Draw("WASD: heavy ball      Arrow keys: light ball      Esc / E: back to menu",
                      80.0f, 70.0f, D3DCOLOR_XRGB(170, 175, 185), brush);

        char buf[96];
        sprintf_s(buf, "heavy  mass %.0f   speed %.1f", a->Mass(), Length(a->Body().velocity));
        hudFont->Draw(buf, 80.0f, 104.0f, kTintA, brush);
        sprintf_s(buf, "light  mass %.0f   speed %.1f", b->Mass(), Length(b->Body().velocity));
        hudFont->Draw(buf, 470.0f, 104.0f, kTintB, brush);
    }

    D3DCOLOR ClearColor() const override { return D3DCOLOR_XRGB(24, 26, 34); }

private:
    void BounceWalls(Ball& ball) {
        D3DXVECTOR2 p = ball.GetPosition();
        D3DXVECTOR2 v = ball.Body().velocity;
        const float r = ball.Radius();
        if (p.x - r < kL) { p.x = kL + r; v.x = -v.x * kWallBounce; }
        if (p.x + r > kR) { p.x = kR - r; v.x = -v.x * kWallBounce; }
        if (p.y - r < kT) { p.y = kT + r; v.y = -v.y * kWallBounce; }
        if (p.y + r > kB) { p.y = kB - r; v.y = -v.y * kWallBounce; }
        ball.SetPosition(p.x, p.y);
        ball.Body().velocity = v;
    }

    std::unique_ptr<Ball> a;   // WASD, heavy
    std::unique_ptr<Ball> b;   // arrows, light
    IDirect3DTexture9* ballTex = nullptr;
    IDirect3DTexture9* whiteTex = nullptr;
    Font* hudFont = nullptr;
    bool escWasDown = false;
    bool eWasDown = false;
    bool wasOverlapping = false;
};

} // namespace

std::unique_ptr<GameScene> CreateBallPitScene() {
    return std::make_unique<BallPitScene>();
}
