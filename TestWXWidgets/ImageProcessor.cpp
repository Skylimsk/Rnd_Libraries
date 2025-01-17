#include "imageprocessor.h"
#include <wx/filedlg.h>
#include <wx/msgdlg.h>
#include <fstream>
#include <sstream>

// Event tables
BEGIN_EVENT_TABLE(SFMLCanvas, wxPanel)
EVT_PAINT(SFMLCanvas::OnPaint)
EVT_SIZE(SFMLCanvas::OnSize)
EVT_LEFT_DOWN(SFMLCanvas::OnMouseDown)
EVT_MOTION(SFMLCanvas::OnMouseMove)
EVT_LEFT_UP(SFMLCanvas::OnMouseUp)
EVT_MOUSEWHEEL(SFMLCanvas::OnMouseWheel)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(ImageProcessor, wxFrame)
EVT_BUTTON(ID_LOAD_BUTTON, ImageProcessor::OnLoadFile)
EVT_BUTTON(ID_SAVE_BUTTON, ImageProcessor::OnSaveFile)
EVT_BUTTON(ID_ZOOM_IN, ImageProcessor::OnZoomIn)
EVT_BUTTON(ID_ZOOM_OUT, ImageProcessor::OnZoomOut)
EVT_BUTTON(ID_FIT_VIEW, ImageProcessor::OnFitView)
END_EVENT_TABLE()

// SFMLCanvas Implementation
SFMLCanvas::SFMLCanvas(wxWindow* parent, wxWindowID id)
    : wxPanel(parent, id),
    m_sfmlWindow(nullptr),
    m_isDragging(false),
    m_isPanning(false)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);

    // Initialize SFML
    m_sfmlWindow = new sf::RenderWindow;

    // Initialize scene and view
    m_view.setScene(&m_scene);

    // Create graphics items
    m_imageItem = std::make_shared<PixmapItem>();
    m_selectionItem = std::make_shared<RectItem>();

    // Add items to scene
    m_scene.addItem(m_imageItem);
    m_scene.addItem(m_selectionItem);
}

SFMLCanvas::~SFMLCanvas() {
    delete m_sfmlWindow;
}

void SFMLCanvas::OnPaint(wxPaintEvent& event) {
    wxPaintDC dc(this);
    updateRenderWindow();

    if (m_sfmlWindow && m_sfmlWindow->isOpen()) {
        m_sfmlWindow->clear(sf::Color::White);
        m_sfmlWindow->setView(m_view.getView());
        m_scene.draw(*m_sfmlWindow, sf::RenderStates::Default);
        m_sfmlWindow->display();
    }
}

void SFMLCanvas::OnSize(wxSizeEvent& event) {
    updateRenderWindow();
    event.Skip();
}

void SFMLCanvas::updateRenderWindow() {
    wxSize size = GetSize();
    if (m_sfmlWindow) {
        m_sfmlWindow->create((sf::WindowHandle)GetHandle());
        m_sfmlWindow->setSize(sf::Vector2u(size.x, size.y));

        // Update view size while maintaining zoom/pan
        sf::View currentView = m_view.getView();
        float aspectRatio = static_cast<float>(size.x) / size.y;
        currentView.setSize(currentView.getSize().y * aspectRatio,
            currentView.getSize().y);
        m_view.setView(currentView);
    }
}

void SFMLCanvas::setImage(const std::vector<std::vector<uint16_t>>& imageData) {
    if (imageData.empty()) return;

    int height = imageData.size();
    int width = imageData[0].size();

    // Create SFML image
    sf::Image image;
    image.create(width, height);

    // Convert image data to SFML format
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Convert 16-bit value to 8-bit for display
            uint8_t pixelValue = static_cast<uint8_t>(imageData[y][x] >> 8);
            image.setPixel(x, y, sf::Color(pixelValue, pixelValue, pixelValue));
        }
    }

    // Update image item and fit view
    m_imageItem->setImage(image);
    m_view.fitInView(m_imageItem->getBoundingRect());
    Refresh();
}

