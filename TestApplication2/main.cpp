#define _CRT_SECURE_NO_WARNINGS

// GLEW must come first
#include <GL/glew.h>

// SDL headers
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

// Project headers
#include "imageprocessor.h"

// Standard headers
#include <Windows.h>
#include <iostream>
#include <stdexcept>

// Optional: Enable memory leak detection in debug mode
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try {
        // Enable memory leak detection in debug mode
#ifdef _DEBUG
        _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

        // Create and run the application
        ImageProcessor app;
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        // Handle known exceptions
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    catch (...) {
        // Handle unknown exceptions
        MessageBoxA(NULL, "Unknown error occurred", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}