#pragma once
#include <memory>
#include <vector>

namespace UI {

class UIScreen {
  public:
    virtual ~UIScreen() = default;
    virtual void Update(float dt) = 0;
    virtual void Render() = 0;
    // Return true to block input reaching the game world
    virtual bool BlocksGameInput() const { return false; }
    // Return true when this screen wants to be popped
    virtual bool IsDone() const { return false; }
};

class UIManager {
  public:
    // Push a screen on top (e.g. death screen over HUD)
    void Push(std::unique_ptr<UIScreen> screen);
    void Pop();
    void Clear();

    template <typename T> bool HasScreenOfType() const {
        for (const auto &screen : m_stack) {
            if (dynamic_cast<const T *>(screen.get()) != nullptr)
                return true;
        }
        return false;
    }
    template <typename T> T *Get() const {
        for (const auto &screen : m_stack) {
            if (auto *ptr = dynamic_cast<T *>(screen.get()))
                return ptr;
        }
        return nullptr;
    }

    void Update(float dt);
    void Render();

    bool BlocksGameInput() const;

  private:
    std::vector<std::unique_ptr<UIScreen>> m_stack;
};
} // namespace UI
