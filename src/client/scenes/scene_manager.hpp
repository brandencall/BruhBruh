#pragma once
#include "scene.hpp"
#include <memory>
#include <vector>

class SceneManager {
  public:
    void Push(std::unique_ptr<Scene> scene) {
        if (!m_stack.empty())
            m_stack.back()->OnExit();
        m_stack.push_back(std::move(scene));
        m_stack.back()->OnEnter();
    }

    void Pop() {
        if (m_stack.empty())
            return;
        m_stack.back()->OnExit();
        m_stack.pop_back();
        if (!m_stack.empty())
            m_stack.back()->OnEnter();
    }

    void RequestReplace(std::unique_ptr<Scene> scene) { m_pendingReplace = std::move(scene); }

    // Replace the current scene entirely
    void Replace(std::unique_ptr<Scene> scene) {
        if (!m_stack.empty()) {
            m_stack.back()->OnExit();
            m_stack.pop_back();
        }
        m_stack.push_back(std::move(scene));
        m_stack.back()->OnEnter();
    }

    void RequestPop() { m_pendingPop = true; }

    void Update(float dt) {
        if (m_pendingReplace) {
            Replace(std::move(m_pendingReplace));
            m_pendingReplace = nullptr;
        }
        if (m_pendingPop) {
            Pop();
            m_pendingPop = false;
        }

        if (!m_stack.empty())
            m_stack.back()->Update(dt);
    }

    void Render() {
        if (!m_stack.empty())
            m_stack.back()->Render();
    }

    bool Empty() const { return m_stack.empty(); }

  private:
    std::vector<std::unique_ptr<Scene>> m_stack;
    std::unique_ptr<Scene> m_pendingReplace;
    bool m_pendingPop = false;
};
