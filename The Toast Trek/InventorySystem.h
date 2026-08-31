#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <vector>
#include <map>
#include <functional>

// Item types for categorization
enum class ItemType {
    NONE,
    WEAPON,
    ARMOR,
    TOOL,
    FOOD,
    MATERIAL,
    QUEST,
    KEY_ITEM,
    SEED,
    FISH,
    FURNITURE,
    DECORATION,
    POTION
};

// Item rarity for color coding
enum class ItemRarity {
    COMMON,
    UNCOMMON,
    RARE,
    EPIC,
    LEGENDARY,
    MYTHIC
};

// Item structure
struct Item {
    int id;
    std::string name;
    std::string description;
    ItemType type;
    ItemRarity rarity;
    int stackSize;
    int maxStackSize;
    std::string iconPath;
    int value;
    bool isEquippable;
    bool isConsumable;
    bool isQuestItem;

    // Stats for equipment
    int attackBonus;
    int defenseBonus;
    int healthBonus;
    int speedBonus;
    int manaBonus;

    // Food/Potion stats
    int hungerRestore;
    int healthRestore;
    int manaRestore;

    Item()
        : id(0), name(""), description(""), type(ItemType::NONE), rarity(ItemRarity::COMMON)
        , stackSize(0), maxStackSize(1), iconPath(""), value(0)
        , isEquippable(false), isConsumable(false), isQuestItem(false)
        , attackBonus(0), defenseBonus(0), healthBonus(0), speedBonus(0), manaBonus(0)
        , hungerRestore(0), healthRestore(0), manaRestore(0) {
    }
};

// Inventory slot
struct InventorySlot {
    Item item;
    int quantity;
    bool isEmpty;
    bool isEquipped;

    InventorySlot() : quantity(0), isEmpty(true), isEquipped(false) {}
};

// Inventory page
struct InventoryPage {
    std::string name;
    std::vector<InventorySlot> slots;
    int rows;
    int cols;
    int pageIndex;

    InventoryPage() : rows(5), cols(6), pageIndex(0) {
        slots.resize(rows * cols);
    }

    InventoryPage(int r, int c) : rows(r), cols(c), pageIndex(0) {
        slots.resize(rows * cols);
    }
};

class InventorySystem {
private:
    std::map<std::string, InventoryPage> pages;
    std::vector<std::string> pageOrder;
    std::map<int, Item> itemDatabase;
    int nextItemId;

    int gold;
    int maxSlots;

    // Equipment slots
    struct Equipment {
        Item weapon;
        Item helmet;
        Item chestplate;
        Item leggings;
        Item boots;
        Item accessory;
        bool hasWeapon, hasHelmet, hasChestplate, hasLeggings, hasBoots, hasAccessory;

        Equipment() : hasWeapon(false), hasHelmet(false), hasChestplate(false),
            hasLeggings(false), hasBoots(false), hasAccessory(false) {
        }
    } equipment;

    // Callbacks for item usage
    std::function<bool(const Item&)> onItemUse;
    std::function<bool(const Item&)> onItemEquip;
    std::function<void(const Item&)> onItemUnequip;

    void InitializeItemDatabase();
    int FindEmptySlot(const std::string& pageName);
    int FindStackSlot(const std::string& pageName, int itemId);

public:
    InventorySystem();
    ~InventorySystem();

    // Initialize with default items
    void Initialize();

    // Page management
    void AddPage(const std::string& name, int rows = 5, int cols = 6);
    bool RemovePage(const std::string& name);
    bool HasPage(const std::string& name) const;
    std::vector<std::string> GetPageNames() const;
    InventoryPage* GetPage(const std::string& name);
    const InventoryPage* GetPage(const std::string& name) const;

    // Item operations
    bool AddItem(const Item& item, int quantity = 1);
    bool RemoveItem(int itemId, int quantity = 1);
    bool RemoveItemFromSlot(const std::string& pageName, int slotIndex, int quantity = 1);
    bool UseItem(const std::string& pageName, int slotIndex);
    bool EquipItem(const std::string& pageName, int slotIndex);
    bool UnequipItem(ItemType slot);
    bool DiscardItem(const std::string& pageName, int slotIndex, int quantity = 1);

    // Sorting and filtering
    void SortPageByName(const std::string& pageName);
    void SortPageByRarity(const std::string& pageName);
    void SortPageByType(const std::string& pageName);
    void FilterPageByType(const std::string& pageName, ItemType type);
    void ClearFilter(const std::string& pageName);

    // Gold management
    void AddGold(int amount);
    bool SpendGold(int amount);
    int GetGold() const { return gold; }
    void SetGold(int amount) { gold = amount; }

    // Equipment access
    const Equipment& GetEquipment() const { return equipment; }
    void SetEquipment(const Equipment& eq) { equipment = eq; }

    // Stats calculation
    int GetTotalAttack() const;
    int GetTotalDefense() const;
    int GetTotalHealthBonus() const;
    int GetTotalSpeedBonus() const;
    int GetTotalManaBonus() const;

    // Item database
    Item* GetItemFromDatabase(int id);
    int GetItemIdByName(const std::string& name) const;

    // Callbacks
    void SetOnItemUse(std::function<bool(const Item&)> callback);
    void SetOnItemEquip(std::function<bool(const Item&)> callback);
    void SetOnItemUnequip(std::function<void(const Item&)> callback);

    // Utility
    int GetTotalItems() const;
    int GetFreeSlots(const std::string& pageName) const;
    bool IsFull(const std::string& pageName) const;
    void ClearPage(const std::string& pageName);
};