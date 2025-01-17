#include "imageprocessor.h"
#include <iostream>
#include <fstream>
#include <sstream>

// Disable warnings for external library
#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#undef _CRT_SECURE_NO_WARNINGS

class LoadingProgress {
public:
    static void updateProgress(const std::string& filename, float percentage, char* statusText, size_t bufferSize) {
        snprintf(statusText, bufferSize, "Loading %s... %.1f%%",
            filename.c_str(), percentage);
    }
};

ImageProcessor::ImageProcessor()
    : m_window(nullptr)
    , m_windowWidth(1280)
    , m_windowHeight(720)
    , m_running(true)
    , m_showLoadDialog(false)
    , m_showSaveDialog(false)
    , m_showControlsWindow(true)
    , m_currentPath(std::filesystem::current_path().string())
    , m_lastPanX(0.0f)
    , m_lastPanY(0.0f)
    , m_regionSelected(false)
{
    if (!initializeWindow()) {
        throw std::runtime_error("Failed to initialize window");
    }

    initializeImGui();

    // Initialize graphics items
    m_imageItem = std::make_shared<TextureItem>();
    m_selectionItem = std::make_shared<RectItem>();

    // Add items to scene
    m_scene.addItem(m_imageItem);
    m_scene.addItem(m_selectionItem);

    // Setup view
    m_view.setScene(&m_scene);
    m_view.setViewportSize(m_windowWidth, m_windowHeight);

    strcpy_s(m_statusText, sizeof(m_statusText), "Ready");
}

ImageProcessor::~ImageProcessor() {
    if (m_displayTexture) {
        glDeleteTextures(1, &m_displayTexture);
        m_displayTexture = 0;
    }
    cleanup();
}

bool ImageProcessor::initializeWindow() {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit()) {
        return false;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Disable window resizing and maximization
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);  // Keep window decorations (title bar, etc)

    // Enable MSAA
    glfwWindowHint(GLFW_SAMPLES, 4);

    m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Image Processor", nullptr, nullptr);
    if (!m_window) {
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // Enable vsync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return false;
    }

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set default viewport
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    // Additional debug info
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    glfwSetWindowUserPointer(m_window, this);

    // Set callbacks
    glfwSetMouseButtonCallback(m_window, ImageProcessor::mouseButtonCallback);
    glfwSetCursorPosCallback(m_window, ImageProcessor::cursorPositionCallback);
    glfwSetScrollCallback(m_window, ImageProcessor::scrollCallback);
    glfwSetFramebufferSizeCallback(m_window, ImageProcessor::framebufferSizeCallback);

    return true;
}

void ImageProcessor::initializeImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void ImageProcessor::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    if (m_window) {
        glfwDestroyWindow(m_window);
    }
    glfwTerminate();
}

void ImageProcessor::drawControlsWindow() {
    
}

