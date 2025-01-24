# Rnd_Libraries

A comprehensive toolkit for UI graphics and visualization, featuring multiple implementations using different graphics and UI libraries.

## Features

Three main applications demonstrating various graphics and UI capabilities:

### TestApplication2
Advanced image manipulation tool built with ImGui and SDL
- Image loading/saving
- Zoom functionality (UI + Mouse wheel with Ctrl)
- Pan capability (Mouse with Alt)
- Image cropping 
- Calibration, interlace & merge operations
- 90-degree rotation (left/right)
- Real-time mouse coordinate tracking

**Tech Stack**: ImGui, Boost.Signals2, SDL, OpenGL

### TestImGuiApplication
Streamlined image viewer using GLFW
- Image loading/saving
- Zoom functionality (UI + Mouse wheel)
- Pan capability (Mouse with Ctrl)

**Tech Stack**: ImGui, Boost.Signals2, GLFW, GLAD, OpenGL

### TestWXWidgets
Alternative implementation using wxWidgets and SFML
- Image loading/saving
- Zoom functionality (UI + Mouse wheel)
- Pan capability (Mouse with Ctrl)

**Tech Stack**: wxWidgets, Boost.Signals2, SFML

## Dependencies
- ImGui
- Boost.Signals2
- SDL
- OpenGL
- GLFW
- GLAD
- wxWidgets
- SFML

## Known Issues
- Crop function: Currently experiencing issues with region accuracy

## Notes
Each implementation showcases different approaches to similar functionality, allowing for comparison of various graphics and UI libraries.

## Documentation
- [Installation/Linking Manual](https://billionprima365-my.sharepoint.com/:f:/g/personal/shikai_billionprima_com_my/Eia7AYJzsXFPuDE2BpSdgXQBs8NB2eI4FC3J222wfBtXqw)
