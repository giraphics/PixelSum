#pragma once

#include "TestCaseHelper.h"

#include "PixelBuffer.h"
#include "UtilityFunctions.h"
#include "PixelSumNaive.h"
#include "PixelSum.h"

#define MAX_SUPPORTED_IMAGE_DIMENSION 4096
#define CORRECT_IMAGE_DIMENSION(IMG_DIM) (g_Clamp(IMG_DIM, 0 , MAX_SUPPORTED_IMAGE_DIMENSION))

const static int IMAGE_WIDTH  = CORRECT_IMAGE_DIMENSION(4096); // clamp the image if invalid in range.
const static int IMAGE_HEIGHT = CORRECT_IMAGE_DIMENSION(4096); // clamp the image if invalid in range.
const static int IMAGE_BOTTOM = (IMAGE_HEIGHT - 1);
const static int IMAGE_RIGHT  = (IMAGE_WIDTH  - 1);

// Preallocate the memory for our pixel buffer and summed area matrix.
void PreallocateMemoryVirtualMemory(uint32_t p_MaxImageCount, uint32_t p_MaxSummedAreaPixelBuffer, uint32_t p_MaxSummedAreaNonZero)
{
    std::vector<VM::UserMemoryRequirementConfig> userMemoryRequirement =
    {
        { IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint8_t) , p_MaxImageCount            },
        { IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint32_t), p_MaxSummedAreaPixelBuffer },
        { IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(uint32_t), p_MaxSummedAreaNonZero     },
    };
    VM::MemoryAllocator::GetInstance().ConfigureMemory(userMemoryRequirement);
}

// Validate the image dimensions
void ConfigureMemoryTest()
{
    EXPECT_EQ(IMAGE_WIDTH  > 0, true, "Image Left Validation");
    EXPECT_EQ(IMAGE_HEIGHT > 0, true, "Image Top Validation");

    EXPECT_EQ(IMAGE_WIDTH  <= MAX_SUPPORTED_IMAGE_DIMENSION, true, "Image Right Validation");
    EXPECT_EQ(IMAGE_HEIGHT <= MAX_SUPPORTED_IMAGE_DIMENSION, true, "Image Bottom Validation");
}

// Memory leak test for class Image and class Pixel sum 100 objects are continous allocated and deallocated.
// Not that our preallocate memory is limited. Therefore, if there is leak the applciation must crash.
void MemoryLeakTest()
{
    for (int i = 0; i < 30; i++)
    {
        // Create a full size pixel buffer and fill half data with 1
        Image* imageFullSize = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
        PixelSum* pixelSumFullSize = new PixelSum(imageFullSize->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

        const size_t halfPixelBufferWidth = IMAGE_WIDTH >> 1;
        const size_t halfPixelBufferHeight = IMAGE_HEIGHT >> 1;

        // Create a half size pixel buffer and fill half data with 1
        Image* imageHalfSize = new Image(halfPixelBufferWidth, halfPixelBufferHeight);
        PixelSum* pixelSumHalfSize = new PixelSum(imageHalfSize->GetPixelBufferPtr(), halfPixelBufferWidth, halfPixelBufferHeight);

        // Release pixel buffer
        delete imageFullSize;
        delete imageHalfSize;

        // Release pixel sum
        delete pixelSumFullSize;
        delete pixelSumHalfSize;
    }

    // If this test case is printed, it mean the application is still alive and there are not memory leaks
    EXPECT_EQ(true, true, "30 image pixel sum class object with full and half size pixel buffers");
}

// This test case matches sum area result from Naive implementation and optimize implementation O(1)
void GetPixelSumVsNaiveSumAreaResult()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillDataWithContinousNumberStartingWith(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);
    PixelSumNaive* pixelSumNaiveImp = new PixelSumNaive(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Valid inputput range to test if the basic pixel sum logic works correct.
    for (int i = 0; i < 50; i++)
    {
        std::srand(i * 100);
        int x0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int y0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int searchWidth = (std::rand() % 10) + 1;
        int searchHeight = (std::rand() % 10) + 1;
        int x1 = x0 + searchWidth;
        int y1 = y0 + searchHeight;

        std::cout << "Search Window: x0: " << x0 << ", y0: " << y0 << ", x1: " << x1 << ", y1: " << y1 << std::endl;
        EXPECT_FLOAT_EQ(pixelSum->GetPixelSum(x0, y0 , x1, y1),
                        pixelSumNaiveImp->GetPixelSum(x0, y0 , x1, y1),
                        "Testing GetPixelSum() result with naive implementation.");
    }

    delete image;
    delete pixelSum;
    delete pixelSumNaiveImp;
}

void GetPixelSumInvalidRangeTest()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Search window in bound test
    EXPECT_EQ(pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
                                    (IMAGE_HEIGHT * IMAGE_WIDTH) / 2,
                                    "Search Window Size == Image Size");

    // Search window out bound test
    const int searchWindowDimension = IMAGE_WIDTH * 10;
    EXPECT_EQ(pixelSum->GetPixelSum(0, 0, searchWindowDimension - 1, searchWindowDimension - 1),
                                   (IMAGE_HEIGHT * IMAGE_WIDTH) / 2,
                                   "Search Window out of bound test. Search Window Size > Image Size");

    delete image;
    delete pixelSum;
}

