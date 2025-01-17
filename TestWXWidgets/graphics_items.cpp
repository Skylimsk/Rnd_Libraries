#include "graphics_items.h"
#include <algorithm>

// GraphicsItem Implementation
GraphicsItem::GraphicsItem() : m_isSelected(false) {}

void GraphicsItem::setPosition(const sf::Vector2f& pos) {
    // Set position in transform matrix
    sf::Transform translation;
    translation.translate(pos);
    m_transform = translation * m_transform;
}

void GraphicsItem::setRotation(float angle) {
    // Set rotation in transform matrix
    sf::Transform rotation;
    rotation.rotate(angle);
    m_transform = rotation * m_transform;
}

void GraphicsItem::setScale(const sf::Vector2f& scale) {
    // Set scale in transform matrix
    sf::Transform scaling;
    scaling.scale(scale);
    m_transform = scaling * m_transform;
}

// Scene Implementation
Scene::Scene() {}

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

void Scene::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
    // Draw all items in order
    for (const auto& item : m_items) {
        item->draw(target, states);
    }
}

std::shared_ptr<GraphicsItem> Scene::itemAt(const sf::Vector2f& pos) const {
    // Check items in reverse order (top to bottom)
    for (auto it = m_items.rbegin(); it != m_items.rend(); ++it) {
        if ((*it)->getBoundingRect().contains(pos)) {
            return *it;
        }
    }
    return nullptr;
}

// GraphicsView Implementation
GraphicsView::GraphicsView(Scene* scene) : m_scene(scene) {
    // Initialize view with default size
    m_view = sf::View(sf::FloatRect(0, 0, 800, 600));
}

void GraphicsView::setScene(Scene* scene) {
    m_scene = scene;
}

void GraphicsView::zoom(float factor) {
    // Apply zoom factor to view
    m_view.zoom(factor);
}

void GraphicsView::pan(const sf::Vector2f& delta) {
    // Move view by delta
    m_view.move(delta);
}

void GraphicsView::fitInView(const sf::FloatRect& rect) {
    // Adjust view to show entire rect
    m_view.reset(rect);
}

sf::Vector2f GraphicsView::mapToScene(const sf::Vector2f& viewPos) const {
    // Convert view coordinates to scene coordinates
    return m_view.getTransform().transformPoint(viewPos);
}

sf::Vector2f GraphicsView::mapFromScene(const sf::Vector2f& scenePos) const {
    // Convert scene coordinates to view coordinates
    return m_view.getInverseTransform().transformPoint(scenePos);
}

// PixmapItem Implementation
PixmapItem::PixmapItem() {}

void PixmapItem::setImage(const sf::Image& image) {
    // Load image into texture and set up sprite
    m_texture.loadFromImage(image);
    m_sprite.setTexture(m_texture);
}

void PixmapItem::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
    // Combine item transform with states
    sf::RenderStates combinedStates = states;
    combinedStates.transform *= m_transform;

    // Draw sprite
    target.draw(m_sprite, combinedStates);

    // Draw selection border if selected
    if (m_isSelected) {
        sf::RectangleShape border(sf::Vector2f(m_sprite.getGlobalBounds().width,
            m_sprite.getGlobalBounds().height));
        border.setFillColor(sf::Color::Transparent);
        border.setOutlineColor(sf::Color::Blue);
        border.setOutlineThickness(2.0f);
        target.draw(border, combinedStates);
    }
}

sf::FloatRect PixmapItem::getBoundingRect() const {
    // Get transformed bounds
    return m_transform.transformRect(m_sprite.getGlobalBounds());
}

// RectItem Implementation
RectItem::RectItem(const sf::FloatRect& rect) {
    setRect(rect);
}

void RectItem::setRect(const sf::FloatRect& rect) {
    // Set up rectangle shape
    m_rect.setSize(sf::Vector2f(rect.width, rect.height));
    m_rect.setPosition(sf::Vector2f(rect.left, rect.top));
}

void RectItem::draw(sf::RenderTarget& target, const sf::RenderStates& states) const {
    // Create temporary rectangle for drawing
    sf::RectangleShape drawRect = m_rect;

    // Set selection appearance
    if (m_isSelected) {
        drawRect.setOutlineColor(sf::Color::Blue);
        drawRect.setOutlineThickness(2.0f);
    }
    else {
        drawRect.setOutlineThickness(0.0f);
    }

    // Combine transforms and draw
    sf::RenderStates combinedStates = states;
    combinedStates.transform *= m_transform;
    target.draw(drawRect, combinedStates);
}

sf::FloatRect RectItem::getBoundingRect() const {
    // Get transformed bounds
    return m_transform.transformRect(m_rect.getGlobalBounds());
}