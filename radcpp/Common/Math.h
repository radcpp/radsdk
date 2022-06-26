#ifndef RADCPP_MATH_H
#define RADCPP_MATH_H
#pragma once

#include "radcpp/Common/Common.h"
#include <cmath>

#define GLM_FORCE_INTRINSICS
#include "glm/glm.hpp"
#include "glm/ext.hpp"

#define MATH_E        2.71828182845904523536   // e
#define MATH_LOG2E    1.44269504088896340736   // log2(e)
#define MATH_LOG10E   0.434294481903251827651  // log10(e)
#define MATH_LN2      0.693147180559945309417  // ln(2)
#define MATH_LN10     2.30258509299404568402   // ln(10)
#define MATH_PI       3.14159265358979323846   // pi
#define MATH_PI_2     1.57079632679489661923   // pi/2
#define MATH_PI_4     0.785398163397448309616  // pi/4
#define MATH_1_PI     0.318309886183790671538  // 1/pi
#define MATH_2_PI     0.636619772367581343076  // 2/pi
#define MATH_2_SQRTPI 1.12837916709551257390   // 2/sqrt(pi)
#define MATH_SQRT2    1.41421356237309504880   // sqrt(2)
#define MATH_SQRT1_2  0.707106781186547524401  // 1/sqrt(2)

bool SolveQuadraticEquation(float a, float b, float c, float& t0, float& t1);

#endif // RADCPP_MATH_H