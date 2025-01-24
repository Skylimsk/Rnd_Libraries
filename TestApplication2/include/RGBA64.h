#include <cstdint>
#include <iostream>

class RGBA64 {
private:
    static constexpr int RedShift = 48;
    static constexpr int GreenShift = 32;
    static constexpr int BlueShift = 16;
    static constexpr int AlphaShift = 0;

    std::uint16_t red;
    std::uint16_t green;
    std::uint16_t blue;
    std::uint16_t alpha;

public:
    RGBA64(std::uint16_t r = 0, std::uint16_t g = 0, std::uint16_t b = 0, std::uint16_t a = 0xFFFF);

    static RGBA64 fromRgba64(std::uint16_t red, std::uint16_t green, std::uint16_t blue, std::uint16_t alpha);
    static RGBA64 fromUint64(std::uint64_t value);

    std::uint16_t getRed() const { return red; }
    std::uint16_t getGreen() const { return green; }
    std::uint16_t getBlue() const { return blue; }
    std::uint16_t getAlpha() const { return alpha; }

    // Print function (for testing)
    //void print() const;
};

