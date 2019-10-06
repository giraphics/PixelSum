#pragma once

template<typename T>
struct Vector4
{
    Vector4() = default;
    Vector4(float p_Top, float p_Left, float p_Bottom, float p_Right)
        : top(p_Top), left(p_Left), bottom(p_Bottom), right(p_Right) {}

    int width() const { return right + 1; }
    int height() const { return bottom + 1; }

    union
    {
        struct{ T top, left, bottom, right; };
        struct{ T y0 ,   x0,     y1,    x1; };
    };
};
typedef struct Vector4<int> PixelBufferCoords_i; // Pixel buffer's (x0, y0), (x1, y1) representation, _i for type int
typedef struct Vector4<int> PixBufTLBR_i;        // Pixel buffer's top, left bottom, right representation, _i for type int
