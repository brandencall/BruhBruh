#pragma once

namespace Component {
class HealthComponent {
  public:
    HealthComponent(int maxHealth);
    ~HealthComponent() = default;
    int getCurrentHealth();
    void takeDamage(int damage);
    bool isDead();
    void regenerateHealth(int amount);

  private:
    int m_currentHealth;
    int m_maxHealth;
};
} // namespace Component