void ImageProcessor::handleLoadDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(m_windowWidth * 0.5f, m_windowHeight * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Load Image##Dialog", &m_showLoadDialog)) {
        ImGui::Text("Current Path: %s", m_currentPath.c_str());
        ImGui::Separator();

        if (ImGui::BeginChild("Files", ImVec2(0, -40), true)) {
            if (ImGui::Button("..")) {
                fs::path current(m_currentPath);
                if (current.has_parent_path()) {
                    m_currentPath = current.parent_path().string();
                }
            }

            try {
                for (const auto& entry : fs::directory_iterator(m_currentPath)) {
                    const auto& path = entry.path();
                    std::string filename = path.filename().string();

                    if (fs::is_directory(entry)) {
                        if (ImGui::Selectable((filename + "/").c_str())) {
                            m_currentPath = path.string();
                            m_selectedFilePath.clear(); // Clear selection when changing directory
                        }
                    }
                    else if (path.extension() == ".txt") {
                        bool isSelected = (m_selectedFilePath == path.string());
                        if (ImGui::Selectable(filename.c_str(), isSelected)) {
                            m_selectedFilePath = path.string();
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }

            ImGui::EndChild();
        }

        // Add buttons at the bottom with proper spacing
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);  // Add some spacing
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 120;
        float spacing = 10;

        // Center the buttons
        ImGui::SetCursorPosX((contentWidth - (2 * buttonWidth + spacing)) * 0.5f);

        if (ImGui::Button("Load", ImVec2(buttonWidth, 0))) {
            if (!m_selectedFilePath.empty()) {
                if (loadImage(m_selectedFilePath)) {
                    m_showLoadDialog = false;
                }
            }
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_showLoadDialog = false;
            m_selectedFilePath.clear();
        }

        ImGui::End();
    }
}

void ImageProcessor::handleSaveDialog() {
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(
        ImVec2(m_windowWidth * 0.5f, m_windowHeight * 0.5f),
        ImGuiCond_FirstUseEver,
        ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Save Image##Dialog", &m_showSaveDialog)) {
        ImGui::Text("Current Path: %s", m_currentPath.c_str());
        ImGui::Separator();

        // File browser
        if (ImGui::BeginChild("Files", ImVec2(0, -90), true)) {
            if (ImGui::Button("..")) {
                fs::path current(m_currentPath);
                if (current.has_parent_path()) {
                    m_currentPath = current.parent_path().string();
                }
            }

            try {
                for (const auto& entry : fs::directory_iterator(m_currentPath)) {
                    const auto& path = entry.path();
                    std::string filename = path.filename().string();

                    if (fs::is_directory(entry)) {
                        if (ImGui::Selectable((filename + "/").c_str())) {
                            m_currentPath = path.string();
                        }
                    }
                    else if (path.extension() == ".png") {
                        if (ImGui::Selectable(filename.c_str())) {
                            strcpy_s(m_inputFilename, sizeof(m_inputFilename), filename.c_str());
                        }
                    }
                }
            }
            catch (const std::exception& e) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Error: %s", e.what());
            }

            ImGui::EndChild();
        }

        // Filename input
        ImGui::Text("Filename:");
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##filename", m_inputFilename, sizeof(m_inputFilename))) {
            // Ensure filename ends with .png
            std::string filename = m_inputFilename;
            if (!filename.empty() && filename.substr(filename.length() - 4) != ".png") {
                filename += ".png";
                strcpy_s(m_inputFilename, sizeof(m_inputFilename), filename.c_str());
            }
        }

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);
        float contentWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = 120;
        float spacing = 10;

        // Center the buttons
        ImGui::SetCursorPosX((contentWidth - (2 * buttonWidth + spacing)) * 0.5f);

        if (ImGui::Button("Save", ImVec2(buttonWidth, 0))) {
            std::string filename = m_inputFilename;
            if (!filename.empty()) {
                // Ensure .png extension
                if (filename.substr(filename.length() - 4) != ".png") {
                    filename += ".png";
                }
                fs::path savePath = fs::path(m_currentPath) / filename;
                if (saveImage(savePath.string())) {
                    m_showSaveDialog = false;
                }
            }
        }
        ImGui::SameLine(0, spacing);
        if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            m_showSaveDialog = false;
        }

        ImGui::End();
    }
}

