#pragma once

#include <stdint.h>
#include <cstddef>

#include "CustomTypes.h"

class PixelSum
{
public:
    PixelSum() = default;
    PixelSum(const unsigned char* p_Buffer, int p_XWidth, int p_YHeight);
    ~PixelSum(void);

    PixelSum(const PixelSum& p_PixelSum);
    PixelSum& operator= (const PixelSum& p_PixelSum);

    unsigned int GetPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    double GetPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const;

    int GetNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    double GetNonZeroAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const;

private:
    enum class PixelSumOperationType
    {
        SummedAreaTable     = (1u << 0u),
        NonZeroElementCount = (1u << 1u),
    };

    /*!
     * Compute the pixel sum in horizontal pass. It can compute
     * (1) sumarea for pixel buffer value or
     * (2) Non-Zero elements
     */
    template<typename T>
    void PixelSumHorizontalPass(PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf/*, T* p_SumAreaNonZero*/);

    /*!
     * Compute the pixel sum in either horizontal or vertical pass. It can compute sum area for pixel
     * buffer value or non-zero elements
     */
    template<typename T>
    void PixelSumVerticalPass(const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf/*, T* p_SumAreaNonZero*/);

    /*!
     * Single Instruction Multiple Data (SIMD) helper function to add two arrays (1) destination and (2) source array
     * into the destination array.
     */
    int SimdAddSSE(int p_ArraySize, unsigned int* p_DestArray, unsigned int* p_SrcArray);

    /*!
     * Calls pixelSumPass(..) with Horizontal pass followed with Vertical pass.
     */
    template<typename T>
    void ComputePixelSum(PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf);

    /*!
     * Compute the Sum area of the search window coordinates with below formula
     *         0        1       2       3
     * 0 +--------+---------------+      A => Area((0,0) To (1, 1))
     *   |        |               |      B => Area((0,0) To (1, 3))
     *   |        |               |      C => Area((0,0) To (3, 1))
     *   |        |               |      D => Area((0,0) To (3, 3))
     * 1 +------------------------+
     *   |        |A              |B
     *   |        |               |      Summed Area(ABCD) => D - C - B + A
     * 2 |        |               |
     *   |        |               |
     * 3 +--------+---------------+
     *            C               D
     */
    template<typename T>
    unsigned int ComputeSumAreaForSearchWindow(int x0, int y0, int x1, int y1, T* p_SumArea) const;

    /*!
     * Allocate virtual memory from preallocated memory pool for summed area matrix
     */
    template<typename T>
    bool AllocateVirtualMemoryForSumAreaMatrix(T*& p_SumAreaMatrix, size_t p_AllocSize);

private:
    PixBufTLBR_i m_SourcePixBufTLBR;

    // Max image size can be 4096x4096 with highest possible val 255, therefore unsigned 32bit storage is more than enough
    uint32_t* m_SumAreaTable = nullptr; /*!< Summed area table for pixel buffer */

    // The non-zero element can be marked with 1 and zero with 0. With 4096x4096 and 1 as max possible value, 32bit storage suffice.
    uint32_t* m_SumAreaNonZeroTable = nullptr; /*!< Summed area table for non-zero pixel buffer */
};
