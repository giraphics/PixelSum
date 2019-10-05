#include "PixelSum.h"

#include <iostream>

#include "MemoryAllocator.h"
#include "UtilityFunctions.h"
#include "ScopedTimer.h"

PixelSum::PixelSum(const unsigned char* buffer, int xWidth, int yHeight)
    : m_SourcePixBufTLBR(0 /*Top Coord*/, 0/*Left Coord*/, yHeight - 1/*Bottom Coord*/, xWidth - 1/*Right Coord*/)
{
    // Allocations are made from virtual memory, we avoid system memory to prevent performance hiches
    // for constant allocation and deallocation.
    const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaPixBuf, srcBufferPixelCount);
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZero, srcBufferPixelCount);

    {
        DefaultResults results;
        ScopedTimer Timer(results);
        // The summed matrix is computed in two pass Horizontal and Vertical
        computeSumAreaMatrixForPixelBufferPass(PixelSumPassType::HorizontalPass, buffer, m_SumAreaPixBuf, m_SumAreaNonZero);
        computeSumAreaMatrixForPixelBufferPass(PixelSumPassType::VerticalPass,   buffer, m_SumAreaPixBuf, m_SumAreaNonZero);
    }
}

PixelSum::~PixelSum()
{
    // Free the memory, this memory will return back to Virtual Memory free stack,
    // where it can be efficiently reused again and again without

    VM::MemoryAllocator::GetInstance().Free(m_SumAreaPixBuf);
    VM::MemoryAllocator::GetInstance().Free(m_SumAreaNonZero);
}

PixelSum::PixelSum(const PixelSum& p_PixelSum)
{
    m_SourcePixBufTLBR = p_PixelSum.m_SourcePixBufTLBR;

    const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();

    // Deep copy the sum areas
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaPixBuf, srcBufferPixelCount);
    memcpy(m_SumAreaPixBuf, p_PixelSum.m_SumAreaPixBuf, srcBufferPixelCount * sizeof(uint32_t));

    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZero, srcBufferPixelCount);
    memcpy(m_SumAreaNonZero, p_PixelSum.m_SumAreaNonZero, srcBufferPixelCount * sizeof(uint32_t));
}

PixelSum& PixelSum::operator=(const PixelSum& p_PixelSum)
{
    if (this != &p_PixelSum)
    {
        // 1. Free the existing summed area matrixes
        if (m_SumAreaPixBuf)
        {
            VM::MemoryAllocator::GetInstance().Free(m_SumAreaPixBuf);
        }

        if (m_SumAreaNonZero)
        {
            VM::MemoryAllocator::GetInstance().Free(m_SumAreaNonZero);
        }

        // 2. Overwrite the pixel buffer top-left and bottom-right
        m_SourcePixBufTLBR = p_PixelSum.m_SourcePixBufTLBR;

        // Perform Deep copy for both summed area matrix
        const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();

        allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaPixBuf, srcBufferPixelCount);
        memcpy(m_SumAreaPixBuf, p_PixelSum.m_SumAreaPixBuf, srcBufferPixelCount * sizeof(uint32_t));

        allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZero, srcBufferPixelCount);
        memcpy(m_SumAreaNonZero, p_PixelSum.m_SumAreaNonZero, srcBufferPixelCount * sizeof(uint32_t));
    }

    return *this;
}

/********************************************************************************
From: https://en.wikipedia.org/wiki/Summed-area_table

        0              1              2               3      A => Area((0,0) To (1, 1))
      0 +--------------+------------------------------+
        |              |                              |      B => Area((0,0) To (1, 3))
        |              |                              |
        |              |                              |      C => Area((0,0) To (3, 1))
        |              |                              |
        |              |                              |      D => Area((0,0) To (3, 3))
      1 +---------------------------------------------+
        |              |A                             |B
        |              |                              |      Summed Area(ABCD) => D - C - B + A
        |              |                              |
        |              |                              |
        |              |                              |
      2 |              |                              |
        |              |                              |
        |              |                              |
        |              |                              |
        |              |                              |
        |              |                              |
      3 +--------------+------------------------------+
                       C                              D

*********************************************************************************/
unsigned int PixelSum::getPixelSum(int x0, int y0, int x1, int y1) const
{
    if (!s_ValidateSearchWindowClipCoords(x0, y0, x1, y1, m_SourcePixBufTLBR)) return 0;

    return computeSumAreaForSearchWindow<uint32_t>(x0, y0, x1, y1, m_SumAreaPixBuf);
}

