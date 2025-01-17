#define GLM_ENABLE_EXPERIMENTAL  // Add this before including GLM headers
#include "graphics_items.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>

// GraphicsItem Implementation
GraphicsItem::GraphicsItem()
    : m_transform(1.0f)  // Initialize with identity matrix
    , m_isSelected(false)
{
}

void GraphicsItem::setPosition(const glm::vec2& pos) {
    // Create translation matrix and combine with existing transform
    glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
    m_transform = translation * m_transform;
}

void GraphicsItem::setRotation(float angle) {
    // Get current position
    glm::vec3 currentPos = glm::vec3(m_transform[3]);

    // Create rotation matrix around current position
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), currentPos);
    transform = glm::rotate(transform, glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f));
    transform = glm::translate(transform, -currentPos);

    m_transform = transform * m_transform;
}

void GraphicsItem::setScale(const glm::vec2& scale) {
    // Get current position
    glm::vec3 currentPos = glm::vec3(m_transform[3]);

    // Create scale matrix around current position
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), currentPos);
    transform = glm::scale(transform, glm::vec3(scale.x, scale.y, 1.0f));
    transform = glm::translate(transform, -currentPos);

    m_transform = transform * m_transform;
}

// Scene Implementation
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
        item->draw(viewMatrix);
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

// GraphicsView Implementation
GraphicsView::GraphicsView(Scene* scene)
    : m_scene(scene)
    , m_viewMatrix(1.0f)  // Initialize with identity matrix
    , m_zoom(1.0f)
{
}

void GraphicsView::setScene(Scene* scene) {
    m_scene = scene;
}

void GraphicsView::zoom(float factor) {
    // Update zoom level with the new factor
    m_zoom *= factor;
    // Get the center of the viewport
    glm::vec2 viewCenter(0.0f, 0.0f);
    // Create transformation matrices
    glm::mat4 toOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(-viewCenter.x, -viewCenter.y, 0.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(factor, factor, 1.0f));
    glm::mat4 fromOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(viewCenter.x, viewCenter.y, 0.0f));
    // Combine transformations
    m_viewMatrix = fromOrigin * scale * toOrigin * m_viewMatrix;
}

void GraphicsView::pan(const glm::vec2& delta) {
    
    m_viewMatrix[3][0] += delta.x;
    m_viewMatrix[3][1] += delta.y;
}


void GraphicsView::fitInView(const glm::vec4& rect) {
    // Reset view matrix
    m_viewMatrix = glm::mat4(1.0f);

    // Calculate scale needed to fit rect in view
    float scaleX = 2.0f / rect.z;  // 2.0 because NDC space is [-1,1]
    float scaleY = 2.0f / rect.w;
    float scale = std::min(scaleX, scaleY);

    // Calculate center of rect
    glm::vec2 center(rect.x + rect.z * 0.5f, rect.y + rect.w * 0.5f);

    // Create transformation matrices
    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-center.x, -center.y, 0.0f));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));

    // Combine transformations
    m_viewMatrix = scaleMatrix * translate;

    // Don't modify zoom level here anymore
    // m_zoom = scale;  // Remove this line
}

float GraphicsView::getActualZoom() const {
    // Get the scale factor from the transform matrix
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    bool success = glm::decompose(m_viewMatrix, scale, rotation, translation, skew, perspective);

    if (success) {
        return (scale.x + scale.y) / 2.0f;
    }
    return m_zoom;  // Return stored zoom if decomposition fails
}

glm::vec2 GraphicsView::mapToScene(const glm::vec2& viewPos) const {
    // Convert view coordinates to scene coordinates
    glm::vec4 pos = glm::vec4(viewPos.x, viewPos.y, 0.0f, 1.0f);
    pos = glm::inverse(m_viewMatrix) * pos;
    return glm::vec2(pos.x, pos.y);
}

glm::vec2 GraphicsView::mapFromScene(const glm::vec2& scenePos) const {
    // Convert scene coordinates to view coordinates
    glm::vec4 pos = glm::vec4(scenePos.x, scenePos.y, 0.0f, 1.0f);
    pos = m_viewMatrix * pos;
    return glm::vec2(pos.x, pos.y);
}

// Shader source code
const char* rectVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 model;
    uniform mat4 view;
    
    void main() {
        gl_Position = view * model * vec4(aPos, 1.0);
    }
)";

