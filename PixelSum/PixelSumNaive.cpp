#include "PixelSumNaive.h"

#include "UtilityFunctions.h"

PixelSumNaive::PixelSumNaive(const void* p_Buffer, int p_Width, int p_Height)
    : m_Buffer(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(p_Buffer)))
    , m_Width(p_Width)
    , m_Height(p_Height)
{
}

PixelSumNaive::~PixelSumNaive()
{
}

// Assuming coordinate system in Scree Coords, X increases left to right, Y increase top to bottom
unsigned int PixelSumNaive::GetPixelSum(int x0, int y0, int x1, int y1) const
{
    if (!m_Buffer && !ValidateCoordinates(x0, y0, x1, y1)) return 0;

    int searchWidowWidth  = std::min((x1 - x0) + 1, m_Width);
    int searchWidowHeight = std::min((y1 - y0) + 1, m_Height);

    // what if window is larger than image buffer
    unsigned char* searchWindowCurrentPixel = nullptr;
    unsigned char* searchWindowTopLeftPixel = m_Buffer + (y0 * m_Width + x0);

    int pixelSum = 0;
    for (int row = 0; row < searchWidowHeight; row++)
    {
        searchWindowCurrentPixel = searchWindowTopLeftPixel + (row * m_Width);
        for (int col = 0; col < searchWidowWidth; col++)
        {
            pixelSum += *searchWindowCurrentPixel;
            searchWindowCurrentPixel++;
        }
    }

    return pixelSum;
}

unsigned int PixelSumNaive::GetNonZeroCount(int x0, int y0, int x1, int y1) const
{
    if (!m_Buffer && !ValidateCoordinates(x0, y0, x1, y1)) return 0;

    int searchWidowWidth  = std::min((x1 - x0) + 1, m_Width);
    int searchWidowHeight = std::min((y1 - y0) + 1, m_Height);

    // what if window is larger than image buffer
    unsigned char* searchWindowCurrentPixel = nullptr;
    unsigned char* searchWindowTopLeftPixel = m_Buffer + (y0 * m_Width + x0);

    int nonZeroSum = 0;
    for (int row = 0; row < searchWidowHeight; row++)
    {
        searchWindowCurrentPixel = searchWindowTopLeftPixel + (row * m_Width);
        for (int col = 0; col < searchWidowWidth; col++)
        {
            nonZeroSum += (*searchWindowCurrentPixel == 0 ? 0 : 1);
            searchWindowCurrentPixel++;
        }
    }

    return nonZeroSum;
}

bool PixelSumNaive::ValidateCoordinates(int& x0, int& y0, int& x1, int& y1) const
{
    x0 = g_Clamp(x0, 0 , m_Width);
    y0 = g_Clamp(y0, 0 , m_Height);
    x1 = g_Clamp(x1, 0 , m_Width);
    y1 = g_Clamp(y1, 0 , m_Height);

    if (x1 < x0)
    {
        std::swap(x0, x1);
    }

    if (y1 < y0)
    {
        std::swap(y0, y1);
    }

    return ((((x1 - x0) + 1) * ((y1 - y0) + 1)) != 0); // Area
}
