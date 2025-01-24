#ifndef TEXTURE_ITEM_H
#define TEXTURE_ITEM_H

#include "graphics_item.h"
#include <vector>

class TextureItem : public GraphicsItem {
public:
    TextureItem();
    ~TextureItem();

    // Image handling
    void setImage(const std::vector<uint16_t>& data, int width, int height);
    void draw(const glm::mat4& viewMatrix) const override;
    bool contains(const glm::vec2& pos) const override;

    // Texture dimensions
    int width() const { return m_width; }
    int height() const { return m_height; }

    // Window level controls
    void setWindowLevel(float min, float max) {
        m_windowMin = min;
        m_windowMax = max;
    }

    std::vector<uint16_t> getDisplayData() const {
        return m_displayData;
    }

    glm::vec2 getImageSize() const {
        return glm::vec2(m_width, m_height);
    }

private:
    GLuint m_texture;    // OpenGL texture ID
    GLuint m_vao;        // Vertex Array Object
    GLuint m_vbo;        // Vertex Buffer Object
    GLuint m_shader;     // Shader program
    int m_width;         // Texture width
    int m_height;        // Texture height

    float m_windowMin;   // Window level minimum
    float m_windowMax;   // Window level maximum

    uint16_t m_dataMin;  // Data range members
    uint16_t m_dataMax;

    void initializeGL();    // Setup OpenGL resources
    void updateTexture();   // Update texture data
    void cleanup();        // Clean up OpenGL resources
    void updateGeometry(); // Update vertex geometry

    std::vector<uint16_t> m_displayData;
};

#endif // TEXTURE_ITEM_H