void SFMLCanvas::OnMouseDown(wxMouseEvent& event) {
    if (event.ControlDown()) {
        // Pan mode
        m_isPanning = true;
    }
    else {
        // Selection mode
        m_isDragging = true;
        sf::Vector2f scenePos = m_view.mapToScene(sf::Vector2f(event.GetX(), event.GetY()));
        m_selectionItem->setRect(sf::FloatRect(scenePos.x, scenePos.y, 0, 0));
        m_selectionItem->setSelected(true);
    }
    m_lastMousePos = event.GetPosition();
    Refresh();
}

void SFMLCanvas::OnMouseMove(wxMouseEvent& event) {
    if (m_isPanning) {
        // Handle panning
        wxPoint delta = event.GetPosition() - m_lastMousePos;
        m_view.pan(sf::Vector2f(-delta.x, -delta.y));
    }
    else if (m_isDragging) {
        // Handle selection
        sf::Vector2f currentPos = m_view.mapToScene(sf::Vector2f(event.GetX(), event.GetY()));
        sf::Vector2f startPos = m_view.mapToScene(sf::Vector2f(m_lastMousePos.x, m_lastMousePos.y));

        float left = std::min(startPos.x, currentPos.x);
        float top = std::min(startPos.y, currentPos.y);
        float width = std::abs(currentPos.x - startPos.x);
        float height = std::abs(currentPos.y - startPos.y);

        m_selectionItem->setRect(sf::FloatRect(left, top, width, height));
    }
    m_lastMousePos = event.GetPosition();
    Refresh();
}

void SFMLCanvas::OnMouseUp(wxMouseEvent& event) {
    if (m_isDragging) {
        // Emit selection event through parent ImageProcessor
        sf::FloatRect selectionBounds = m_selectionItem->getBoundingRect();
        wxRect selection(selectionBounds.left, selectionBounds.top,
            selectionBounds.width, selectionBounds.height);

        wxCommandEvent selEvent(wxEVT_COMMAND_BUTTON_CLICKED);
        selEvent.SetInt(selection.GetRight() - selection.GetLeft());
        wxPostEvent(GetParent(), selEvent);
    }

    m_isDragging = false;
    m_isPanning = false;
    Refresh();
}

void SFMLCanvas::OnMouseWheel(wxMouseEvent& event) {
    float factor = event.GetWheelRotation() > 0 ? 0.9f : 1.1f;
    m_view.zoom(factor);
    Refresh();
}

// ImageProcessor Implementation
ImageProcessor::ImageProcessor(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600)),
    m_regionSelected(false)
{
    setupUI();
    connectSignals();
}

ImageProcessor::~ImageProcessor() {
}

void ImageProcessor::setupUI() {
    // Create main sizer
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);

    // Create canvas
    m_canvas = new SFMLCanvas(this);
    mainSizer->Add(m_canvas, 1, wxEXPAND | wxALL, 5);

    // Create button panel
    wxPanel* buttonPanel = new wxPanel(this);
    wxBoxSizer* buttonSizer = new wxBoxSizer(wxHORIZONTAL);

    // Create buttons
    m_loadButton = new wxButton(buttonPanel, ID_LOAD_BUTTON, "Load Image");
    m_saveButton = new wxButton(buttonPanel, ID_SAVE_BUTTON, "Save Image");
    m_zoomInButton = new wxButton(buttonPanel, ID_ZOOM_IN, "Zoom In");
    m_zoomOutButton = new wxButton(buttonPanel, ID_ZOOM_OUT, "Zoom Out");
    m_fitButton = new wxButton(buttonPanel, ID_FIT_VIEW, "Fit View");

    // Add buttons to sizer
    buttonSizer->Add(m_loadButton, 0, wxALL, 5);
    buttonSizer->Add(m_saveButton, 0, wxALL, 5);
    buttonSizer->Add(m_zoomInButton, 0, wxALL, 5);
    buttonSizer->Add(m_zoomOutButton, 0, wxALL, 5);
    buttonSizer->Add(m_fitButton, 0, wxALL, 5);

    buttonPanel->SetSizer(buttonSizer);
    mainSizer->Add(buttonPanel, 0, wxALIGN_CENTER | wxALL, 5);

    // Create status bar
    m_statusBar = CreateStatusBar();
    m_statusBar->SetStatusText("Ready");

    SetSizer(mainSizer);
}

