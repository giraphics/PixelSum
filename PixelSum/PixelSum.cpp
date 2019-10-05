#include "PixelSum.h"

#include <iostream>

#include "MemoryAllocator.h"
#include "UtilityFunctions.h"
#include "ScopedTimer.h"

PixelSum::PixelSum(const unsigned char* p_Buffer, int p_XWidth, int p_YHeight)
    : m_SourcePixBufTLBR(0 /*Top Coord*/, 0/*Left Coord*/, p_YHeight - 1/*Bottom Coord*/, p_XWidth - 1/*Right Coord*/)
{
    // Pixel Sum Allocations are made from preallocated virtual memory
    // This helps in quick allocation and deallocation of pixel sum preventing performance hiches
    // that can cause by constant allocation and deallocation Pixel Sum class objects.
    const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaPixBuf, srcBufferPixelCount);
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZero, srcBufferPixelCount);

    // The summed matrix pixel buffer is computed in two passes (1) Horizontal and (2) Vertical
    {
        DefaultResults results;
        ScopedTimer Timer(results);
        computePixelSum<uint32_t>(PixelSumPassType::Horizontal, PixelSumOperationType::SummedAreaTable, p_Buffer, m_SumAreaPixBuf);
        computePixelSum<uint32_t>(PixelSumPassType::Vertical,   PixelSumOperationType::SummedAreaTable, p_Buffer, m_SumAreaPixBuf);
    }

    // Non-Zero Element summed matrix is computed in two passes (1) Horizontal and (2) Vertical
    {
        DefaultResults results;
        ScopedTimer Timer(results);
        computePixelSum<uint32_t>(PixelSumPassType::Horizontal, PixelSumOperationType::NonZeroElementCount, p_Buffer, m_SumAreaNonZero);
        computePixelSum<uint32_t>(PixelSumPassType::Vertical,   PixelSumOperationType::NonZeroElementCount, p_Buffer, m_SumAreaNonZero);
    }

    // Note: The two m_SumAreaPixBuf and m_SumAreaNonZero can be computed in single
    // function call which might a marginal faster but not a good idea for scalablily
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

    // Deep copy the sum areas pixel buffer
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaPixBuf, srcBufferPixelCount);
    memcpy(m_SumAreaPixBuf, p_PixelSum.m_SumAreaPixBuf, srcBufferPixelCount * sizeof(uint32_t));

    // Deep copy the non-zero elements sum areas
    allocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZero, srcBufferPixelCount);
    memcpy(m_SumAreaNonZero, p_PixelSum.m_SumAreaNonZero, srcBufferPixelCount * sizeof(uint32_t));
}

PixelSum& PixelSum::operator=(const PixelSum& p_PixelSum)
{
    if (this != &p_PixelSum)
    {
        // 1. Free the existing summed area matrixes if object is being reassigned
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
unsigned int PixelSum::getPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    if (!s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR)) return 0;

    return computeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaPixBuf);
}

double PixelSum::getPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0;

    return computeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaPixBuf) / static_cast<double>(searchWindowPixelCount);
}

int PixelSum::getNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    return computeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaNonZero);
}

double PixelSum::getNonZeroAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0;

    return computeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaNonZero) / static_cast<double>(searchWindowPixelCount);
}

template<typename T>
bool PixelSum::computePixelSum(PixelSumPassType p_PassType, PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixelBuffer/*, T* p_SumAreaNonZero*/)
{
    if (!p_PixelBuffer || !p_SumAreaPixelBuffer) return false;

    const int srcPixBufWidth  = m_SourcePixBufTLBR.width();
    const int srcPixBufHeight = m_SourcePixBufTLBR.height();

    if (srcPixBufWidth * srcPixBufHeight == 0) return false;

    T* sumAreaPtr = p_SumAreaPixelBuffer;
    const uint8_t* pixelBufferPtr = p_PixelBuffer;
    for (int row = 0; row < srcPixBufHeight; row++)
    {
        const bool isFirstRow = (row == 0);
        for (int col = 0; col < srcPixBufWidth; col++)
        {
            if (p_PassType == PixelSumPassType::Horizontal)
            {
                *sumAreaPtr = (p_OperationType == PixelSumOperationType::NonZeroElementCount) ? ((*pixelBufferPtr == 0) ? 0 : 1) : *pixelBufferPtr;
                *sumAreaPtr += (col == 0)/*isFirstCol*/ ? 0 : *(sumAreaPtr - 1);
            }
            else
            {
                *sumAreaPtr += isFirstRow ? 0 : *(sumAreaPtr - srcPixBufWidth);
            }

            pixelBufferPtr++;

            sumAreaPtr++;
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
