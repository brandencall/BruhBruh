#pragma once

#include <string>
#include <vector>

namespace System {

// Could add an emblem for each killer
struct Feed {
    std::string killer;
    std::string victim;
    float lifeTime;
};

class KillFeed {

  public:
    KillFeed() = default;
    void Update(float dt);
    void Push(std::string killer, std::string victim);
    const std::vector<Feed> &GetFeed() const;

  private:
    static constexpr int killFeedSize = 5;
    static constexpr float initLifeTime = 3.5;
    std::vector<Feed> m_feed;
};

} // namespace System
