#pragma once

#include "collision_components.hpp"
class Entity;

namespace Component {
class Collider {
  public:
    Collider(AABB b, bool isStatic, Entity *owner) : bounds(b), isStatic(isStatic), owner(owner) {}

  public:
    AABB bounds;
    bool isStatic = false;
    Entity *owner = nullptr;
};
} // namespace Component
