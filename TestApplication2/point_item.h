#ifndef POINT_ITEM_H
#define POINT_ITEM_H

#include "graphics_item.h"

class PointItem : public GraphicsItem {
public:
    explicit PointItem(const glm::vec2& pos = glm::vec2(0));
    ~PointItem();

    void setPosition(const glm::vec2& pos);
    void draw(const glm::mat4& viewMatrix) const override;
    bool contains(const glm::vec2& pos) const override;

    void setPointSize(float size) { m_pointSize = size; }
    void setColor(const glm::vec4& color) { m_color = color; }

private:
    GLuint m_vao;           // Vertex Array Object
    GLuint m_vbo;           // Vertex Buffer Object
    GLuint m_shader;        // Shader program

    glm::vec2 m_position;   
    float m_pointSize = 2.0f; 
    glm::vec4 m_color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); 

    void initializeGL();    
    void updateGeometry();  
    void cleanup();       
};

#endif // POINT_ITEM_H