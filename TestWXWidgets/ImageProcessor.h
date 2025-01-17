#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <wx/wx.h>
#include <wx/panel.h>
#include <SFML/Graphics.hpp>
#include <boost/signals2.hpp>
#include "graphics_items.h"
#include <vector>
#include <memory>

// Canvas class that integrates SFML with wxWidgets and handles graphics view
class SFMLCanvas : public wxPanel {
public:
    SFMLCanvas(wxWindow* parent, wxWindowID id = wxID_ANY);
    virtual ~SFMLCanvas();

    // Image handling
    void setImage(const std::vector<std::vector<uint16_t>>& imageData);

    // Event handlers
    void OnPaint(wxPaintEvent& event);
    void OnSize(wxSizeEvent& event);
    void OnMouseDown(wxMouseEvent& event);
    void OnMouseMove(wxMouseEvent& event);
    void OnMouseUp(wxMouseEvent& event);
    void OnMouseWheel(wxMouseEvent& event);

    // Access to scene and view
    Scene* getScene() { return &m_scene; }
    GraphicsView* getView() { return &m_view; }

private:
    // SFML rendering
    sf::RenderWindow* m_sfmlWindow;

    // Graphics view architecture components
    Scene m_scene;
    GraphicsView m_view;
    std::shared_ptr<PixmapItem> m_imageItem;      // For displaying the main image
    std::shared_ptr<RectItem> m_selectionItem;    // For selection rectangle

    // Mouse interaction
    bool m_isDragging;
    wxPoint m_lastMousePos;
    bool m_isPanning;  // New: for view panning

    void updateRenderWindow();
    DECLARE_EVENT_TABLE()
};

class ImageProcessor : public wxFrame {
public:
    ImageProcessor(const wxString& title);
    virtual ~ImageProcessor();

private:
    // UI Components
    SFMLCanvas* m_canvas;
    wxButton* m_loadButton;
    wxButton* m_saveButton;
    wxButton* m_zoomInButton;   // New: zoom controls
    wxButton* m_zoomOutButton;
    wxButton* m_fitButton;      // New: fit to view
    wxStatusBar* m_statusBar;

    // Data storage
    std::vector<std::vector<uint16_t>> m_imgData;
    std::vector<std::vector<uint16_t>> m_originalImg;
    bool m_regionSelected;
    wxRect m_selectedRegion;

    // Signals
    boost::signals2::signal<void(const std::string&)> imageLoadedSignal;
    boost::signals2::signal<void(const wxRect&)> regionSelectedSignal;

    // Event handlers
    void OnLoadFile(wxCommandEvent& event);
    void OnSaveFile(wxCommandEvent& event);
    void OnZoomIn(wxCommandEvent& event);     // New
    void OnZoomOut(wxCommandEvent& event);    // New
    void OnFitView(wxCommandEvent& event);    // New
    void OnRegionSelected(const wxRect& region);

    // Helper methods
    void setupUI();
    void connectSignals();
    void loadTxtImage(const std::string& txtFilePath);
    void updateDisplayImage();

    DECLARE_EVENT_TABLE()
};

// Control IDs
enum {
    ID_LOAD_BUTTON = wxID_HIGHEST + 1,
    ID_SAVE_BUTTON,
    ID_ZOOM_IN,    // New
    ID_ZOOM_OUT,   // New
    ID_FIT_VIEW    // New
};

#endif // IMAGEPROCESSOR_H