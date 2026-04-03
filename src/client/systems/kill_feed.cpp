#include "kill_feed.hpp"
#include <vector>

namespace System {

void KillFeed::Update(float dt) {
    for (auto &entry : m_feed) {
        entry.lifeTime -= dt;
    }
    std::erase_if(m_feed, [](Feed entry) { return entry.lifeTime <= 0.0; });
}

void KillFeed::Push(std::string killer, std::string victim) {
    Feed entry = {.killer = killer, .victim = victim, .lifeTime = initLifeTime};

    if (m_feed.size() >= killFeedSize) {
        m_feed.erase(m_feed.begin());
    }
    m_feed.push_back(entry);
}

const std::vector<Feed> &KillFeed::GetFeed() const { return m_feed; }

} // namespace System