bool ImageProcessor::loadImage(const std::string& path) {
    std::ifstream inFile(path, std::ios::binary | std::ios::ate);
    if (!inFile) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: Cannot open file %s", path.c_str());
        return false;
    }

    // Get file size for progress calculation
    std::streamsize fileSize = inFile.tellg();
    inFile.seekg(0, std::ios::beg);

    // Clear existing data
    m_imgData.clear();
    
    // Get filename for status updates
    std::string filename = std::filesystem::path(path).filename().string();
    size_t bytesRead = 0;
    std::string line;

    // Initialize statistics
    m_minValue = 65535;  // Max possible value for uint16_t
    m_maxValue = 0;      // Min possible value for uint16_t
    double sum = 0.0;
    size_t totalPixels = 0;

    // Read file line by line
    while (std::getline(inFile, line)) {
        bytesRead += line.length() + 1;  // +1 for newline
        float progress = (float)bytesRead / fileSize * 100.0f;

        // Update progress in status bar
        snprintf(m_statusText, sizeof(m_statusText),
            "Loading %s... %.1f%%", filename.c_str(), progress);

        // Process line
        std::vector<uint16_t> row;
        std::stringstream ss(line);
        uint32_t value;

        while (ss >> value) {
            // Ensure 16-bit range
            uint16_t pixel = static_cast<uint16_t>(std::min(value, uint32_t(65535)));
            row.push_back(pixel);

            // Update statistics
            m_minValue = std::min(m_minValue, pixel);
            m_maxValue = std::max(m_maxValue, pixel);
            sum += pixel;
            totalPixels++;
        }

        if (!row.empty()) {
            m_imgData.push_back(std::move(row));
        }
    }

    // Check if we got any valid data
    if (m_imgData.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: No valid data in file");
        return false;
    }

    // Check if all rows have the same size
    size_t firstRowSize = m_imgData[0].size();
    for (size_t i = 1; i < m_imgData.size(); ++i) {
        if (m_imgData[i].size() != firstRowSize) {
            snprintf(m_statusText, sizeof(m_statusText), 
                "Error: Inconsistent row lengths in image data");
            m_imgData.clear();
            return false;
        }
    }

    // Calculate mean value
    m_meanValue = totalPixels > 0 ? sum / totalPixels : 0.0;

    // Store original image for potential reset
    m_originalImg = m_imgData;

    // Update the display
    updateDisplayImage();

    // Update status with image info
    snprintf(m_statusText, sizeof(m_statusText),
        "Loaded: %s (%zux%zu) Range:[%u,%u] Mean:%.1f",
        filename.c_str(),
        m_imgData[0].size(), m_imgData.size(),
        m_minValue, m_maxValue, m_meanValue);

    //convertDataToTexture();

    return true;
}

bool ImageProcessor::saveImage(const std::string& path) {
    if (m_imgData.empty()) {
        snprintf(m_statusText, sizeof(m_statusText), "Error: No image to save");
        return false;
    }

    int width = m_imgData[0].size();
    int height = m_imgData.size();

    // Allocate memory for pixel data (single channel)
    std::vector<unsigned char> pixels(width * height);

    // Convert 16-bit grayscale to 8-bit grayscale
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float normalized = static_cast<float>(m_imgData[y][x] - m_minValue) /
                static_cast<float>(m_maxValue - m_minValue);
            pixels[y * width + x] = static_cast<unsigned char>(normalized * 255.0f);
        }
    }

    // Save as PNG using stb_image_write (grayscale)
    if (!stbi_write_png(path.c_str(), width, height, 1, pixels.data(), width)) {
        snprintf(m_statusText, sizeof(m_statusText),
            "Error: Failed to save PNG file %s", path.c_str());
        return false;
    }

    snprintf(m_statusText, sizeof(m_statusText), "Saved: %s", path.c_str());
    return true;
}

void ImageProcessor::convertDataToTexture() {
    if (m_imgData.empty() || m_imgData[0].empty()) return;

    int width = m_imgData[0].size();
    int height = m_imgData.size();

    // Generate single channel 8-bit image data
    std::vector<unsigned char> pixels(width * height);

    // Convert 16-bit data to 8-bit and normalize
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float normalized = static_cast<float>(m_imgData[y][x] - m_minValue) /
                static_cast<float>(m_maxValue - m_minValue);
            pixels[y * width + x] = static_cast<unsigned char>(normalized * 255.0f);
        }
    }

    // Delete existing texture if it exists
    if (m_displayTexture) {
        glDeleteTextures(1, &m_displayTexture);
    }

    // Create new OpenGL texture
    glGenTextures(1, &m_displayTexture);
    glBindTexture(GL_TEXTURE_2D, m_displayTexture);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Set swizzle mask to show grayscale properly
    GLint swizzleMask[] = { GL_RED, GL_RED, GL_RED, GL_ONE };
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

    // Upload texture data as single channel
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0,
        GL_RED, GL_UNSIGNED_BYTE, pixels.data());
}