// This test case matches sum area average result with expected const precalculated result.
void GetPixelAverage()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillFullPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // We request the average of pixels (-2, -2)-(0, 0) the resulting value should be 1/9 (=0.111…).
    EXPECT_FLOAT_EQ(pixelSum->GetPixelAverage(-2, -2, 0, 0), 1.0 / 9.0, "Search Window with top-left outside valid range");

    // Request the average of pixels (-10, -10)-(imgBottom+10,imgRight+10) the resulting value should be 4 / 16 (0.25).
    EXPECT_FLOAT_EQ(pixelSum->GetPixelAverage(0, 0, 0, 0), 1.0, "One pixel avg test");

    int x0Offset = -9;
    int y0Offset = -9;
    int x1Offset = 9;
    int y1Offset = 9;
    EXPECT_FLOAT_EQ(pixelSum->GetPixelAverage((IMAGE_RIGHT + x0Offset), (IMAGE_BOTTOM + y0Offset) , IMAGE_RIGHT + x1Offset, IMAGE_BOTTOM + y1Offset),
                    ((abs(x0Offset) + 1) * (abs(y0Offset) + 1)) / static_cast<float>((abs(x1Offset - x0Offset) + 1) * (abs(y1Offset - y0Offset) + 1)),
                    "Search Window bottom right is outside the image region");

    delete image;
    delete pixelSum;
}

// This test case generate random samples and matches sum area average result from Naive implementation and optimize implementation.
void GetPixelAverageVsNaiveSumAreaResult()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);
    PixelSumNaive* pixelSumNaiveImp = new PixelSumNaive(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Valid inputput range to test if the basic pixel sum logic works correct.
    for (int i = 0; i < 100; i++)
    {
        std::srand(i * 100);
        int x0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int y0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int searchWidth = (std::rand() % 10) + 1;
        int searchHeight = (std::rand() % 10) + 1;
        int x1 = x0 + searchWidth;
        int y1 = y0 + searchHeight;

        std::cout << "Search Window: x0: " << x0 << ", y0: " << y0 << ", x1: " << x1 << ", y1: " << y1 << std::endl;
        EXPECT_FLOAT_EQ(pixelSum->GetPixelAverage(x0, y0 , x1, y1),
                        pixelSumNaiveImp->GetPixelAverage(x0, y0 , x1, y1),
                        "sss");
    }

    delete image;
    delete pixelSum;
    delete pixelSumNaiveImp;
}

