#include "BallPitScene.h"
#include "GameStateManager.h"
#include "Ball.h"
#include "Button.h"
#include "PhysicsManager.h"
#include "UiFill.h"
#include "SoundManager.h"
#include <dinput.h>
#include <cmath>
#include <memory>

namespace {

// Bounds the balls are clamped to (invisible - this screen is deliberately bare:
// just the two balls and a Back button)
constexpr float kL = 70.0f, kT = 150.0f, kR = 1210.0f, kB = 690.0f;

// One fixed simulation tick (the game loop is frame-locked ~60 fps, so
// speeds/forces here are "per tick", not per second)
constexpr float kDt          = 1.0f;

constexpr float kThrust      = 0.85f;   // steering force while a key is held (a = F / mass)
constexpr float kMaxSpeed    = 12.0f;   // velocity cap, px per tick
constexpr float kDrag        = 0.990f;  // per-tick velocity damping (friction)
constexpr float kWallBounce  = 0.92f;   // restitution against the walls
constexpr float kBallBounce  = 0.98f;   // restitution ball-to-ball

constexpr float kMassHeavy   = 3.0f;
constexpr float kMassLight   = 1.0f;

const D3DCOLOR kTintA = D3DCOLOR_XRGB(150, 200, 255);   // heavy ball (WASD)
const D3DCOLOR kTintB = D3DCOLOR_XRGB(255, 190, 120);   // light ball (arrows)

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
        if (ballTex) ballTex->Release();
    }

    void Initialize(GameContext& context) override {
        ballTex = ui::LoadTexture(context.device, "Assets/Characters/football.png", 1330, 1183);

        // Heavy + big vs light + small, so the mass term shows in both the
        // steering (F = m a) and the collision response
        a = std::make_unique<Ball>(ballTex, 360.0f, 420.0f, 54.0f, kMassHeavy, kMaxSpeed, kDrag);
        b = std::make_unique<Ball>(ballTex, 900.0f, 420.0f, 32.0f, kMassLight, kMaxSpeed, kDrag);

        backButton = std::make_unique<Button>(context.device, "Back", 1078, 90, 132, 36, 18);
        backButton->SetColours(D3DCOLOR_XRGB(40, 44, 56), D3DCOLOR_XRGB(72, 84, 108),
                               D3DCOLOR_XRGB(150, 160, 190), D3DCOLOR_XRGB(232, 234, 240));

        // Whatever opened this screen may still be held
        escWasDown = GameScene::IsKeyDown(context.keys, DIK_ESCAPE);
        eWasDown   = GameScene::IsKeyDown(context.keys, DIK_E);
    }

    void HandleInput(GameContext& context, GameStateManager& manager) override {
        const bool clickedBack =
            backButton && backButton->Update(context.mouseX, context.mouseY, context.mouseLeftDown);

        if (clickedBack ||
            JustPressed(context.keys, DIK_ESCAPE, escWasDown) ||
            JustPressed(context.keys, DIK_E, eWasDown)) {
            manager.Pop();               // back to the menu
        }
    }

    void Update(GameContext& context, GameStateManager&) override {
        const BYTE* k = context.keys;

        // Input -> force, then integrate the rigid body one tick
        a->ApplyThrust(ReadDir(k, DIK_W, DIK_S, DIK_A, DIK_D),            kThrust);
        b->ApplyThrust(ReadDir(k, DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT), kThrust);

        a->Step(kDt);
        b->Step(kDt);

        BounceWalls(*a);
        BounceWalls(*b);

        // Ball-to-ball: non-axis-aligned elastic resolution (impulse along the
        // contact normal, split by mass)
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

        a->Render(brush, kTintA);
        b->Render(brush, kTintB);

        if (backButton) backButton->Render(brush);
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
    std::unique_ptr<Button> backButton;
    IDirect3DTexture9* ballTex = nullptr;
    bool escWasDown = false;
    bool eWasDown = false;
    bool wasOverlapping = false;
};

} // namespace

std::unique_ptr<GameScene> CreateBallPitScene() {
    return std::make_unique<BallPitScene>();
}
