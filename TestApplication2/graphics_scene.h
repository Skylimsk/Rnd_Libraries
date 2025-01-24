#ifndef GRAPHICS_SCENE_H
#define GRAPHICS_SCENE_H

#include <GL/glew.h>
#include <vector>
#include <memory>
#include "graphics_item.h"

// Scene manages all graphics items
class Scene {
public:
    Scene();
    ~Scene() = default;

    // Item management
    void addItem(std::shared_ptr<GraphicsItem> item);
    void removeItem(std::shared_ptr<GraphicsItem> item);
    void clear();

    // Drawing and item lookup
    void draw(const glm::mat4& viewMatrix) const;
    std::shared_ptr<GraphicsItem> itemAt(const glm::vec2& pos) const;

    // Access to items
    const std::vector<std::shared_ptr<GraphicsItem>>& items() const { return m_items; }

private:
    std::vector<std::shared_ptr<GraphicsItem>> m_items;  // List of all items in scene
    GLuint m_selectionShader;  // Shader program for selection feedback
};

#endif // GRAPHICS_SCENE_H