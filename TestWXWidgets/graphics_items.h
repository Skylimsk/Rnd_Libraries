#ifndef GRAPHICS_ITEMS_H
#define GRAPHICS_ITEMS_H

#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

// Base class for all graphics items
class GraphicsItem {
public:
    GraphicsItem();
    virtual ~GraphicsItem() = default;

    // Pure virtual functions for drawing and bounds
    virtual void draw(sf::RenderTarget& target, const sf::RenderStates& states) const = 0;
    virtual sf::FloatRect getBoundingRect() const = 0;

    // Transformation methods
    void setPosition(const sf::Vector2f& pos);
    void setRotation(float angle);
    void setScale(const sf::Vector2f& scale);

    // Selection handling
    bool isSelected() const { return m_isSelected; }
    void setSelected(bool selected) { m_isSelected = selected; }

    // Transform access
    const sf::Transform& getTransform() const { return m_transform; }

protected:
    sf::Transform m_transform;  // Stores position, rotation, scale
    bool m_isSelected;         // Selection state
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
    void draw(sf::RenderTarget& target, const sf::RenderStates& states) const;
    std::shared_ptr<GraphicsItem> itemAt(const sf::Vector2f& pos) const;

    // Access to items
    const std::vector<std::shared_ptr<GraphicsItem>>& items() const { return m_items; }

private:
    std::vector<std::shared_ptr<GraphicsItem>> m_items;  // List of all items in scene
};

// View class for managing viewport and transformations
class GraphicsView {
public:
    GraphicsView(Scene* scene = nullptr);

    // Scene management
    void setScene(Scene* scene);
    Scene* scene() const { return m_scene; }

    // View transformations
    void zoom(float factor);
    void pan(const sf::Vector2f& delta);
    void fitInView(const sf::FloatRect& rect);

    // Coordinate mapping
    sf::Vector2f mapToScene(const sf::Vector2f& viewPos) const;
    sf::Vector2f mapFromScene(const sf::Vector2f& scenePos) const;

    // View access
    const sf::View& getView() const { return m_view; }
    void setView(const sf::View& view) { m_view = view; }

private:
    Scene* m_scene;     // Associated scene
    sf::View m_view;    // SFML view for transformations
};

// Concrete item for displaying images
class PixmapItem : public GraphicsItem {
public:
    PixmapItem();

    // Image handling
    void setImage(const sf::Image& image);
    void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;
    sf::FloatRect getBoundingRect() const override;

private:
    sf::Texture m_texture;  // Image texture
    sf::Sprite m_sprite;    // Sprite for rendering
};

// Concrete item for rectangles (e.g., selection)
class RectItem : public GraphicsItem {
public:
    explicit RectItem(const sf::FloatRect& rect = sf::FloatRect());

    // Rectangle handling
    void setRect(const sf::FloatRect& rect);
    void draw(sf::RenderTarget& target, const sf::RenderStates& states) const override;
    sf::FloatRect getBoundingRect() const override;

private:
    sf::RectangleShape m_rect;  // Rectangle shape
};

#endif // GRAPHICS_ITEMS_H