#pragma once
#include <cstdint>

// Poor man's implementation for Pixel sum, no summed area.
class PixelSumNaive
{
public:
    PixelSumNaive(const void* p_Buffer, int p_Width, int p_Height);
    ~PixelSumNaive();

    unsigned int GetPixelSum(int x0, int y0, int x1, int y1) const;
    unsigned int GetNonZeroCount(int x0, int y0, int x1, int y1) const;

private:
    bool ValidateCoordinates(int& x0, int& y0, int& x1, int& y1) const;

private:
    unsigned char* m_Buffer = nullptr;

    int m_Width  = 0;
    int m_Height = 0;
};