double PixelSum::getPixelAverage(int x0, int y0, int x1, int y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(x0, y0, x1, y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0;

    return computeSumAreaForSearchWindow<uint32_t>(x0, y0, x1, y1, m_SumAreaPixBuf) / static_cast<double>(searchWindowPixelCount);
}

int PixelSum::getNonZeroCount(int x0, int y0, int x1, int y1) const
{
    return computeSumAreaForSearchWindow<uint32_t>(x0, y0, x1, y1, m_SumAreaNonZero);
}

double PixelSum::getNonZeroAverage(int x0, int y0, int x1, int y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(x0, y0, x1, y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0;

    return computeSumAreaForSearchWindow<uint32_t>(x0, y0, x1, y1, m_SumAreaNonZero) / static_cast<double>(searchWindowPixelCount);
}

bool PixelSum::computeSumAreaMatrixForPixelBufferPass(PixelSumPassType p_PassType, const unsigned char* p_PixelBuffer, uint32_t* p_SumAreaPixBuf, uint32_t* p_SumAreaNonZero)
{
    if (!p_PixelBuffer || !p_SumAreaPixBuf || !p_SumAreaNonZero) return false;

    const int srcPixBufWidth  = m_SourcePixBufTLBR.width();
    const int srcPixBufHeight = m_SourcePixBufTLBR.height();

    if (srcPixBufWidth * srcPixBufHeight == 0) return false;

    uint32_t* pixeBufSATPtr = p_SumAreaPixBuf;
    uint32_t* nonZeroSATPtr = p_SumAreaNonZero;

    const uint8_t* pixelBufferPtr = p_PixelBuffer;
    for (int row = 0; row < srcPixBufHeight; row++)
    {
        const bool isFirstRow = (row == 0);
        for (int col = 0; col < srcPixBufWidth; col++)
        {
            if (p_PassType == PixelSumPassType::HorizontalPass)
            {
                const bool isFirstCol = (col == 0);
                *pixeBufSATPtr = *pixelBufferPtr;
                *pixeBufSATPtr += isFirstCol ? 0 : *(pixeBufSATPtr - 1);

                *nonZeroSATPtr = (*pixelBufferPtr == 0) ? 0 : 1;
                *nonZeroSATPtr += isFirstCol ? 0 : *(nonZeroSATPtr - 1);
            }
            else
            {
                *pixeBufSATPtr += isFirstRow ? 0 : *(pixeBufSATPtr - srcPixBufWidth);
                *nonZeroSATPtr += isFirstRow ? 0 : *(nonZeroSATPtr - srcPixBufWidth);
            }

            pixelBufferPtr++;

            pixeBufSATPtr++;
            nonZeroSATPtr++;
        }
    }
}

template<typename T>
bool PixelSum::allocateVirtualMemoryForSumAreaMatrix(T*& p_SumAreaMatrix, size_t p_AllocSize)
{
    p_SumAreaMatrix = static_cast<T*>(VM::MemoryAllocator::GetInstance().Allocate(p_AllocSize * sizeof(T)));
    return p_SumAreaMatrix != nullptr;
}

template<typename T>
unsigned int PixelSum::computeSumAreaForSearchWindow(int x0, int y0, int x1, int y1, T* p_SumArea) const
{
    const T* sumAreaPtr = p_SumArea;
    const int srcPixBufWidth  = m_SourcePixBufTLBR.width();

    const bool isX0AtFirstRow = (x0 == 0); // true: Area A and B is zero, no need to compute A and B
    const bool isY0AtFirstCol = (y0 == 0); // true: Area A and C is zero, no need to compute A and C

    uint32_t x0Left = (isX0AtFirstRow ? 0 : (x0 - 1));
    uint32_t y0Top  = (isY0AtFirstCol ? 0 : ((y0 - 1) * srcPixBufWidth));

    // Summed Area => D - C - B + A
    T pixelSum = 0;
    pixelSum += *(sumAreaPtr + (y1 * srcPixBufWidth + x1));                                 // Region D => (x1,     y1)
    pixelSum -= isY0AtFirstCol ? 0 : *(sumAreaPtr + (y1 * srcPixBufWidth) + x0Left);        // Region C => (x0 - 1, y1)
    pixelSum -= isX0AtFirstRow ? 0 : *(sumAreaPtr + y0Top + x1);                            // Region B => (x1,     y0 - 1)
    pixelSum += (isX0AtFirstRow && isY0AtFirstCol) ? 0 : *(sumAreaPtr + y0Top + x0Left);    // Region A => (x0 - 1, y0 - 1)

    return pixelSum;
}
