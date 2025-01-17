#ifndef GRAPHICS_ITEMS_H
#define GRAPHICS_ITEMS_H

#define GLM_ENABLE_EXPERIMENTAL  // Add this before including GLM headers
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <vector>
#include <iostream> 

// Base class for all graphics items
class GraphicsItem {
public:
    GraphicsItem();
    virtual ~GraphicsItem() = default;

    // Pure virtual functions for drawing and hit testing
    virtual void draw(const glm::mat4& viewMatrix) const = 0;
    virtual bool contains(const glm::vec2& pos) const = 0;

    // Transformation methods
    void setPosition(const glm::vec2& pos);
    void setRotation(float angle);
    void setScale(const glm::vec2& scale);

    // Selection handling
    bool isSelected() const { return m_isSelected; }
    void setSelected(bool selected) { m_isSelected = selected; }

    // Transform access
    const glm::mat4& getTransform() const { return m_transform; }

protected:
    glm::mat4 m_transform;  // Stores position, rotation, scale
    bool m_isSelected;      // Selection state
};

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

// View class for managing viewport and transformations
class GraphicsView {
public:
    GraphicsView(Scene* scene = nullptr);
    ~GraphicsView();

    // Scene management
    void setScene(Scene* scene);
    Scene* scene() const { return m_scene; }

    // View transformations
    void zoom(float factor);
    void pan(const glm::vec2& delta);
    void fitInView(const glm::vec4& rect);

    // Coordinate mapping
    glm::vec2 mapToScene(const glm::vec2& viewPos) const;
    glm::vec2 mapFromScene(const glm::vec2& scenePos) const;

    // View matrix access
    const glm::mat4& getViewMatrix() const { return m_viewMatrix; }
    void setViewMatrix(const glm::mat4& matrix) { m_viewMatrix = matrix; }

    // Window dimensions
    void setViewportSize(int width, int height);

    // Zoom control
    void setZoom(float newZoom) { m_zoom = newZoom; }
    float getZoom() const { return m_zoom; }
    float getActualZoom() const;

private:
    Scene* m_scene;           // Associated scene
    glm::mat4 m_viewMatrix;   // View transformation matrix
    glm::mat4 m_projMatrix;   // Projection matrix
    glm::vec2 m_viewportSize; // Current viewport dimensions
    float m_zoom;            // Current zoom level
};
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

private:
    GLuint m_texture;    // OpenGL texture ID
    GLuint m_vao;        // Vertex Array Object
    GLuint m_vbo;        // Vertex Buffer Object
    GLuint m_shader;     // Shader program
    int m_width;         // Texture width
    int m_height;        // Texture height

    float m_windowMin;   // Window level minimum
    float m_windowMax;   // Window level maximum

    uint16_t m_dataMin;  // Add data range members
    uint16_t m_dataMax;

    void initializeGL();    // Setup OpenGL resources
    void updateTexture();   // Update texture data
    void cleanup();        // Clean up OpenGL resources
    void updateGeometry(); // Update vertex geometry
};

// Concrete item for rectangles (e.g., selection)
class RectItem : public GraphicsItem {
public:
    explicit RectItem(const glm::vec4& rect = glm::vec4(0));
    ~RectItem();

    // Rectangle handling
    void setRect(const glm::vec4& rect);  // x, y, width, height
    void draw(const glm::mat4& viewMatrix) const override;
    bool contains(const glm::vec2& pos) const override;

    // Rectangle properties
    const glm::vec4& rect() const { return m_rect; }

private:
    glm::vec4 m_rect;     // Rectangle geometry (x, y, width, height)
    GLuint m_vao;         // Vertex Array Object
    GLuint m_vbo;         // Vertex Buffer Object
    GLuint m_shader;      // Shader program

    void initializeGL();   // Setup OpenGL resources
    void updateGeometry(); // Update vertex data
    void cleanup();       // Clean up OpenGL resources
};

#endif // GRAPHICS_ITEMS_H