#include "PixelSum.h"

#include <immintrin.h>
#include <iostream>

#include "MemoryAllocator.h"
#include "ScopedTimer.h"
#include "UtilityFunctions.h"

PixelSum::PixelSum(const unsigned char* p_Buffer, int p_XWidth, int p_YHeight)
    : m_SourcePixBufTLBR(0 /*Top Coord*/, 0/*Left Coord*/, p_YHeight - 1/*Bottom Coord*/, p_XWidth - 1/*Right Coord*/)
{
    // Pixel Sum Allocations are made from preallocated virtual memory
    // This helps in quick allocation and deallocation of pixel sum preventing performance hiches
    // that can cause by constant allocation and deallocation Pixel Sum class objects.
    const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();
    AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaTable, srcBufferPixelCount);
    AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZeroTable, srcBufferPixelCount);

    // The summed matrix pixel buffer
    ComputePixelSum<uint32_t>(PixelSumOperationType::SummedAreaTable, p_Buffer, m_SumAreaTable);

    // Non-Zero Element summed matrix
    ComputePixelSum<uint32_t>(PixelSumOperationType::NonZeroElementCount, p_Buffer, m_SumAreaNonZeroTable);
}

PixelSum::~PixelSum()
{
    // Free the memory, this memory will return back to Virtual Memory free stack,
    // where it can be efficiently reused again and again without

    VM::MemoryAllocator::GetInstance().Free(m_SumAreaTable);
    VM::MemoryAllocator::GetInstance().Free(m_SumAreaNonZeroTable);
}

PixelSum::PixelSum(const PixelSum& p_PixelSum)
{
    m_SourcePixBufTLBR = p_PixelSum.m_SourcePixBufTLBR;

    const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();

    // Deep copy the sum areas pixel buffer
    AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaTable, srcBufferPixelCount);
    memcpy(m_SumAreaTable, p_PixelSum.m_SumAreaTable, srcBufferPixelCount * sizeof(uint32_t));

    // Deep copy the non-zero elements sum areas
    AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZeroTable, srcBufferPixelCount);
    memcpy(m_SumAreaNonZeroTable, p_PixelSum.m_SumAreaNonZeroTable, srcBufferPixelCount * sizeof(uint32_t));
}

PixelSum& PixelSum::operator=(const PixelSum& p_PixelSum)
{
    if (this != &p_PixelSum)
    {
        // 1. Free the existing summed area matrixes if object is being reassigned
        if (m_SumAreaTable)
        {
            VM::MemoryAllocator::GetInstance().Free(m_SumAreaTable);
        }

        if (m_SumAreaNonZeroTable)
        {
            VM::MemoryAllocator::GetInstance().Free(m_SumAreaNonZeroTable);
        }

        // 2. Overwrite the pixel buffer top-left and bottom-right
        m_SourcePixBufTLBR = p_PixelSum.m_SourcePixBufTLBR;

        // Perform Deep copy for both summed area matrix
        const size_t srcBufferPixelCount = m_SourcePixBufTLBR.width() * m_SourcePixBufTLBR.height();

        AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaTable, srcBufferPixelCount);
        memcpy(m_SumAreaTable, p_PixelSum.m_SumAreaTable, srcBufferPixelCount * sizeof(uint32_t));

        AllocateVirtualMemoryForSumAreaMatrix<uint32_t>(m_SumAreaNonZeroTable, srcBufferPixelCount);
        memcpy(m_SumAreaNonZeroTable, p_PixelSum.m_SumAreaNonZeroTable, srcBufferPixelCount * sizeof(uint32_t));
    }

    return *this;
}

unsigned int PixelSum::GetPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    if (!s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR)) return 0;

    return ComputeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaTable);
}

double PixelSum::GetPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0; // Prevent return Nan

    return ComputeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaTable) / static_cast<double>(searchWindowPixelCount);
}

int PixelSum::GetNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    return ComputeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaNonZeroTable);
}

double PixelSum::GetNonZeroAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const
{
    uint32_t searchWindowPixelCount = s_ValidateSearchWindowClipCoords(p_X0, p_Y0, p_X1, p_Y1, m_SourcePixBufTLBR);

    if (searchWindowPixelCount == 0) return 0.0;

    return ComputeSumAreaForSearchWindow<uint32_t>(p_X0, p_Y0, p_X1, p_Y1, m_SumAreaNonZeroTable) / static_cast<double>(searchWindowPixelCount);
}

