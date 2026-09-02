#include "BattleState.h"
#include "Battlefield.h"
#include "BattleUI.h"
#include "Cheats.h"
#include "Pochi.h"
#include "Enemy.h"
#include "SoundManage.h"
#include <dinput.h>

namespace {
bool JustPressed(BYTE* keys, int key, bool& wasDown) {
    const bool down = keys != nullptr && (keys[key] & 0x80) != 0;
    const bool pressed = down && !wasDown;
    wasDown = down;
    return pressed;
}

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
// complete types when the unique_ptr members are destroyed.
BattleState::~BattleState() = default;

void BattleState::Initialize(GameContext& context) {
	pochi = context.playerStats;
	lastPochiHealth = (pochi != nullptr) ? (pochi->GetHealth() + pochi->GetArmor()) : 0;
    battleUI = std::make_unique<BattleUI>(context.device);
    Enemy* enemy = CreateBossEnemy(context.device, bossId, 600.0f, 50.0f);
    battlefield = std::make_unique<Battlefield>(context.device, battleUI.get(), bossId, enemy, pochi, context.inventory);
}

namespace {
    // Pochi's FIGHT swing sfx. Called from every place that runs a fight.
    void PlayAttackSfx(GameContext& context) {
        if (context.sound != nullptr) context.sound->PlaySfx("attack");
    }
}

void BattleState::HandleInput(GameContext& context, GameStateManager&) {
    if (phase == ENCOUNTER) {
        phase = PLAYER_TURN;
        return;
    }
    if (phase == PLAYER_TURN) {
        battleUI->UpdateMenuButtons(context);
        int action = battleUI->GetSelectButton(context);
        const int actionKeys[4] = { DIK_1, DIK_2, DIK_3, DIK_4 };
        for (int i = 0; i < 4; ++i) {
            if (JustPressed(context.keys, actionKeys[i], actionKeyWasDown[i])) action = i;
        }
        //Fight
        if (action == 0) {
            phase = ENEMY_HIT;
            battleUI->SetShowEncounterMessage(false);
            PlayAttackSfx(context);
            battlefield->PerformFight();
        }
        //Act
        else if (action == 1) {
            phase = ACT_MENU;
            battleUI->SetShowEncounterMessage(false);
            battleUI->SetShowActChoices(true);
            const int choiceKeys[3] = { DIK_1, DIK_2, DIK_3 };
            for (int i = 0; i < 3; ++i) {
                actChoiceWasDown[i] = context.keys != nullptr &&
                    (context.keys[choiceKeys[i]] & 0x80) != 0;
            }
        }
        //Item
        else if (action == 2) {
			phase = ITEM_MENU;
			battleUI->SetShowEncounterMessage(false);
			battleUI->SetShowItemChoices(true, context.inventory);
			const int itemKeys[3] = { DIK_1, DIK_2, DIK_3 };
			for (int i = 0; i < 3; ++i) itemChoiceWasDown[i] =
				context.keys != nullptr && (context.keys[itemKeys[i]] & 0x80) != 0;
        }
        //Mercy
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
		const int itemKeys[3] = { DIK_1, DIK_2, DIK_3 };
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
        const int choiceKeys[3] = { DIK_1, DIK_2, DIK_3 };
        for (int i = 0; i < 3; ++i) {
            if (!actChoiceUsed[i] && JustPressed(context.keys, choiceKeys[i], actChoiceWasDown[i])) {
                choice = i;
                break;
            }
		}
		if (choice >= 0 && !actChoiceUsed[choice]) {
			// All three ACT choices intentionally share this outcome.
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

void BattleState::Update(GameContext& context, GameStateManager& manager) {
    // --- developer cheats (F5 toggles the switch, see Cheats.h) -----------
    if (Cheats::enabled) {
        // Keep the tester alive: top Pochi's health/armor back up every frame.
        if (pochi != nullptr) pochi->RestoreFull();
        // K: win the fight immediately. (Plain letter keys - laptop OEM
        // Fn rows eat F8/F9 for airplane mode / brightness before the game
        // ever sees them.)
        if (JustPressed(context.keys, DIK_K, cheatWinWasDown)) {
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

    // Pochi took damage this frame (armour or heart) -> play the hurt sfx.
    if (pochi != nullptr) {
        const int hpNow = pochi->GetHealth() + pochi->GetArmor();
        if (hpNow < lastPochiHealth && context.sound != nullptr) {
            context.sound->PlaySfx("hurt");
        }
        lastPochiHealth = hpNow;
    }

    if (battlefield->IsPlayerDefeated()) {
        context.lastBattleOutcome = BattleOutcome::Defeat;
        context.lastBattleBoss = bossId;
        // Pochi is out of health -> game over screen (replaces the whole
        // stack; the ruins/forest run doesn't continue).
        manager.ClearAndPush(CreateGameOverState(context.sound));
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

void BattleState::Render(GameContext& context) {
    battlefield->Render(context.spriteBrush);
    battleUI->Render(context.spriteBrush);
}

D3DCOLOR BattleState::ClearColor() const {
	return D3DCOLOR_XRGB(255, 255, 255);
}

std::unique_ptr<GameState> CreateBattleState(BossId bossId) {
	return std::make_unique<BattleState>(bossId);
}