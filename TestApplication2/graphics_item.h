// graphics_item.h
#ifndef GRAPHICS_ITEMS_H
#define GRAPHICS_ITEMS_H

#define GLM_ENABLE_EXPERIMENTAL

#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

class GraphicsItem {
public:
    GraphicsItem();
    virtual ~GraphicsItem() = default;

    virtual void draw(const glm::mat4& viewMatrix) const = 0;
    virtual bool contains(const glm::vec2& pos) const = 0;

    void setPosition(const glm::vec2& pos);
    void setRotation(float angle);
    void setScale(const glm::vec2& scale);

    bool isSelected() const { return m_isSelected; }
    void setSelected(bool selected) { m_isSelected = selected; }

    const glm::mat4& getTransform() const { return m_transform; }

protected:
    glm::mat4 m_transform;  // Stores position, rotation, scale
    bool m_isSelected;      // Selection state


};

#endif // GRAPHICS_ITEMS_H