#include "GameContext.h"
#include "Pochi.h"
#include "Inventory.h"
#include "Sprite.h"
#include "SaveGame.h"

// Resets everything a fresh playthrough starts from (main menu, retry)
void ResetRunProgress(GameContext& context) {
    if (context.playerStats != NULL) context.playerStats->SetLevel(1); // Fresh Pochi, stats refilled
    if (context.inventory != NULL) context.inventory->Reset();         // Empty the pack
    context.clearedMaps.clear();                                       // Every map locked again
    context.collectedItems.clear();
    context.clearedBosses.clear();
    context.lastBattleOutcome = BattleOutcome::None;                   // Drop any stale battle result
    context.hasPendingSpawn = false;                                   // Drop any pending spawn hand-off
}

void SaveCurrentRun(const GameContext& context) {
    save::Progress p;
    p.valid = true;
    p.mapId = (int)context.currentMapId;
    if (context.pochi != NULL) {
        const D3DXVECTOR2 pos = context.pochi->GetPosition();
        p.px = pos.x;
        p.py = pos.y;
    }
    p.level = context.playerStats != NULL ? context.playerStats->GetLevel() : 1;
    if (context.inventory != NULL) {
        p.potions = context.inventory->GetCount(ItemType::HealthPotion);
        p.bones = context.inventory->GetCount(ItemType::Bone);
        p.toast = context.inventory->GetCount(ItemType::Toast);
    }
    for (MapId m : context.clearedMaps) p.clearedMaps.push_back((int)m);
    for (int k : context.collectedItems) p.collectedItems.push_back(k);
    for (int k : context.clearedBosses) p.clearedBosses.push_back(k);
    save::SaveProgress(p);
}
