#include "texture_item.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <iostream>

// Shader source code
const char* textureVertexShader = R"(
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

const char* textureFragmentShader = R"(
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
        
        vec3 color = vec3(value);
        if (isSelected) {
            color = mix(color, vec3(0.2, 0.2, 1.0), 0.2);
        }
        FragColor = vec4(color, 1.0);
    }
)";

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
    initializeGL();
}

TextureItem::~TextureItem() {
    cleanup();
}

void TextureItem::initializeGL() {
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &textureVertexShader, NULL);
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
    glShaderSource(fragmentShader, 1, &textureFragmentShader, NULL);
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

    // Cleanup individual shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Generate buffers and texture
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenTextures(1, &m_texture);

    // Setup texture parameters
    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void TextureItem::setImage(const std::vector<uint16_t>& data, int width, int height) {
    m_width = width;
    m_height = height;
    m_displayData = data;

    // Store original data range
    m_dataMin = *std::min_element(data.begin(), data.end());
    m_dataMax = *std::max_element(data.begin(), data.end());

    // Convert 16-bit data to normalized float values
    std::vector<float> normalizedData(data.size());
    for (size_t i = 0; i < data.size(); ++i) {
        normalizedData[i] = static_cast<float>(data[i]) / 65535.0f;
    }

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F,
        width, height, 0,
        GL_RED, GL_FLOAT,
        normalizedData.data());

    // Set swizzle mask to show grayscale properly
    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    updateGeometry();
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