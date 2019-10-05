#pragma once

#include <cmath>    // isgreaterequal
#include <iostream>

#define RUN    "[RUN TESTCASE] "
#define FAILED "[      FAILED] "
#define OK     "[          OK] "

#define TEST_CASE(X) { std::cout << RUN << #X << std::endl; X(); }
#define DECIMAL_TOLERANCE 1000000

#define EPSILON 2^-52

bool DoubleCompare(double x, double y)
{
    if (int(x * DECIMAL_TOLERANCE) == int(y * DECIMAL_TOLERANCE)) return true;
}

#define EXPECT_EQ(a, b, MSG) \
{ \
    const auto va = (a); \
    const auto vb = (b); \
    if (va != vb) \
    { \
        std::cout << FAILED << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << ": " << #a << " != " << #b << ", " << va << " != " << vb << std::endl; \
    } \
    else \
    {\
        std::cout << OK  << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << std::endl; \
    }\
}

#define EXPECT_NE(a, b, MSG) \
{ \
    const auto va = (a); \
    const auto vb = (b); \
    if (va == vb) \
    { \
        std::cout << FAILED << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << ": " << #a << " != " << #b << ", " << va << " != " << vb << std::endl; \
    } \
    else \
    {\
        std::cout << OK  << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << std::endl; \
    }\
}

#define EXPECT_FLOAT_EQ(a, b, MSG) \
{ \
    const auto va = (a); \
    const auto vb = (b); \
    if (!DoubleCompare(a, b)) \
    { \
        std::cout << FAILED << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << ": " << #a << " != " << #b << ", " << va << " != " << vb << std::endl; \
    } \
    else \
    {\
        std::cout << OK  << __func__ << ", " << #MSG << ", [Line]" << __LINE__ << std::endl; \
    }\
}
