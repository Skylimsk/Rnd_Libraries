#include "point_item.h"
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

const char* pointVertexShader = R"(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    
    uniform mat4 model;
    uniform mat4 view;
    uniform float pointSize;
    
    void main() {
        gl_Position = view * model * vec4(aPos, 1.0);
        gl_PointSize = pointSize;
    }
)";


const char* pointFragmentShader = R"(
    #version 330 core
    out vec4 FragColor;
    
    uniform vec4 pointColor;
    
    void main() {

        vec2 circCoord = 2.0 * gl_PointCoord - 1.0;
        if (dot(circCoord, circCoord) > 1.0) {
            discard;
        }
        FragColor = pointColor;
    }
)";

PointItem::PointItem(const glm::vec2& pos)
    : m_position(pos)
    , m_vao(0)
    , m_vbo(0)
    , m_shader(0)
{
    initializeGL();
    updateGeometry();
}

PointItem::~PointItem() {
    cleanup();  // Call the existing cleanup method to free OpenGL resources
}

void PointItem::initializeGL() {

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &pointVertexShader, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &pointFragmentShader, NULL);
    glCompileShader(fragmentShader);

  
    m_shader = glCreateProgram();
    glAttachShader(m_shader, vertexShader);
    glAttachShader(m_shader, fragmentShader);
    glLinkProgram(m_shader);

  
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);


    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void PointItem::updateGeometry() {
    float vertices[] = {
        m_position.x, m_position.y, 0.0f 
    };

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
}

void PointItem::setPosition(const glm::vec2& pos) {
    m_position = pos;
    updateGeometry();
}

void PointItem::draw(const glm::mat4& viewMatrix) const {
    glUseProgram(m_shader);


    glUniformMatrix4fv(glGetUniformLocation(m_shader, "model"), 1, GL_FALSE, glm::value_ptr(m_transform));
    glUniformMatrix4fv(glGetUniformLocation(m_shader, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniform4fv(glGetUniformLocation(m_shader, "pointColor"), 1, glm::value_ptr(m_color));
    glUniform1f(glGetUniformLocation(m_shader, "pointSize"), m_pointSize);


    glEnable(GL_PROGRAM_POINT_SIZE);


    glBindVertexArray(m_vao);
    glDrawArrays(GL_POINTS, 0, 1);


    glDisable(GL_PROGRAM_POINT_SIZE);
}

bool PointItem::contains(const glm::vec2& pos) const {

    float distance = glm::length(pos - m_position);
    return distance < m_pointSize * 0.5f;
}

void PointItem::cleanup() {
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