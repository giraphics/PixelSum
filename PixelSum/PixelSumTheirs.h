#pragma once

#include <vector>

//----------------------------------------------------------------------------
// Class for providing fast region queries from an 8-bit pixel buffer.
// Note: all coordinates are *inclusive* and clamped internally to the borders
// of the buffer by the implementation.
//
// For example: getPixelSum(4,8,7,10) gets the sum of a 4x3 region where top left
// corner is located at (4,8) and bottom right at (7,10). In other words
// all coordinates are _inclusive_.
//
// If the resulting region after clamping is empty, the return value for all
// functions should be 0.
//
// The width and height of the buffer dimensions < 4096 x 4096.
//----------------------------------------------------------------------------
class PixelSumTheirs
{
public:
    PixelSumTheirs (const unsigned char* buffer, int xWidth, int yHeight);
    ~PixelSumTheirs (void) = default;

    PixelSumTheirs (const PixelSumTheirs&) = default;
    PixelSumTheirs& operator= (const PixelSumTheirs&) = default;

    unsigned int getPixelSum (int x0, int y0, int x1, int y1) const;
    double getPixelAverage (int x0, int y0, int x1, int y1) const;
    int getNonZeroCount (int x0, int y0, int x1, int y1) const;
    double getNonZeroAverage (int x0, int y0, int x1, int y1) const;

    inline void validateCoordinates(int& x0, int& y0, int& x1, int& y1, int width, int height);

private:
    int width;
    int height;
    std::vector<unsigned int> summedAreaTable; // Pammy1: Allocation every time.... this kills
    std::vector<unsigned int> summedZeroTable;
};