void ImageProcessor::connectSignals() {
    // Connect image loading signal
    imageLoadedSignal.connect([this](const std::string& path) {
        m_statusBar->SetStatusText(wxString::Format("Loaded: %s", path));
        });

    // Connect region selection signal
    regionSelectedSignal.connect([this](const wxRect& region) {
        OnRegionSelected(region);
        });
}

void ImageProcessor::OnLoadFile(wxCommandEvent& event) {
    wxFileDialog openFileDialog(this, "Open Text File", "", "",
        "Text files (*.txt)|*.txt|All files (*.*)|*.*",
        wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return;

    loadTxtImage(openFileDialog.GetPath().ToStdString());
}

void ImageProcessor::loadTxtImage(const std::string& txtFilePath) {
    std::ifstream inFile(txtFilePath);
    if (!inFile.is_open()) {
        wxMessageBox("Cannot open file: " + wxString(txtFilePath),
            "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Clear existing data
    m_imgData.clear();
    size_t maxWidth = 0;

    // Read image data from text file
    std::string line;
    while (std::getline(inFile, line)) {
        std::stringstream ss(line);
        std::vector<uint16_t> row;
        uint32_t value;

        while (ss >> value) {
            // Clamp values to 16-bit
            row.push_back(static_cast<uint16_t>(std::min(value,
                static_cast<uint32_t>(65535))));
        }

        if (!row.empty()) {
            maxWidth = std::max(maxWidth, row.size());
            m_imgData.push_back(row);
        }
    }
    inFile.close();

    if (m_imgData.empty()) {
        wxMessageBox("No valid data found in the file.",
            "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Normalize row widths
    for (auto& row : m_imgData) {
        if (row.size() < maxWidth) {
            row.resize(maxWidth, row.empty() ? 0 : row.back());
        }
    }

    // Store original image and update display
    m_originalImg = m_imgData;
    m_regionSelected = false;
    m_selectedRegion = wxRect();

    // Update canvas with new image
    m_canvas->setImage(m_imgData);

    // Emit signal
    imageLoadedSignal(txtFilePath);
}

void ImageProcessor::OnSaveFile(wxCommandEvent& event) {
    wxFileDialog saveFileDialog(this, "Save Image", "", "",
        "PNG files (*.png)|*.png|All files (*.*)|*.*",
        wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return;

    // Get scene from canvas
    Scene* scene = m_canvas->getScene();
    if (!scene) return;

    // Create render texture for saving
    sf::RenderTexture renderTexture;
    if (!renderTexture.create(m_imgData[0].size(), m_imgData.size())) {
        wxMessageBox("Failed to create render texture",
            "Error", wxOK | wxICON_ERROR);
        return;
    }

    // Setup view for saving
    renderTexture.clear(sf::Color::White);
    renderTexture.setView(renderTexture.getDefaultView());

    // Draw scene
    scene->draw(renderTexture, sf::RenderStates::Default);
    renderTexture.display();

    // Save to file
    if (!renderTexture.getTexture().copyToImage().saveToFile(
        saveFileDialog.GetPath().ToStdString())) {
        wxMessageBox("Failed to save image",
            "Error", wxOK | wxICON_ERROR);
    }
}

void ImageProcessor::OnZoomIn(wxCommandEvent& event) {
    m_canvas->getView()->zoom(0.9f);
    m_canvas->Refresh();
}

void ImageProcessor::OnZoomOut(wxCommandEvent& event) {
    m_canvas->getView()->zoom(1.1f);
    m_canvas->Refresh();
}

void ImageProcessor::OnFitView(wxCommandEvent& event) {
    if (m_canvas->getScene()->items().size() > 0) {
        auto bounds = m_canvas->getScene()->items()[0]->getBoundingRect();
        m_canvas->getView()->fitInView(bounds);
        m_canvas->Refresh();
    }
}

void ImageProcessor::OnRegionSelected(const wxRect& region) {
    m_selectedRegion = region;
    m_regionSelected = true;

    // Update status bar with selection info
    m_statusBar->SetStatusText(wxString::Format(
        "Selection: (%d, %d) - (%d, %d) [%d x %d]",
        region.GetLeft(), region.GetTop(),
        region.GetRight(), region.GetBottom(),
        region.GetWidth(), region.GetHeight()));
}

void ImageProcessor::updateDisplayImage() {
    if (!m_imgData.empty()) {
        m_canvas->setImage(m_imgData);
    }
}