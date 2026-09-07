#include "BattleScene.h"
#include "GameStateManager.h"
#include "GameOverScene.h"
#include "Battlefield.h"
#include "BattleUI.h"
#include "Cheats.h"
#include "Pochi.h"
#include "Enemy.h"
#include "SoundManager.h"
#include "Keys.h"
#include <memory>

// The turn-based boss fight. Private to this file - the rest of the game only
// ever calls CreateBattleScene(bossId).
class BattleScene : public GameScene {
private:
	std::unique_ptr<Battlefield> battlefield;
	std::unique_ptr<BattleUI> battleUI;
	Pochi* pochi;
	BossId bossId;

	enum BattlePhase {
		ENCOUNTER, PLAYER_TURN, ACT_MENU, ITEM_MENU, ACT_ANIMATION, ENEMY_HIT, ENEMY_ATTACK
	};

	bool showEncounterMessage;
	float enemyFlashTimer;
	int enemyHitFrames;
	bool actionKeyWasDown[4];
	bool actChoiceWasDown[3];
	bool itemChoiceWasDown[3];
	bool actChoiceUsed[3];	// After use, the selected button is gone
	bool cheatWinWasDown;	// Dev cheat: K ends the fight in a win
	int lastPochiHealth;	// Prev-frame heart + armour total -> a drop plays "hurt"

public:
	explicit BattleScene(BossId bossId) : bossId(bossId), battlefield(nullptr), battleUI(nullptr), phase(ENCOUNTER),
		showEncounterMessage(true), enemyFlashTimer(0.0f), enemyHitFrames(0),
		actionKeyWasDown{ false, false, false, false }, actChoiceWasDown{ false, false, false },
		itemChoiceWasDown{ false, false, false }, actChoiceUsed{ false, false, false },
		cheatWinWasDown(false), lastPochiHealth(0) {}

	~BattleScene() override;
	BattlePhase phase;

	void Initialize(GameContext& context) override;
	void HandleInput(GameContext& context, GameStateManager& manager) override;
	void Update(GameContext& context, GameStateManager& manager) override;
	void Render(GameContext& context) override;
	D3DCOLOR ClearColor() const override;
};

namespace {

const char* BossDisplayName(BossId id) {
    switch (id) {
    case BossId::SkullBones: return "Skullie";
    case BossId::Goblin:     return "the Goblin";
    case BossId::Maki:       return "Makima";
    case BossId::MrAndrew:   return "Mr Andrew";
    }
    return "the enemy";
}
}

// Defined here (not =default in the header) so Battlefield / BattleUI are
// complete types when the unique_ptr members are destroyed
BattleScene::~BattleScene() = default;

void BattleScene::Initialize(GameContext& context) {
	pochi = context.pochi;
	lastPochiHealth = (pochi != nullptr) ? (pochi->GetHealth() + pochi->GetArmor()) : 0;
    battleUI = std::make_unique<BattleUI>(context.device);
    Enemy* enemy = CreateBossEnemy(context.device, bossId, 600.0f, 50.0f);
    battlefield = std::make_unique<Battlefield>(context.device, battleUI.get(), bossId, enemy, pochi, context.inventory);
}

namespace {
    // Pochi's FIGHT swing sfx
    // Called from every place that runs a fight
    void PlayAttackSfx(GameContext& context) {
        if (context.sound != nullptr) context.sound->PlaySfx("attack");
    }
}

