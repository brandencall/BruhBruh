#pragma once
#include "../../shared/systems/ability_system.hpp"
#include "../../shared/systems/bullet_system.hpp"
#include "raylib.h"
#include <cstdint>
#include <functional>

namespace System {

class ServerBulletSystem : public BulletSystem<state::BulletState> {
  public:
    ServerBulletSystem(std::array<state::PlayerState, MAX_PLAYERS> &players) : m_players(players) {}

    void Initialize(Map::WallManager &wallManager, System::AbilitySystem &abilitySystem, EventBus &eventBus) {
        m_wallManager = &wallManager;
        m_abilitySystem = &abilitySystem;
        m_eventBus = &eventBus;
    }

  protected:
    void OnWallHit(Map::Vector2i gridPos, float damage, uint32_t shooterId) override {
        m_wallManager->DamageWall(gridPos, damage, shooterId);
    }

    void OnPlayerHit(uint32_t playerId, float damage, uint32_t shooterId) override {
        auto &target = m_players[playerId];
        auto &attacker = m_players[shooterId];

        if (target.invincibilityTimer > 0.0f)
            return;

        target.health -= state::GetOverallDamage(damage, attacker.effects);
        m_abilitySystem->ApplyDebuffs(target, attacker);
        target.lastDamageTakenTimer = 0.0f;
        if (target.health <= 0.0f) {
            HandlePlayerDied(target, shooterId);
            AddHealthOnKill(attacker);
            return;
        }
    }

    void HandlePlayerDied(state::PlayerState &player, uint32_t shooterId) {
        player.state = state::State::Dead;
        player.health = 0.0f;
        player.respawnTimer = RESPAWN_TIME;
        m_abilitySystem->ClearAbilitiesAndEffects(player);
        auto &killer = m_players[shooterId];
        player.score.deaths++;
        killer.score.kills++;
        m_wallManager->ClearWallsForPlayer(player.id);
        m_eventBus->publish(event::PlayerDiedEvent{player, killer});
    }

    void AddHealthOnKill(state::PlayerState &player) {
        Character::CharacterDef charDef = Character::GetCharacterDef(player.characterId);
        player.health = std::min(charDef.maxHealth, player.health + 25);
    }

    void OnBulletSpawn(uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId, Vector2 position,
                       Vector2 velocity, uint32_t bulletPredSequence) override {
        m_eventBus->publish(
            event::BulletSpawnEvent{bulletId, ownerId, characterId, bulletPredSequence, position, velocity});
    }

    void OnBulletDestroyed(uint32_t bulletId, Vector2 position, Character::CharacterId characterId) override {
        m_eventBus->publish(event::BulletDestroyedEvent{bulletId, position, characterId});
    }

  private:
    std::function<void(Map::Vector2i gridPos, float damage, uint32_t shooterId)> m_onWallHit;
    std::function<void(uint32_t playerId, float damage, uint32_t shooterId)> m_onPlayerHit;
    std::function<void(uint32_t bulletId, uint32_t ownerId, Character::CharacterId characterId, Vector2 position,
                       Vector2 velocity, uint32_t bulletPredSequence)>
        m_onBulletSpawn;
    std::function<void(uint32_t bulletId, Vector2 position, Character::CharacterId characterId)> m_onBulletDestroyed;

    Map::WallManager *m_wallManager = nullptr;
    std::array<state::PlayerState, MAX_PLAYERS> &m_players;
    AbilitySystem *m_abilitySystem = nullptr;
    EventBus *m_eventBus = nullptr;
};

} // namespace System
