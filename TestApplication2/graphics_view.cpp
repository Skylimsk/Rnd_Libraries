#include "graphics_view.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>
#include <iostream>
#include <cmath>

GraphicsView::GraphicsView(Scene* scene)
    : m_scene(scene)
    , m_viewMatrix(1.0f)
    , m_projMatrix(1.0f)
    , m_zoom(1.0f)
    , m_currentRect(nullptr)
    , m_windowWidth(1280)
    , m_windowHeight(720)
    , m_controlWindowCollapsed(false)
    , m_isPanning(false)
{

    updateViewMatrix();
}

GraphicsView::~GraphicsView() {

    m_currentRect = nullptr;
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

void GraphicsView::handleMousePress(const SDL_Event& event) {
    const float CONTROL_PANEL_HEIGHT = 50.0f;

    // Check if click is within viewport area
    if (event.button.button != SDL_BUTTON_LEFT ||
        event.button.y < CONTROL_PANEL_HEIGHT ||
        event.button.y > m_viewportSize.y) {
        return;
    }

    glm::vec2 screenPos(event.button.x, event.button.y);
    glm::vec2 scenePos = mapToScene(screenPos);

    if (!m_isDrawing) {
        if (m_currentRect) {
            m_scene->removeItem(m_currentRect);
            m_currentRect = nullptr;
        }
        m_isDrawing = true;
        m_currentRect = std::make_shared<RectItem>();
        m_currentRect->startDrawing(scenePos);
        m_scene->addItem(m_currentRect);
    }
    m_lastMousePos = screenPos;
}

void GraphicsView::handleMouseMove(const SDL_Event& event) {
    if (!m_isDrawing || !m_currentRect) return;

    float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 2);

    // Convert mouse coordinates taking control panel into account
    glm::vec2 screenPos(event.motion.x, event.motion.y);

    // Check if mouse is within viewport
    if (event.motion.y < controlPanelHeight ||
        event.motion.y > m_viewportSize.y) {
        return;
    }

    glm::vec2 scenePos = mapToScene(screenPos);
    m_currentRect->updateDrawing(scenePos);
    m_lastMousePos = screenPos;
}

void GraphicsView::handleMouseRelease(const SDL_Event& event) {
    if (!m_isDrawing || !m_currentRect) return;

    float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 2);

    // Convert coordinates taking control panel into account
    glm::vec2 screenPos(event.button.x, event.button.y);
    glm::vec2 scenePos = mapToScene(screenPos);

    m_currentRect->finishDrawing(scenePos);
    m_isDrawing = false;

    // Remove tiny rectangles
    const glm::vec4& rect = m_currentRect->rect();
    if (rect.z < 1.0f || rect.w < 1.0f) {
        m_scene->removeItem(m_currentRect);
        m_currentRect = nullptr;
    }
}

void GraphicsView::handleMouseWheel(const SDL_Event& event) {
    if (ImGui::GetIO().WantCaptureMouse) return;

    const Uint8* keystate = SDL_GetKeyboardState(nullptr);
    bool ctrlPressed = keystate[SDL_SCANCODE_LCTRL] || keystate[SDL_SCANCODE_RCTRL];

    if (ctrlPressed) {
        float zoomFactor = (event.wheel.y > 0) ? 1.1f : 0.9f;


        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        glm::vec2 mousePos(mouseX, mouseY);


        glm::vec2 beforePos = mapToScene(mousePos);
        zoom(zoomFactor);
        glm::vec2 afterPos = mapToScene(mousePos);


        pan(afterPos - beforePos);

        updateViewMatrix();
    }
}

void GraphicsView::pan(const glm::vec2& delta) {
    // Scale the pan delta based on viewport size
    float scaleX = 2.0f / m_viewportSize.x;
    float scaleY = 2.0f / m_viewportSize.y;

    m_viewMatrix[3][0] += delta.x * scaleX;
    m_viewMatrix[3][1] += delta.y * scaleY;

    updateViewMatrix();
}