void BattleScene::HandleInput(GameContext& context, GameStateManager&) {
    if (phase == ENCOUNTER) {
        phase = PLAYER_TURN;
        return;
    }
    if (phase == PLAYER_TURN) {
        battleUI->UpdateMenuButtons(context);
        int action = battleUI->GetSelectButton(context);
        const int actionKeys[4] = { NUM1_KEY, NUM2_KEY, NUM3_KEY, NUM4_KEY };
        for (int i = 0; i < 4; ++i) {
            if (JustPressed(context.keys, actionKeys[i], actionKeyWasDown[i])) action = i;
        }
        // Fight
        if (action == 0) {
            phase = ENEMY_HIT;
            battleUI->SetShowEncounterMessage(false);
            PlayAttackSfx(context);
            battlefield->PerformFight();
        }
        // Act
        else if (action == 1) {
            phase = ACT_MENU;
            battleUI->SetShowEncounterMessage(false);
            battleUI->SetShowActChoices(true);
            const int choiceKeys[3] = { NUM1_KEY, NUM2_KEY, NUM3_KEY };
            for (int i = 0; i < 3; ++i)
                actChoiceWasDown[i] = KeyDown(context.keys, choiceKeys[i]);
        }
        // Item
        else if (action == 2) {
			phase = ITEM_MENU;
			battleUI->SetShowEncounterMessage(false);
			battleUI->SetShowItemChoices(true, context.inventory);
			const int itemKeys[3] = { NUM1_KEY, NUM2_KEY, NUM3_KEY };
			for (int i = 0; i < 3; ++i) itemChoiceWasDown[i] =
				KeyDown(context.keys, itemKeys[i]);
        }
        // Mercy
        else if (action == 3) {
            battlefield->Flee();
        }
    }
	else if (phase == ITEM_MENU) {
		battleUI->UpdateMenuButtons(context);
		int action = battleUI->GetSelectButton(context);
		if (action == 0) {
			battleUI->SetShowItemChoices(false, context.inventory);
			phase = ENEMY_HIT;
			PlayAttackSfx(context);
			battlefield->PerformFight();
			return;
		}
		if (action == 1) {
			battleUI->SetShowItemChoices(false, context.inventory);
			battleUI->SetShowActChoices(true);
			phase = ACT_MENU;
			return;
		}
		if (action == 3) {
			battleUI->SetShowItemChoices(false, context.inventory);
			battlefield->Flee();
			return;
		}
		int choice = battleUI->GetItemSelection(context);
		const int itemKeys[3] = { NUM1_KEY, NUM2_KEY, NUM3_KEY };
		for (int i = 0; i < 3; ++i) {
			if (JustPressed(context.keys, itemKeys[i], itemChoiceWasDown[i])) choice = i;
		}
		if (choice >= 0) {
			ItemType item = ItemType::Toast;
			if (choice == 0) item = ItemType::HealthPotion;
			else if (choice == 1) item = ItemType::Bone;
			if (battlefield->PerformItem(item)) {
				battleUI->SetShowItemChoices(false, context.inventory);
				phase = ENEMY_ATTACK;
				battlefield->SetShowProjectiles(true);
			}
		}
	}
    else if (phase == ACT_MENU) {
		battleUI->UpdateMenuButtons(context);
		int action = battleUI->GetSelectButton(context);
		if (action == 0) {
			battleUI->SetShowActChoices(false);
			phase = ENEMY_HIT;
			PlayAttackSfx(context);
			battlefield->PerformFight();
			return;
		}
		if (action == 2) {
			battleUI->SetShowActChoices(false);
			battleUI->SetShowItemChoices(true, context.inventory);
			phase = ITEM_MENU;
			return;
		}
		if (action == 3) {
			battleUI->SetShowActChoices(false);
			battlefield->Flee();
			return;
		}
		int choice = battleUI->GetActSelection(context);
        const int choiceKeys[3] = { NUM1_KEY, NUM2_KEY, NUM3_KEY };
        for (int i = 0; i < 3; ++i) {
            if (!actChoiceUsed[i] && JustPressed(context.keys, choiceKeys[i], actChoiceWasDown[i])) {
                choice = i;
                break;
            }
		}
		if (choice >= 0 && !actChoiceUsed[choice]) {
			// All three ACT choices intentionally share this outcome
            actChoiceUsed[choice] = true;
            battleUI->SetActChoiceUsed(choice, true);
            battlefield->PerformAct();
            battleUI->SetShowActChoices(false);
			phase = ACT_ANIMATION;
			battlefield->StartActAnimation();
        }
    }
	else if (phase == ACT_ANIMATION && battlefield->IsActAnimationFinished()) {
		phase = PLAYER_TURN;
	}
    else if (phase == ENEMY_HIT) {
        if (battlefield->IsEnemyHitAnimationFinished()) {
            if (battlefield->IsEnemyDefeated()) {
				phase = PLAYER_TURN;
                return;
            }
            else {
                phase = ENEMY_ATTACK;
                battlefield->SetShowProjectiles(true);
            }
        }
    }
    else if (phase == ENEMY_ATTACK && battlefield->IsProjectileAttackFinished()) {
        battlefield->SetShowProjectiles(false);
        phase = PLAYER_TURN;
    }
}

void BattleScene::Update(GameContext& context, GameStateManager& manager) {
    // --- Developer cheats (F5) -----------
    if (Cheats::enabled) {
        // K: win the fight immediately
        if (JustPressed(context.keys, K_KEY, cheatWinWasDown)) {
            context.lastBattleOutcome = BattleOutcome::Victory;
            context.lastBattleBoss = bossId;
            manager.Pop();
            return;
        }
    }

    if (phase == ACT_ANIMATION) {
        battlefield->UpdateActAnimation();
        if (battlefield->IsActAnimationFinished()) {
            bool allUsed = true;
            for (int i = 0; i < 3; i++) {
                if (!actChoiceUsed[i]) {
                    allUsed = false;
                    break;
                }
            }
            phase = PLAYER_TURN;
            if (allUsed) {
                battleUI->SetShowActChoices(false);
                //Show "Pochi has used all of his alternate attacks."
            }
        }
    }
    battlefield->Update(context);

    // Cheat mode: undo any damage taken this frame so Maki (etc.) can't chip
    // the tester down, and don't let the fight be lost
    if (Cheats::enabled && pochi != nullptr) pochi->RestoreFull();

    // Pochi took damage this frame (armour or heart) -> play the hurt sfx
    if (pochi != nullptr) {
        const int hpNow = pochi->GetHealth() + pochi->GetArmor();
        if (hpNow < lastPochiHealth && context.sound != nullptr) {
            context.sound->PlaySfx("hurt");
        }
        lastPochiHealth = hpNow;
    }

    if (!Cheats::enabled && battlefield->IsPlayerDefeated()) {
        context.lastBattleOutcome = BattleOutcome::Defeat;
        context.lastBattleBoss = bossId;
        // Pochi is out of health -> game over screen
        // (replaces the whole stack, the ruins/forest run doesn't continue)
        manager.ClearAndPush(CreateGameOverScene(context.sound));
    }
    else if (phase != ENEMY_HIT && battlefield->IsEnemyDefeated()) {
        context.lastBattleOutcome = BattleOutcome::Victory;
        context.lastBattleBoss = bossId;
        manager.Pop();
    }
    else if (battlefield->HasFled()) {
        context.lastBattleOutcome = BattleOutcome::Fled;
        context.lastBattleBoss = bossId;
        manager.Pop();
    }
}

void BattleScene::Render(GameContext& context) {
    battlefield->Render(context.spriteBrush);
    battleUI->Render(context.spriteBrush);
}

D3DCOLOR BattleScene::ClearColor() const {
	return D3DCOLOR_XRGB(255, 255, 255);
}

std::unique_ptr<GameScene> CreateBattleScene(BossId bossId) {
	return std::make_unique<BattleScene>(bossId);
}