// [NOTE] We are creating objects more than the actual allocation made for pixel buffer objects (MAX_IMAGE_COUNT,
// MAX_SUMMED_AREA_NON_ZERO, MAX_SUMMED_AREA_PIXEL_BUFFER) if there memory leak the VM would assert
void MemoryLeakTestCopyCtor()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Memory leak test on CopyCtor
    for (int i = 0; i < 100; ++i)
    {
        PixelSum* pixelSumCopyCtrObj = new PixelSum(*pixelSum);
        EXPECT_EQ(pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
                  pixelSumCopyCtrObj->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT), "Create and Desctroyed 100 pixel sum objects and computed their pixel sum.");

        delete pixelSumCopyCtrObj;
    }

    // We are creating objects more than the actual allocation made for pixel buffer objects (MAX_IMAGE_COUNT,
    // MAX_SUMMED_AREA_NON_ZERO, MAX_SUMMED_AREA_PIXEL_BUFFER) if there memory leak the VM would assert

    EXPECT_EQ(true, true, "Memory leak test on copy ctor success");
}

// This test is similar to memoryLeakTestCopyCtor except the fact we allocate and destroy 1000 pixel sum object
// via assignment operator.

// [NOTE] We are creating objects more than the actual allocation made for pixel buffer objects (MAX_IMAGE_COUNT,
// MAX_SUMMED_AREA_NON_ZERO, MAX_SUMMED_AREA_PIXEL_BUFFER) if there memory leak the VM would assert
void MemoryLeakTestPixelSumAssignOperator()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Memory leak test with Pixel Sum Assignment operator
    for (int i = 0; i < 100; ++i)
    {
        // Create empty Pixel Sum object
        PixelSum* pixelSumAssignOperatorTest = new PixelSum();

        // Fill empty pixel sum object with assignment operator
        *pixelSumAssignOperatorTest = *pixelSum; // Assignment operator called

        EXPECT_EQ(pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
                  pixelSumAssignOperatorTest->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
                  "Memory leak test, Creating and Desctroying 100 pixel sum objects and computed their pixel sum.");

        delete pixelSumAssignOperatorTest;
    }

    EXPECT_EQ(true, true, "Memory leak test on copy ctor success");
}

void CopyConstructor()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    PixelSum* pixelSumCopyCtrObj = new PixelSum(*pixelSum);

    EXPECT_EQ(pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
              pixelSumCopyCtrObj->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT), "Copy Constructor pixel Sum Test");
}

void AssignmentOperator()
{
    // Create a pixel buffer and fill half of data with 1
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
    s_FillHalfPixelBufferWithConstValue((IMAGE_WIDTH * IMAGE_HEIGHT), image->GetPixelBufferPtr(), 1);

    // Create Pixel Sum object filled with data
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Create empty Pixel Sum object
    PixelSum* pixelSumAssignOperatorTest = new PixelSum();

    // Fill empty pixel sum object with assignment operator
    *pixelSumAssignOperatorTest = *pixelSum; // Assignment operator called

    for (int i = 0; i < 10; i++)
    {
        std::srand(i * 100);
        int x0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int y0 = CORRECT_IMAGE_DIMENSION(std::rand() % MAX_SUPPORTED_IMAGE_DIMENSION);
        int searchWidth = (std::rand() % 10) + 1;
        int searchHeight = (std::rand() % 10) + 1;
        int x1 = x0 + searchWidth;
        int y1 = y0 + searchHeight;

        // Random sample test
        EXPECT_EQ(pixelSumAssignOperatorTest->GetPixelSum(x0, y0, x1, y1),
                  pixelSum->GetPixelSum(x0, y0, x1, y1), "Assignment operator random pixel sum sampling test");
    }

    // Full image extent pixel sum test
    EXPECT_EQ(pixelSumAssignOperatorTest->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
              pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT), "Assignment operator - Full image extent pixel sum test");

    // Half image extent pixel sum test
    EXPECT_EQ(pixelSumAssignOperatorTest->GetPixelSum(0, 0, IMAGE_BOTTOM >> 1, IMAGE_RIGHT >> 1),
              pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM >> 1, IMAGE_RIGHT >> 1), "Assignment operator - Half image extent pixel sum test");

    // Over flow image extent pixel sum test
    EXPECT_EQ(pixelSumAssignOperatorTest->GetPixelSum(0, 0, IMAGE_BOTTOM, IMAGE_RIGHT),
              pixelSum->GetPixelSum(0, 0, IMAGE_BOTTOM + 10, IMAGE_RIGHT + 10), "Assignment operator - Over flow image extent pixel sum test");

    // Release pixel buffer
    delete image;

    // Release pixel sum
    delete pixelSum;
}

