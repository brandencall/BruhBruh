#pragma once

#include "entity.hpp"

struct AABB {
    float x;
    float y;
    float width;
    float height;
};

class Collider {
  public:
    Collider(AABB b, bool isStatic) : bounds(b), isStatic(isStatic) {}

  public:
    AABB bounds;
    bool isStatic = false;
    Entity *owner = nullptr;
};
