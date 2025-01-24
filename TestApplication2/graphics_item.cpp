#define GLM_ENABLE_EXPERIMENTAL
#include "graphics_item.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

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