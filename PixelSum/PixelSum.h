#pragma once

#include <vector>

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
typedef struct Vector4<int> PixelBufferCoords_i; // Pixel buffer's (x0, y0), (x1, y1), _i for type int
typedef struct Vector4<int> PixBufTLBR_i;        // Pixel buffer's top, left bottom, right, _i for type int

class PixelSum
{
public:
    PixelSum() = default;
    PixelSum(const unsigned char* buffer, int xWidth, int yHeight);
    ~PixelSum(void);

    PixelSum(const PixelSum& p_PixelSum);
    PixelSum& operator= (const PixelSum& p_PixelSum);

    unsigned int getPixelSum(int x0, int y0, int x1, int y1) const;
    double getPixelAverage(int x0, int y0, int x1, int y1) const;

    int getNonZeroCount(int x0, int y0, int x1, int y1) const;
    double getNonZeroAverage(int x0, int y0, int x1, int y1) const;

private:
    template<typename T>
    bool allocateVirtualMemoryForSumAreaMatrix(T*& p_SumAreaMatrix, size_t p_AllocSize);

    enum PixelSumPassType
    {
        HorizontalPass = (1u << 0u),
        VerticalPass   = (1u << 1u),
    };
    bool computeSumAreaMatrixForPixelBufferPass(PixelSumPassType p_IsHPass, const unsigned char* p_PixelBuffer, uint32_t* p_SumAreaPixBuf, uint32_t* p_SumAreaNonZero);

    template<typename T>
    unsigned int computeSumAreaForSearchWindow(int x0, int y0, int x1, int y1, T* p_SumArea) const;

private:
    PixBufTLBR_i m_SourcePixBufTLBR;

    // Sum area of pixel buffer, since max size can be 4096x4096 with highest possible val 255,
    // the unsigned 32bit storage is more than enough
    uint32_t* m_SumAreaPixBuf = nullptr;

    // Sum area of pixel buffer's non-zero components, the non-zero element can be marked with 1 and zero with 0.
    // With 4096x4096 and 1 as max possible value the sum cannot go greater that unsigned 16bit storage max value range.
    uint32_t* m_SumAreaNonZero = nullptr; // TODO mention the reason why
};

// What should the the data type of SumArea
// TODO: Doygen doc comments


// Test cases:
// Write test case for boundary checks - -1,-1 to 0, 0
// Check if Nan return type can be possible
// https://github.com/gfx/cpp-scoped_timer/blob/master/example/rope.cpp - do it