void GraphicsView::updateViewMatrix() {
    float aspectRatio = m_viewportSize.x / m_viewportSize.y;
    m_projMatrix = glm::ortho(-aspectRatio, aspectRatio, -1.0f, 1.0f, -1.0f, 1.0f);
}

void GraphicsView::setViewportSize(int width, int height) {
    m_viewportSize = glm::vec2(width, height);
    updateViewMatrix();
}

float GraphicsView::getActualZoom() const {
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;

    if (glm::decompose(m_viewMatrix, scale, rotation, translation, skew, perspective)) {
        return (scale.x + scale.y) * 0.5f;
    }
    return m_zoom;
}

void GraphicsView::fitInView(const glm::vec4& rect) {

    m_viewMatrix = glm::mat4(1.0f);


    float scaleX = 2.0f / rect.z;
    float scaleY = 2.0f / rect.w;
    float scale = std::min(scaleX, scaleY);


    glm::vec2 center(rect.x + rect.z * 0.5f, rect.y + rect.w * 0.5f);


    glm::mat4 translate = glm::translate(glm::mat4(1.0f), glm::vec3(-center.x, -center.y, 0.0f));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f));


    m_viewMatrix = scaleMatrix * translate;
    updateViewMatrix();
}

glm::vec2 GraphicsView::mapToScene(const glm::vec2& viewPos) const {
    const float CONTROL_PANEL_HEIGHT = 50.0f;  // Fixed control panel height
    const float TITLE_BAR_HEIGHT = 16.5f;
    float viewportHeight = m_viewportSize.y - CONTROL_PANEL_HEIGHT;

    // Convert to viewport space
    glm::vec2 viewportPos(
        viewPos.x,
        viewPos.y - CONTROL_PANEL_HEIGHT - TITLE_BAR_HEIGHT   // Subtract control panel height
    );

    // Convert to NDC (-1 to 1)
    glm::vec2 ndcPos(
        (2.0f * viewportPos.x / m_viewportSize.x) - 1.0f,
        1.0f - (2.0f * viewportPos.y / viewportHeight)
    );

    // Transform to scene coordinates
    glm::mat4 invTransform = glm::inverse(m_viewMatrix);
    glm::vec4 scenePos = invTransform * glm::vec4(ndcPos.x, ndcPos.y, 0.0f, 1.0f);

    return glm::vec2(scenePos.x / scenePos.w, scenePos.y / scenePos.w);
}

glm::vec2 GraphicsView::mapFromScene(const glm::vec2& scenePos) const {

    glm::mat4 mvp = m_projMatrix * m_viewMatrix;
    glm::vec4 clipPos = mvp * glm::vec4(scenePos.x, scenePos.y, 0.0f, 1.0f);
    clipPos /= clipPos.w;


    glm::vec2 winPos(
        (clipPos.x + 1.0f) * 0.5f * m_viewportSize.x,
        (1.0f - (clipPos.y + 1.0f) * 0.5f) * m_viewportSize.y
    );

    return winPos;
}

bool GraphicsView::isPointInImage(const glm::vec2& scenePos) const {
    if (!m_scene) return false;
    const auto& items = m_scene->items();
    for (const auto& item : items) {
        if (auto textureItem = std::dynamic_pointer_cast<TextureItem>(item)) {
            return scenePos.x >= 0 && scenePos.x < textureItem->width() &&
                scenePos.y >= 0 && scenePos.y < textureItem->height();
        }
    }
    return false;
}

glm::vec2 GraphicsView::getImageSize() const {
    if (!m_scene) return glm::vec2(0);
    const auto& items = m_scene->items();
    for (const auto& item : items) {
        if (auto textureItem = std::dynamic_pointer_cast<TextureItem>(item)) {
            return glm::vec2(textureItem->width(), textureItem->height());
        }
    }
    return glm::vec2(0);
}

glm::vec2 GraphicsView::viewToImageCoordinates(const glm::vec2& viewPos, const glm::vec2& imageSize) const {
    glm::vec2 viewportSize = getViewportSize();
    float scaleX = imageSize.x / viewportSize.x;
    float scaleY = imageSize.y / viewportSize.y;
    return glm::vec2(viewPos.x * scaleX, viewPos.y * scaleY);
}