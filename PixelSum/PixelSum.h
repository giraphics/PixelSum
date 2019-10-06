#pragma once

#include <vector>
#include "UtilityFunctions.h"
//template<typename T>
//struct Vector4
//{
//    Vector4() = default;
//    Vector4(float p_Top, float p_Left, float p_Bottom, float p_Right)
//        : top(p_Top), left(p_Left), bottom(p_Bottom), right(p_Right) {}

//    int width() const { return right + 1; }
//    int height() const { return bottom + 1; }

//    union
//    {
//        struct{ T top, left, bottom, right; };
//        struct{ T y0 ,   x0,     y1,    x1; };
//    };
//};
//typedef struct Vector4<int> PixelBufferCoords_i; // Pixel buffer's (x0, y0), (x1, y1) representation, _i for type int
//typedef struct Vector4<int> PixBufTLBR_i;        // Pixel buffer's top, left bottom, right representation, _i for type int

class PixelSum
{
public:
    PixelSum() = default;
    PixelSum(const unsigned char* p_Buffer, int p_XWidth, int p_YHeight);
    ~PixelSum(void);

    PixelSum(const PixelSum& p_PixelSum);
    PixelSum& operator= (const PixelSum& p_PixelSum);

    unsigned int getPixelSum(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    double getPixelAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const;

    int getNonZeroCount(int p_X0, int p_Y0, int p_X1, int p_Y1) const;
    double getNonZeroAverage(int p_X0, int p_Y0, int p_X1, int p_Y1) const;

private:
    enum class PixelSumPassType
    {
        Horizontal = (1u << 0u),
        Vertical   = (1u << 1u),
    };

    enum class PixelSumOperationType
    {
        SummedAreaTable     = (1u << 0u),
        NonZeroElementCount = (1u << 1u),
    };

    /*!
     * Compute the pixel sum in either horizontal or vertical pass. It can compute sumarea for pixel buffer value or non-zero elements
     */
    template<typename T>
    void pixelSumPass(PixelSumPassType p_IsHPass, PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf/*, T* p_SumAreaNonZero*/);

    template<typename T>
    void pixelSumPassVerticalPass(PixelSumPassType p_IsHPass, PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf/*, T* p_SumAreaNonZero*/);
    int add_SSE(int size, unsigned int* first_array, unsigned int* second_array);

    /*!
     * Calls pixelSumPass(..) with Horizontal pass followed with Vertical pass.
     */
    template<typename T>
    void computePixelSum(PixelSumOperationType p_OperationType, const unsigned char* p_PixelBuffer, T* p_SumAreaPixBuf);

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
    unsigned int computeSumAreaForSearchWindow(int x0, int y0, int x1, int y1, T* p_SumArea) const;

    /*!
     * Allocate virtual memory from preallocated memory pool for summed area matrix
     */
    template<typename T>
    bool allocateVirtualMemoryForSumAreaMatrix(T*& p_SumAreaMatrix, size_t p_AllocSize);

private:
    PixBufTLBR_i m_SourcePixBufTLBR;

    // Max image size can be 4096x4096 with highest possible val 255, therefore unsigned 32bit storage is more than enough
    uint32_t* m_SumAreaTable = nullptr; /*!< Summed area table for pixel buffer */

    // The non-zero element can be marked with 1 and zero with 0. With 4096x4096 and 1 as max possible value, 32bit storage suffice.
    uint32_t* m_SumAreaNonZeroTable = nullptr; /*!< Summed area table for non-zero pixel buffer */
};
