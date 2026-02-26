#pragma once
#include <raylib.h>

class Player {
  public:
    Player(Vector2 startPos);

    void Update(float dt);
    void Draw();

    Vector2 GetPosition() const;

  private:
    float m_speed = 200.0f;
    Vector2 m_position;
};
