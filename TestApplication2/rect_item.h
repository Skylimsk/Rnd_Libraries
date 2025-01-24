#ifndef RECT_ITEM_H
#define RECT_ITEM_H

#include "graphics_item.h"

class RectItem : public GraphicsItem {
public:
    explicit RectItem(const glm::vec4& rect = glm::vec4(0));
    ~RectItem();

    // Rectangle handling
    void setRect(const glm::vec4& rect);  // x, y, width, height
    void draw(const glm::mat4& viewMatrix) const override;
    bool contains(const glm::vec2& pos) const override;

    // Drawing state management
    void startDrawing(const glm::vec2& startPos);
    void updateDrawing(const glm::vec2& currentPos);
    void finishDrawing(const glm::vec2& endPos);

    // Visual properties
    void setBorderWidth(float width) { m_borderWidth = width; }
    void setDrawingColor(const glm::vec4& color) { m_drawingColor = color; }
    void setSelectedColor(const glm::vec4& color) { m_selectedColor = color; }

    // Selection state
    void setSelected(bool selected) { m_isSelected = selected; }
    bool isSelected() const { return m_isSelected; }

    const glm::vec4& rect() const { return m_rect; }

private:
    glm::vec4 m_rect;        // Rectangle geometry (x, y, width, height)
    GLuint m_vao;           // Vertex Array Object
    GLuint m_vbo;           // Vertex Buffer Object
    GLuint m_shader;        // Shader program

    // Drawing state
    bool m_isDrawing = false;
    bool m_isSelected = false;
    glm::vec2 m_startPos;
    glm::vec2 m_currentPos;

    // Visual properties
    float m_borderWidth = 2.0f;
    glm::vec4 m_drawingColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); 
    glm::vec4 m_selectedColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); 

    void initializeGL();    // Setup OpenGL resources
    void updateGeometry();  // Update vertex data
    void cleanup();        // Clean up OpenGL resources
};

#endif // RECT_ITEM_H