#pragma once

#include <stddef.h>
#include <iostream>

#include "CustomTypes.h"
#include "PixelSum.h"

// std::clamp is C++ 17 feature, the local implementation will sallow to run with older C++ versions
template<class T>
constexpr const T& g_Clamp( const T& v, const T& low, const T& high )
{
    return (v < low) ? low : (high < v) ? high : v;
}

// Return total number of pixels for a valid search window, for invalid search window return 0
static uint32_t inline s_ValidateSearchWindowClipCoords(int& x0, int& y0, int& x1, int& y1, const PixBufTLBR_i& sourcePixBufTLBR)
{
    if (sourcePixBufTLBR.right * sourcePixBufTLBR.bottom  == 0 ||
        x1 < 0                                                 ||
        x0 > sourcePixBufTLBR.right                            ||
        y1 < 0                                                 ||
        y0 > sourcePixBufTLBR.bottom)
    {
        return 0;
    }

    if (x1 < x0) { std::swap(x0, x1); }
    if (y1 < y0) { std::swap(y0, y1); }

    // Must compute the search window total pixel before clamping
    const uint32_t searchWindowTotalPixels = (x1 - x0 + 1) * (y1 - y0 + 1);

    x0 = g_Clamp(x0, 0, sourcePixBufTLBR.right);
    x1 = g_Clamp(x1, 0, sourcePixBufTLBR.right);
    y0 = g_Clamp(y0, 0, sourcePixBufTLBR.bottom);
    y1 = g_Clamp(y1, 0, sourcePixBufTLBR.bottom);

    return searchWindowTotalPixels; // searchWindowWidth * searchWindowHeight
}

template<typename T>
static void s_PrintSummedAreaMatrix(const T* p_SumArea, int p_Width, int p_Height)
{
    std::cout << "---------------" << std::endl;
    for (int row = 0; row < p_Height; row++)
    {
        std::cout << std::endl;
        for (int col = 0; col < p_Width; col++)
        {
            std::cout << "[" << (T)p_SumArea[row * p_Width + col] << "]";
        }
        std::cout << ", ";
    }

    std::cout << std::endl;
}

static void s_FillDataWithContinousNumberStartingWith(size_t p_Size, unsigned char* p_DataPtr, size_t p_StartVal)
{
    for (size_t i = 0; i < p_Size; i++) { p_DataPtr[i] = (p_StartVal + i) % 255; }
}

static void s_FillHalfPixelBufferWithConstValue(size_t p_Size, unsigned char* p_DataPtr, uint8_t p_Val)
{
    for (size_t i = 0; i < p_Size; i++) { p_DataPtr[i] = (i % 2) ? p_Val : 0; }
}

static void s_FillFullPixelBufferWithConstValue(size_t p_Size, unsigned char* p_DataPtr, uint8_t p_Val)
{
    for (size_t i = 0; i < p_Size; i++) { p_DataPtr[i] = p_Val; }
}
