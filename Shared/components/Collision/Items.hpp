enum class ItemType {
    Unknown = 0,
    HealthPotion,
    ManaPotion,
    Key,
    Coin
};

struct Items {
    ItemType type;
    int item_id;
    bool picked_up;
};
