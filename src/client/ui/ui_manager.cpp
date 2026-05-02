#include "ui_manager.hpp"

namespace UI {

UIManager::~UIManager() { Clear(); }

void UIManager::Push(std::unique_ptr<UIScreen> screen) { m_stack.push_back(std::move(screen)); }

void UIManager::Pop() {
    if (!m_stack.empty())
        m_stack.pop_back();
}

void UIManager::Clear() { m_stack.clear(); }

const UIScreen *UIManager::Peek() const {
    if (m_stack.empty())
        return nullptr;

    return m_stack.back().get();
}

// Current logic only updates the very top screen. May need to change this in the future
void UIManager::Update(float dt) {
    if (!m_stack.empty() && m_stack.back()->IsDone())
        m_stack.pop_back();

    if (!m_stack.empty())
        m_stack.back()->Update(dt);
}

void UIManager::Render() {
    for (auto &screen : m_stack)
        screen->Render();
}

bool UIManager::BlocksGameInput() const { return !m_stack.empty() && m_stack.back()->BlocksGameInput(); }

} // namespace UI