// Test case to check Non-Zero elements
void NonZeroCountElementsCounts()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);
    PixelSumNaive* pixelSumNaiveImp = new PixelSumNaive(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Valid inputput range to test if the basic pixel sum logic works correct.
    for (int i = 0; i < 10; i++)
    {
        std::srand(i * 100);
        int x0 = std::rand() % (IMAGE_WIDTH >> 1);
        int y0 = std::rand() % (IMAGE_HEIGHT >> 1);
        int x1 = x0 + (std::rand() % (IMAGE_WIDTH >> 2));
        int y1 = y0 + (std::rand() % (IMAGE_HEIGHT >> 2));

        EXPECT_EQ(pixelSum->GetNonZeroCount(x0, y0, x1, y1),
                  pixelSumNaiveImp->GetNonZeroCount(x0, y0, x1, y1),
                  "Random sample search window sample to compute non-zero element counts");
    }

    delete image;
    delete pixelSum;
    delete pixelSumNaiveImp;
}

// Test case to check Non-Zero elements
void NanValueTest()
{
    Image* image = new Image(IMAGE_WIDTH, IMAGE_HEIGHT);

    // Fill full pixel buffer with value 1
    const size_t dataSize = IMAGE_WIDTH * IMAGE_HEIGHT;
    s_FillHalfPixelBufferWithConstValue(dataSize, image->GetPixelBufferPtr(), 1);
    PixelSum* pixelSum = new PixelSum(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);
    PixelSumNaive* pixelSumNaiveImp = new PixelSumNaive(image->GetPixelBufferPtr(), IMAGE_WIDTH, IMAGE_HEIGHT);

    // Valid inputput range to test if the basic pixel sum logic works correct.
    for (int i = 0; i < 10; i++)
    {
        std::srand(i * 100);
        int x0 = std::rand() % (IMAGE_WIDTH >> 1);
        int y0 = std::rand() % (IMAGE_HEIGHT >> 1);
        int x1 = x0 + (std::rand() % (IMAGE_WIDTH >> 2));
        int y1 = y0 + (std::rand() % (IMAGE_HEIGHT >> 2));

        EXPECT_EQ(pixelSum->GetNonZeroCount(-1, -1, -1, -1),
                  pixelSumNaiveImp->GetNonZeroCount(-1, -1, -1, -1),
                  "Random sample search window sample to compute non-zero element counts");
    }

    delete image;
    delete pixelSum;
    delete pixelSumNaiveImp;
}


void TestCaseEntry()
{
    constexpr int MAX_IMAGE_COUNT              = 10;
    constexpr int MAX_SUMMED_AREA_PIXEL_BUFFER = MAX_IMAGE_COUNT * 2;
    constexpr int MAX_SUMMED_AREA_NON_ZERO     = MAX_SUMMED_AREA_PIXEL_BUFFER;

    PreallocateMemoryVirtualMemory(MAX_IMAGE_COUNT, MAX_SUMMED_AREA_PIXEL_BUFFER, MAX_SUMMED_AREA_NON_ZERO);

    TEST_CASE(ConfigureMemoryTest);
    TEST_CASE(MemoryLeakTest);
    TEST_CASE(MemoryLeakTestCopyCtor);
    TEST_CASE(MemoryLeakTestPixelSumAssignOperator);
    TEST_CASE(GetPixelSumInvalidRangeTest);
    TEST_CASE(GetPixelSumVsNaiveSumAreaResult);
    TEST_CASE(GetPixelAverage);
    TEST_CASE(GetPixelAverageVsNaiveSumAreaResult);
    TEST_CASE(CopyConstructor);
    TEST_CASE(AssignmentOperator);
    TEST_CASE(NonZeroCountElementsCounts);
    TEST_CASE(NanValueTest);
}
