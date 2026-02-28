#include "health_component.hpp"

namespace Component {
HealthComponent::HealthComponent(int maxHealth) : m_maxHealth(maxHealth), m_currentHealth(maxHealth) {}

int HealthComponent::getCurrentHealth() { return m_currentHealth; }

void HealthComponent::takeDamage(int damage) {
    if (damage > 0) {
        m_currentHealth -= damage;
        if (m_currentHealth < 0) {
            m_currentHealth = 0;
        }
    }
}

bool HealthComponent::isDead() { return (m_currentHealth <= 0); }

void HealthComponent::regenerateHealth(int amount) {
    if (amount > 0) {
        m_currentHealth += amount;
        if (m_currentHealth > m_maxHealth) {
            m_currentHealth = m_maxHealth;
        }
    }
}
} // namespace Component
