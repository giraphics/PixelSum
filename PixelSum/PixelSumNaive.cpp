#include "PixelSumNaive.h"

#include "UtilityFunctions.h"

PixelSumNaive::PixelSumNaive(const void* p_Buffer, int p_Width, int p_Height)
    : m_Buffer(const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(p_Buffer)))
    , m_SourcePixBufTLBR(0 /*Top Coord*/, 0/*Left Coord*/, p_Height - 1/*Bottom Coord*/, p_Width - 1/*Right Coord*/)
//    , m_Width(p_Width)
//    , m_Height(p_Height)
{
}

PixelSumNaive::~PixelSumNaive()
{
}

// Assuming coordinate system in Scree Coords, X increases left to right, Y increase top to bottom
int PixelSumNaive::GetPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
//    if (!m_Buffer && !ValidateCoordinates(x0, y0, x1, y1)) return 0;
    if (!m_Buffer || !s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR)) return 0;

    int pixelBufferWidth  = m_SourcePixBufTLBR.width();
    int pixelBufferHeight = m_SourcePixBufTLBR.height();

    int searchWidowWidth  = std::min((p_X1 - p_X0) + 1, pixelBufferWidth);
    int searchWidowHeight = std::min((p_Y1 - p_Y0) + 1, pixelBufferHeight);
//    int searchWidowWidth  = std::min((x1 - x0) + 1, m_SourcePixBufTLBR.width());
//    int searchWidowHeight = std::min((y1 - y0) + 1, m_SourcePixBufTLBR.height());

    // what if window is larger than image buffer
    unsigned char* searchWindowCurrentPixel = nullptr;
    unsigned char* searchWindowTopLeftPixel = m_Buffer + (p_Y0 * pixelBufferWidth + p_X0);
//    unsigned char* searchWindowTopLeftPixel = m_Buffer + (y0 * m_SourcePixBufTLBR.width() + x0);

    int pixelSum = 0;
    for (int row = 0; row < searchWidowHeight; row++)
    {
        searchWindowCurrentPixel = searchWindowTopLeftPixel + (row * pixelBufferWidth);
        for (int col = 0; col < searchWidowWidth; col++)
        {
            pixelSum += *searchWindowCurrentPixel;
            searchWindowCurrentPixel++;
        }
    }

    return pixelSum;
}

int PixelSumNaive::GetNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    if (!m_Buffer || !s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR)) return 0;

    int pixelBufferWidth  = m_SourcePixBufTLBR.width();
    int pixelBufferHeight = m_SourcePixBufTLBR.height();

    int searchWidowWidth  = std::min((p_X1 - p_X0) + 1, pixelBufferWidth);
    int searchWidowHeight = std::min((p_Y1 - p_Y0) + 1, pixelBufferHeight);

    // what if window is larger than image buffer
    unsigned char* searchWindowCurrentPixel = nullptr;
    unsigned char* searchWindowTopLeftPixel = m_Buffer + (p_Y0 * pixelBufferWidth + p_X0);

    int nonZeroSum = 0;
    for (int row = 0; row < searchWidowHeight; row++)
    {
        searchWindowCurrentPixel = searchWindowTopLeftPixel + (row * pixelBufferWidth);
        for (int col = 0; col < searchWidowWidth; col++)
        {
            nonZeroSum += (*searchWindowCurrentPixel == 0 ? 0 : 1);
            searchWindowCurrentPixel++;
        }
    }

    return nonZeroSum;
}

double PixelSumNaive::GetPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0; // Prevent return Nan

    return GetPixelSum(p_X0, p_Y0, p_X1, p_Y1) / static_cast<double>(searchWindowPixelCount);
}