const char* rectFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    
    uniform bool isSelected;
    
    void main() {
        if (isSelected) {
            FragColor = vec4(0.2, 0.2, 1.0, 0.3);  // Blue with transparency
        } else {
            FragColor = vec4(0.7, 0.7, 0.7, 0.3);  // Gray with transparency
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

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &rectFragmentShader, NULL);
    glCompileShader(fragmentShader);

    // Create shader program
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

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
    float vertices[] = {
        m_rect.x,              m_rect.y,               0.0f,  // bottom left
        m_rect.x + m_rect.z,   m_rect.y,               0.0f,  // bottom right
        m_rect.x + m_rect.z,   m_rect.y + m_rect.w,    0.0f,  // top right
        m_rect.x,              m_rect.y + m_rect.w,    0.0f   // top left
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void RectItem::setRect(const glm::vec4& rect) {
    m_rect = rect;
    updateGeometry();
}

void RectItem::draw(const glm::mat4& viewMatrix) const {
    glUseProgram(m_shader);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"),
        1, GL_FALSE, glm::value_ptr(m_transform));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"),
        1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform1i(glGetUniformLocation(m_shader, "isSelected"), m_isSelected);

    // Draw rectangle
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // Restore blend state
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

//GraphicsView Implementation

GraphicsView::~GraphicsView() {
    // No resources to clean up
}

void GraphicsView::setViewportSize(int width, int height) {
    m_viewportSize = glm::vec2(width, height);

    // Update projection matrix if needed
    float aspectRatio = static_cast<float>(width) / height;
    m_projMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
}

//TextureItem Implementation

TextureItem::TextureItem()
    : m_texture(0)
    , m_vao(0)
    , m_vbo(0)
    , m_shader(0)
    , m_width(0)
    , m_height(0)
    , m_windowMin(0.0f)
    , m_windowMax(1.0f)
    , m_dataMin(0)
    , m_dataMax(65535)
{
    // Vertex shader for basic texture rendering
    const char* vertexShaderSource = R"(
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

    // Fragment shader for grayscale display with enhanced contrast
    // Fragment shader for grayscale display
    const char* fragmentShaderSource = R"(
    #version 330 core
    out vec4 FragColor;
    
    in vec2 TexCoord;
    
    uniform sampler2D texture1;
    uniform bool isSelected;
    uniform float windowMin;
    uniform float windowMax;
    
    void main() {
        float value = texture(texture1, TexCoord).r;
        
        // Apply window/level adjustment
        value = (value - windowMin) / (windowMax - windowMin);
        value = clamp(value, 0.0, 1.0);
        
        // Simply output grayscale value
        FragColor = vec4(value, value, value, 1.0);
    }
)";
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
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
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        throw std::runtime_error("Fragment shader compilation failed");
    }

    // Create shader program and link shaders
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

    // Cleanup individual shaders as they're now linked into the program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Generate and bind vertex array object (VAO)
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // Generate texture
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Set default texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check for OpenGL errors
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        std::cerr << "OpenGL error during initialization: " << err << std::endl;
        cleanup();
        throw std::runtime_error("OpenGL initialization failed");
    }
}

TextureItem::~TextureItem() {
    cleanup();
}

void TextureItem::initializeGL() {
    // Vertex shader for basic texture rendering
    const char* vertexShaderSource = R"(
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

    // Fragment shader for grayscale display
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D texture1;
        uniform bool isSelected;
        uniform float windowMin;
        uniform float windowMax;
        
        void main() {
            float value = texture(texture1, TexCoord).r;
            
            // Apply window level
            value = (value - windowMin) / (windowMax - windowMin);
            value = clamp(value, 0.0, 1.0);
            
            vec3 color = vec3(value);
            if (isSelected) {
                color = mix(color, vec3(0.2, 0.2, 1.0), 0.2);
            }
            FragColor = vec4(color, 1.0);
        }
    )";

    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Check vertex shader compilation
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // Create shader program and link shaders
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

    // Check shader program linking
    glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    // Cleanup shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Generate and bind buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    // Generate texture
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void TextureItem::cleanup() {
    if (m_texture != 0) {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
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

void TextureItem::setImage(const std::vector<uint16_t>& data, int width, int height) {
    m_width = width;
    m_height = height;

    // Store original data range
    m_dataMin = *std::min_element(data.begin(), data.end());
    m_dataMax = *std::max_element(data.begin(), data.end());

    // Convert 16-bit data to normalized float values
    std::vector<float> normalizedData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        normalizedData[i] = static_cast<float>(data[i]) / 65535.0f;  // Normalize to [0,1]
    }

    // Create and bind texture
    glBindTexture(GL_TEXTURE_2D, m_texture);

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,  // Internal format: 32-bit float
        width, height, 0,
        GL_RED, GL_FLOAT,            // Format and type of the pixel data
        normalizedData.data());

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set swizzle mask to show grayscale properly
    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    // Update geometry for the quad that will display the texture
    updateGeometry();

    // Set initial window/level values
    float range = m_dataMax - m_dataMin;
    setWindowLevel(m_dataMin / 65535.0f, m_dataMax / 65535.0f);
}

void TextureItem::updateGeometry() {
    float vertices[] = {
        // positions        // texture coords
        0.0f,    0.0f,    0.0f,  0.0f, 1.0f,  // bottom left
        m_width, 0.0f,    0.0f,  1.0f, 1.0f,  // bottom right
        m_width, m_height, 0.0f, 1.0f, 0.0f,  // top right
        0.0f,    m_height, 0.0f, 0.0f, 0.0f   // top left
    };

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void TextureItem::draw(const glm::mat4& viewMatrix) const {
    glUseProgram(m_shader);

    // Set uniforms
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(m_transform));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform1i(glGetUniformLocation(m_shader, "isSelected"), m_isSelected);
    glUniform1f(glGetUniformLocation(m_shader, "windowMin"), m_windowMin);
    glUniform1f(glGetUniformLocation(m_shader, "windowMax"), m_windowMax);

    // Bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glUniform1i(glGetUniformLocation(m_shader, "texture1"), 0);

    // Draw quad
    glBindVertexArray(m_vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

bool TextureItem::contains(const glm::vec2& pos) const {
    // Transform point to local coordinates
    glm::vec4 localPos = glm::inverse(m_transform) * glm::vec4(pos.x, pos.y, 0.0f, 1.0f);

    // Check if point is within image bounds
    return localPos.x >= 0 && localPos.x <= m_width &&
        localPos.y >= 0 && localPos.y <= m_height;
}