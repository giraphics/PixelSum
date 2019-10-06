#pragma once

#include "CustomTypes.h"

// Poor man's implementation for Pixel sum, no summed area.
class PixelSumNaive
{
public:
    PixelSumNaive(const void* p_Buffer, int p_Width, int p_Height);
    ~PixelSumNaive();

    int GetPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    int GetNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    double GetPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const;

private:
    unsigned char* m_Buffer = nullptr;
    PixBufTLBR_i m_SourcePixBufTLBR;
};
