#include "PixelSumTheirs.h"

#include <algorithm> // clamp
#include <iostream>

template<class T>
constexpr const T& clamp( const T& v, const T& lo, const T& hi )
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

PixelSumTheirs::PixelSumTheirs(const unsigned char* buffer, int xWidth, int yHeight)
: width{xWidth},
  height{xWidth},
  summedAreaTable(xWidth * yHeight),
  summedZeroTable(xWidth * yHeight)
{
    // TODO: handle buffer nullptr
    for (auto row = 0; row < yHeight; ++row) {
        for (auto col = 0; col < xWidth; ++col) {
            const auto i = row * xWidth + col;
            const auto pixel = buffer[i];

            auto sumUp = 0;
            auto zeroUp = 0;
            if (row > 0) {
                sumUp = summedAreaTable[i - xWidth];
                zeroUp = summedZeroTable[i - xWidth];
            }
            
            auto sumLeft = 0;
            auto zeroLeft = 0;
            if (col > 0) {
                sumLeft = summedAreaTable[i - 1];
                zeroLeft = summedZeroTable[i - 1];
            }
            
            auto sumUpLeft = 0;
            auto zeroUpLeft = 0;
            if (row > 0 && col > 0) {
                sumUpLeft = summedAreaTable[i - xWidth - 1];
                zeroUpLeft = summedZeroTable[i - xWidth - 1];
            }

            summedAreaTable[i] = (sumUp + sumLeft - sumUpLeft + pixel);
            summedZeroTable[i] = (zeroUp + zeroLeft - zeroUpLeft + ((pixel != 0) ? 1 : 0));
        }
    }
}

static inline void orderCoordinates(int& x0, int& y0, int& x1, int& y1)
{
    if (x0 > x1) {
        std::swap(x0, x1);
    }
    if (y0 > y1) {
        std::swap(y0, y1);
    }
}

/**
 * Returns true if x is in [min, max). Exclusive of max.
 */
static inline bool isInRange(int x, int min, int max)
{
    return (min <= x && x < max);
}

/**
 * Returns true if [(x0,y0) , (x1,y1)] intersects with [(minX, minY), (maxX, maxY)).
 * Exclusive of the maxX, maxY edges (i.e. the right, bottom edges).
 */
static inline bool hasIntersection(int x0, int y0, int x1, int y1,
                                   int minX, int minY, int maxX, int maxY)
{
    return !((x0 < minX && x1 < minX)
             || (x0 > maxX && x1 > maxX)
             || (y0 < minY && y1 < minY)
             || (y0 > maxY && y1 > maxY));
}

static inline void clampCoordinates(int& x0, int& y0, int& x1, int& y1, int width, int height)
{
    const auto maxX = width - 1;
    const auto maxY = height - 1;
    x0 = /*std::*/clamp(x0, 0, maxX);
    x1 = /*std::*/clamp(x1, 0, maxX);
    y0 = /*std::*/clamp(y0, 0, maxY);
    y1 = /*std::*/clamp(y1, 0, maxY);
}

static inline unsigned int lookupSumTable(int x0, int y0,
                                          int width, int height,
                                          const std::vector<unsigned int>& table)
{
    if (!isInRange(x0, 0, width)
        || !isInRange(y0, 0, height)) {
        return 0;
    }
    return table[y0 * width + x0];
}

static inline unsigned int lookupSumTable(int x0, int y0, int x1, int y1,
                                          int width, int height,
                                          const std::vector<unsigned int>& table)
{
    if (!hasIntersection(x0, y0, x1, y1, 0, 0, width, height)) {
        return 0;
    }
    clampCoordinates(x0, y0, x1, y1, width, height);
    const auto leftTop = lookupSumTable(x0 - 1, y0 - 1, width, height, table);
    const auto leftBottom = lookupSumTable(x0 - 1, y1, width, height, table);
    const auto rightTop = lookupSumTable(x1, y0 - 1, width, height, table);
    const auto rightBottom = lookupSumTable(x1, y1, width, height, table);
    return rightBottom - rightTop - leftBottom + leftTop;
}

unsigned int PixelSumTheirs::getPixelSum(int x0, int y0, int x1, int y1) const
{
    orderCoordinates(x0, y0, x1, y1);
    return lookupSumTable(x0, y0, x1, y1, width, height, summedAreaTable);
}

double PixelSumTheirs::getPixelAverage(int x0, int y0, int x1, int y1) const
{
    orderCoordinates(x0, y0, x1, y1);
    const auto w = x1 - x0 + 1;
    const auto h = y1 - y0 + 1;
    return getPixelSum(x0, y0, x1, y1) / static_cast<double>(w * h);
}

int PixelSumTheirs::getNonZeroCount(int x0, int y0, int x1, int y1) const
{
    // Pammy: can we use the sparse matrix representation for store the non-zero, think over?
    orderCoordinates(x0, y0, x1, y1);
    return lookupSumTable(x0, y0, x1, y1, width, height, summedZeroTable);
}

double PixelSumTheirs::getNonZeroAverage(int x0, int y0, int x1, int y1) const
{
    orderCoordinates(x0, y0, x1, y1);
    const auto nonZeroCount = getNonZeroCount(x0, y0, x1, y1);
    if (nonZeroCount == 0) {
        return 0;
    }
    return getPixelSum(x0, y0, x1, y1) / static_cast<double>(nonZeroCount);
}

void PixelSumTheirs::validateCoordinates(int& x0, int& y0, int& x1, int& y1, int width, int height)
{
    const int right = width - 1;
    const int bottom = height - 1;
    x0 = clamp(x0, 0, right);
    x1 = clamp(x1, 0, right);
    y0 = clamp(y0, 0, bottom);
    y1 = clamp(y1, 0, bottom);
}
