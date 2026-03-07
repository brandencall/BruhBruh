#pragma once
#include <stdint.h>

namespace Character {

enum class CharacterId : uint8_t {
    None = 0,
    Tonts = 1,
    Hodge = 2,
    Raff = 3,
    JJ = 4,
};

struct BulletDef {
    float speed;
    float radius;
    float damage;
    float lifetime;
};

struct CharacterDef {
    CharacterId id;
    float moveSpeed;
    float hurtboxRadius;
    float maxHealth;
    float fireRate;
    BulletDef bullet;
};

} // namespace Character
