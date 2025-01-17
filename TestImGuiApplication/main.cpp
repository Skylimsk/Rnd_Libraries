#include "imageprocessor.h"
#include <Windows.h>
#include <iostream>

// Windows entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    try {
        ImageProcessor app;
        app.run();
        return 0;
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
    catch (...) {
        MessageBoxA(NULL, "Unknown error occurred", "Error", MB_OK | MB_ICONERROR);
        return -1;
    }
}