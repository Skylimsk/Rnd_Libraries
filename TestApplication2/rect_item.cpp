#include "rect_item.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

const char* rectVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aTexCoord;
    
    uniform mat4 model;
    uniform mat4 view;
    
    out vec2 TexCoord;
    
    void main() {
        gl_Position = view * model * vec4(aPos, 1.0);
        TexCoord = aTexCoord;
    }
)";

// Update the fragment shader
const char* rectFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    uniform vec4 rectColor;
    
    void main() {
        vec2 center = vec2(0.5, 0.5);
        vec2 d = abs(TexCoord - center) * 2.0;
        
        float thickness = 0.02;
        
        if (max(d.x, d.y) > 0.99 - thickness) {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0); // Red border
        } else {
            FragColor = vec4(0.0, 0.0, 0.0, 0.0); // Transparent interior
        }
    }
)";

RectItem::RectItem(const glm::vec4& rect)
    : m_rect(rect)
    , m_vao(0)
    , m_vbo(0)
    , m_shader(0)
{
    initializeGL();
    updateGeometry();
}

RectItem::~RectItem() {
    cleanup();
}

void RectItem::initializeGL() {
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &rectVertexShader, NULL);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
        throw std::runtime_error("Vertex shader compilation failed");
    }

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &rectFragmentShader, NULL);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        throw std::runtime_error("Fragment shader compilation failed");
    }

    // Create shader program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

    // Check shader program linking
    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        throw std::runtime_error("Shader program linking failed");
    }

    // Cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // Set up vertex attributes
    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void RectItem::updateGeometry() {
    // Round all coordinates to nearest pixel for precise rendering
    glm::vec2 topLeft = glm::round(glm::vec2(m_rect.x, m_rect.y));
    glm::vec2 size = glm::round(glm::vec2(m_rect.z, m_rect.w));

    float vertices[] = {
        // Position (x, y, z)           // TexCoords (u, v)
        topLeft.x,         topLeft.y,          0.0f,  0.0f, 0.0f, // Bottom left
        topLeft.x + size.x, topLeft.y,          0.0f,  1.0f, 0.0f, // Bottom right
        topLeft.x + size.x, topLeft.y + size.y, 0.0f,  1.0f, 1.0f, // Top right
        topLeft.x,         topLeft.y + size.y, 0.0f,  0.0f, 1.0f  // Top left
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Configure vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void RectItem::setRect(const glm::vec4& rect) {
    m_rect = rect;
    updateGeometry();
}

void RectItem::startDrawing(const glm::vec2& startPos) {
    m_isDrawing = true;
    m_startPos = glm::round(startPos); // Round to nearest pixel
    setRect(glm::vec4(m_startPos.x, m_startPos.y, 0, 0));
}

void RectItem::updateDrawing(const glm::vec2& currentPos) {
    if (!m_isDrawing) return;

    glm::vec2 roundedPos = glm::round(currentPos);

    // Calculate pixel-perfect dimensions
    float width = roundedPos.x - m_startPos.x;
    float height = roundedPos.y - m_startPos.y;

    // Support drawing in any direction
    float x = width > 0 ? m_startPos.x : roundedPos.x;
    float y = height > 0 ? m_startPos.y : roundedPos.y;

    setRect(glm::vec4(x, y, std::abs(width), std::abs(height)));
}

void RectItem::finishDrawing(const glm::vec2& endPos) {
    updateDrawing(endPos);
    m_isDrawing = false;
}

void RectItem::draw(const glm::mat4& viewMatrix) const {
    if (m_rect.z <= 0 || m_rect.w <= 0) return;

    glUseProgram(m_shader);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Enable line antialiasing
    glEnable(GL_LINE_SMOOTH);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"),
        1, GL_FALSE, glm::value_ptr(m_transform));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"),
        1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform2f(glGetUniformLocation(m_shader, "rectDimensions"), m_rect.z, m_rect.w);

    // Draw rectangle
    glBindVertexArray(m_vao);
    glDrawArrays(GL_LINE_LOOP, 0, 4);

    // Cleanup
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_BLEND);
}

bool RectItem::contains(const glm::vec2& pos) const {
    // Transform point to local coordinates
    glm::vec4 localPos = glm::inverse(m_transform) * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);

    // Check if point is within rectangle bounds
    return localPos.x >= m_rect.x && localPos.x <= (m_rect.x + m_rect.z) &&
        localPos.y >= m_rect.y && localPos.y <= (m_rect.y + m_rect.w);
}

void RectItem::cleanup() {
    if (m_vao != 0) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo != 0) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_shader != 0) {
        glDeleteProgram(m_shader);
        m_shader = 0;
    }
}