void ImageProcessor::drawMainWindow() {
    // Calculate control panel dimensions
    // Keep the panel height minimal to maximize image viewing area
    float controlPanelHeight = ImGui::GetFrameHeightWithSpacing() * 5; // Three rows of controls
    float windowPadding = 1.0f;

    // Set window flags to make the control panel behave as desired
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoResize |    // Prevent resizing
        ImGuiWindowFlags_NoMove |         // Prevent moving
        ImGuiWindowFlags_NoCollapse |     // Prevent collapsing
        ImGuiWindowFlags_NoBringToFrontOnFocus; // Keep z-order

    // Position the control panel at the top of the window
    ImGui::SetNextWindowPos(ImVec2(windowPadding, windowPadding));
    ImGui::SetNextWindowSize(ImVec2(m_windowWidth - 2 * windowPadding, controlPanelHeight));

    if (ImGui::Begin("Controls", nullptr, windowFlags)) {
        // Create two-column layout
        ImGui::Columns(2, nullptr, false);

        // Left column: File Operations
        ImGui::Text("File Operations");
        float columnWidth = ImGui::GetContentRegionAvail().x - 10.0f;
        float buttonWidth = (columnWidth - 10.0f) / 2;
        float buttonHeight = ImGui::GetFrameHeight();

        // Load and Save buttons
        if (ImGui::Button("Load Image", ImVec2(buttonWidth, buttonHeight))) {
            m_showLoadDialog = true;
        }
        ImGui::SameLine(0, 10.0f);
        if (ImGui::Button("Save Image", ImVec2(buttonWidth, buttonHeight))) {
            if (!m_imgData.empty()) {
                m_showSaveDialog = true;
            }
        }

        ImGui::NextColumn();

        // Right column: View Controls
        ImGui::Text("View Controls");
        columnWidth = ImGui::GetContentRegionAvail().x - 10.0f;
        
        // First row: Zoom controls with Fit to View
        float zoomButtonWidth = (columnWidth - 20.0f) / 2;  // Adjusted for 2 buttons
        float currentZoom = m_view.getZoom();
        ImGui::Text("Zoom: %.2fx", currentZoom);

        if (ImGui::Button("-", ImVec2(zoomButtonWidth, buttonHeight))) {
            m_view.zoom(0.9f); // Zoom out
        }
        ImGui::SameLine(0, 10.0f);
        if (ImGui::Button("+", ImVec2(zoomButtonWidth, buttonHeight))) {
            m_view.zoom(1.1f); // Zoom in
        }

        // Second row: Fit to View button
        if (ImGui::Button("Fit to View", ImVec2(columnWidth, buttonHeight))) {
            resetView(); // Fit image to view
        }

        ImGui::Columns(1);
    }
    ImGui::End();
}

void ImageProcessor::run() {
    while (!glfwWindowShouldClose(m_window) && m_running) {
        // Process window and input events
        glfwPollEvents();

        // Initialize ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Clear the entire window with background color
        glViewport(0, 0, m_windowWidth, m_windowHeight);
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw control panel first to get its actual height
        drawMainWindow();

        // Calculate control panel height based on its collapsed state
        float controlPanelHeight = m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 4);

        // Calculate viewport dimensions for the image area
        // The image viewport starts right after the control panel
        int imageViewportY = static_cast<int>(controlPanelHeight);
        int imageViewportHeight = m_windowHeight - imageViewportY;

        // Set OpenGL viewport for image rendering
        // This ensures the image is rendered in the correct area
        glViewport(0, imageViewportY, m_windowWidth, imageViewportHeight);

        // Update the view with new viewport dimensions
        // This ensures proper aspect ratio and scaling
        m_view.setViewportSize(m_windowWidth, imageViewportHeight);

        // Render the scene (image and any overlays)
        m_scene.draw(m_view.getViewMatrix());

        // Handle any open dialogs
        if (m_showLoadDialog) handleLoadDialog();
        if (m_showSaveDialog) handleSaveDialog();

        // Render ImGui overlay elements
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap front and back buffers
        //glfwSwapBuffers(m_window);
    }
}