template<typename T>
void PixelSum::ComputePixelSum(PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf)
{
    // Horizontal prefix sum pass
    PixelSumHorizontalPass<T>(p_OperationType, p_PixelBuffer, p_SumAreaPixBuf);

    // SIMD optimized vertical pass
    PixelSumVerticalPass<T>(p_PixelBuffer, p_SumAreaPixBuf);
}

template<typename T>
void PixelSum::PixelSumHorizontalPass(PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixelBuffer/*, T* p_SumAreaNonZero*/)
{
    if (!p_PixelBuffer || !p_SumAreaPixelBuffer) return;

    const int srcPixBufWidth  = m_SourcePixBufTLBR.width();
    const int srcPixBufHeight = m_SourcePixBufTLBR.height();

    if (srcPixBufWidth * srcPixBufHeight == 0) return;

    T* sumAreaPtr = p_SumAreaPixelBuffer;
    const uint8_t* pixelBufferPtr = p_PixelBuffer;
    for (int row = 0; row < srcPixBufHeight; row++)
    {
        for (int col = 0; col < srcPixBufWidth; col++)
        {
            *sumAreaPtr = (p_OperationType == PixelSumOperationType::NonZeroElementCount) ? ((*pixelBufferPtr == 0) ? 0 : 1) : *pixelBufferPtr;
            *sumAreaPtr += (col == 0)/*isFirstCol*/ ? 0 : *(sumAreaPtr - 1);

            pixelBufferPtr++;
            sumAreaPtr++;
        }
    }
}

template<typename T>
void PixelSum::PixelSumVerticalPass(const unsigned char* p_PixelBuffer, T* p_SumAreaPixelBuffer)
{
    if (!p_PixelBuffer || !p_SumAreaPixelBuffer) return;

    const int srcPixBufWidth  = m_SourcePixBufTLBR.width();
    const int srcPixBufHeight = m_SourcePixBufTLBR.height();

    if (srcPixBufWidth * srcPixBufHeight == 0) return;

    T* sumAreaPtr = p_SumAreaPixelBuffer;
    T* prevRow = nullptr;
    T* currentRow = nullptr;
    for (int row = 0; row < srcPixBufHeight; row++)
    {
        currentRow = sumAreaPtr;
        if (row == 0) // Skip the first row, since we there is not previous row to add into
        {
            sumAreaPtr += srcPixBufWidth;
            prevRow = currentRow;
            continue;
        }

        SimdAddSSE(srcPixBufWidth, currentRow, prevRow);

        sumAreaPtr += srcPixBufWidth;
        prevRow = currentRow;
    }
}

void PixelSum::SimdAddSSE(int p_ArraySize, unsigned int* p_DestArray, unsigned int* p_SrcArray)
{
    int i = 0;
#if 0 // For debugging SIMD arrays
    std::cout<< "Before Curr =>";
    for (int j = 0; j < p_ArraySize; ++j)
    {
        std::cout<< p_SrcArray[j] << ", ";
    }
    std::cout << std::endl;

    std::cout<< "Before Prev =>";
    for (int j = 0; j < p_ArraySize; ++j)
    {
        std::cout<< p_DestArray[j] << ", ";
    }
    std::cout << std::endl;
    std::cout << "----------------------------------------"<<std::endl;
#endif

    const int leftOverSize = p_ArraySize % 4;
    const int alignedSize = p_ArraySize - leftOverSize;
    for (; i < alignedSize; i = i + 4)
    {
        // Load 128-bit chunks of each array
        __m128i destValues = _mm_loadu_si128((__m128i*) (p_DestArray + i));
        __m128i srcValues = _mm_loadu_si128((__m128i*) (p_SrcArray + i));

        // Add each pair of 32-bit integers in the 128-bit chunks
        destValues = _mm_add_epi32(destValues, srcValues);

        // Store 128-bit chunk to destination array
        _mm_storeu_si128((__m128i*) (p_DestArray + i), destValues);
    }

    if (leftOverSize == 0) return;

    // Handle left-over
    for (; i < p_ArraySize; i++)
    {
        *(p_DestArray + i) += *(p_SrcArray + i);
    }
}
/********************************************************************************
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
template<typename T>
unsigned int PixelSum::ComputeSumAreaForSearchWindow(int x0, int y0, int x1, int y1, T* p_SumArea) const
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

template<typename T>
bool PixelSum::AllocateVirtualMemoryForSumAreaMatrix(T*& p_SumAreaMatrix, size_t p_AllocSize)
{
    p_SumAreaMatrix = static_cast<T*>(VM::MemoryAllocator::GetInstance().Allocate(p_AllocSize * sizeof(T)));

    return p_SumAreaMatrix != nullptr;
}
