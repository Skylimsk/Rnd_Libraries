#include "pch.h"

class ImageReader {
public:

    static void ReadTextToU2D(const std::string& filename, double**& matrix, int& rows, int& cols);

    static void ReadTextToU1D(const std::string& filename, uint32_t*& matrix, int& rows, int& cols);

    static bool SaveImage(const std::string& filepath, double** data, int totalRow, int totalCol, int maxValue);
    static bool SaveImage(const std::string& filepath, const CGData64& imageMatrix);
};