void ImageProcessor::updateDisplayImage() {
    if (m_imgData.empty() || m_imgData[0].empty()) return;

    int height = m_imgData.size();
    int width = m_imgData[0].size();

    // Convert 2D image data to 1D texture data
    std::vector<uint16_t> textureData;
    textureData.reserve(width * height);
    for (const auto& row : m_imgData) {
        textureData.insert(textureData.end(), row.begin(), row.end());
    }

    // Update the texture item with new image data
    m_imageItem->setImage(textureData, width, height);

    // Set zoom to 1.0 first
    m_view.setZoom(1.0f);

    // Then reset and center the view
    resetView();
}

void ImageProcessor::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto app = static_cast<ImageProcessor*>(glfwGetWindowUserPointer(window));
    app->m_windowWidth = width;
    app->m_windowHeight = height;
    app->m_view.setViewportSize(width, height);
    glViewport(0, 0, width, height);
}

void ImageProcessor::glfwErrorCallback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void ImageProcessor::resetView() {
    if (!m_imgData.empty()) {
        // Always set zoom to 1.0 first
        m_view.setZoom(1.0f);

        // Then fit in view with 1.0 zoom
        m_view.setViewMatrix(glm::mat4(1.0f)); // Reset view matrix through GraphicsView
        m_view.fitInView(glm::vec4(0, 0, m_imgData[0].size(), m_imgData.size()));

        m_panX = 0.0f;
        m_panY = 0.0f;
        m_lastPanX = 0.0f;
        m_lastPanY = 0.0f;
    }
}

void ImageProcessor::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto app = static_cast<ImageProcessor*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    // Get current mouse position
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    // Calculate control panel height and padding
    float controlPanelHeight = app->m_windowHeight * 0.20f;
    float padding = 10.0f;

    // Calculate viewport area
    float viewportY = app->m_controlWindowCollapsed ? padding : controlPanelHeight + 2 * padding;

    // Check if mouse is in the image viewport area
    if (mouseX >= padding &&
        mouseX <= app->m_windowWidth - padding &&
        mouseY >= viewportY &&
        mouseY <= app->m_windowHeight - padding) {

        float zoomFactor = (yoffset > 0) ? 1.1f : 0.9f;
        app->m_view.zoom(zoomFactor);
    }
}

void ImageProcessor::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto app = static_cast<ImageProcessor*>(glfwGetWindowUserPointer(window));
    if (!app) return;

    if (ImGui::GetIO().WantCaptureMouse) {
        return;
    }

    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    float controlPanelHeight = app->m_controlWindowCollapsed ? 0.0f : (ImGui::GetFrameHeightWithSpacing() * 4);

    if (mouseY > controlPanelHeight) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                app->m_isPanning = true;
                app->m_lastMouseX = mouseX;
                app->m_lastMouseY = mouseY;

                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_HAND_CURSOR));
            }
            else if (action == GLFW_RELEASE) {
                app->m_isPanning = false;

                glfwSetCursor(window, glfwCreateStandardCursor(GLFW_ARROW_CURSOR));
            }
        }
    }
}

void ImageProcessor::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    auto app = static_cast<ImageProcessor*>(glfwGetWindowUserPointer(window));
    if (!app || !app->m_isPanning) return;

    float deltaX = static_cast<float>(xpos - app->m_lastMouseX);
    float deltaY = static_cast<float>(ypos - app->m_lastMouseY);

    const float panSpeedFactor = 0.05f; 

    float ndcDeltaX = (deltaX / (app->m_windowWidth * 0.5f)) * panSpeedFactor;
    float ndcDeltaY = (deltaY / (app->m_windowHeight * 0.5f)) * panSpeedFactor;

    float zoom = app->m_view.getActualZoom();
    if (zoom != 0.0f) {
        ndcDeltaX /= zoom;
        ndcDeltaY /= zoom;
    }

    app->m_view.pan(glm::vec2(ndcDeltaX, -ndcDeltaY));

    app->m_lastMouseX = xpos;
    app->m_lastMouseY = ypos;
}