#include "graphics_scene.h"
#include <algorithm>

Scene::Scene() {
    // No initialization needed for now
}

void Scene::addItem(std::shared_ptr<GraphicsItem> item) {
    if (item) {
        m_items.push_back(item);
    }
}

void Scene::removeItem(std::shared_ptr<GraphicsItem> item) {
    auto it = std::find(m_items.begin(), m_items.end(), item);
    if (it != m_items.end()) {
        m_items.erase(it);
    }
}

void Scene::clear() {
    m_items.clear();
}

void Scene::draw(const glm::mat4& viewMatrix) const {
    // Draw all items in order
    for (const auto& item : m_items) {
        if (item) {  // Add null check
            item->draw(viewMatrix);
        }
    }
}

std::shared_ptr<GraphicsItem> Scene::itemAt(const glm::vec2& pos) const {
    // Check items in reverse order (top to bottom)
    for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
        if ((*it)->contains(pos)) {
            return *it;
        }
    }
    return nullptr;
}

