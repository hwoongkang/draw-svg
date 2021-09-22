#include "renderer_utils.h"
#include "vector2D.h"
#include "vector3D.h"
#include "matrix3x3.h"
#include "color.h"

#include <iostream>
#include <cmath>
#include <vector>

using namespace CS248;

namespace utils
{
    int upperBound(float a, float b, float c)
    {
        float max = (a > b) ? ((a > c) ? a : c) : ((b > c) ? b : c);
        return (int)std::ceil(max);
    }
    int lowerBound(float a, float b, float c)
    {
        float min = (a < b) ? ((a < c) ? a : c) : ((b < c) ? b : c);
        return (int)std::floor(min);
    }

    bool isOnLeft(const Vector2D &start, const Vector2D &end, float x, float y)
    {
        auto edge = end - start;
        auto point = Vector2D(x, y) - start;
        return cross(point, edge) <= 0;
    }

    uint8_t average(const std::vector<unsigned char> &vec, int begin, int end)
    {
        float sum = 0;
        int LEN = end - begin;
        float AVG = 1.0 / LEN;
        for (auto it = vec.begin() + begin; it != vec.begin() + end; it++)
        {
            sum += AVG * (*it);
        }
        return (uint8_t)sum;
    }

    bool Triangle::isCounterClockwise()
    {
        auto edgeAB = vertexB - vertexA;
        auto edgeAC = vertexC - vertexA;
        return cross(edgeAB, edgeAC) >= 0;
    }

    void Triangle::makeCounterClockwise()
    {
        if (isCounterClockwise())
            return;

        std::swap(vertexB, vertexC);
    }
    int Triangle::getMaxX() const
    {

        return upperBound(vertexA.x, vertexB.x, vertexC.x);
    }
    int Triangle::getMinX() const
    {

        return lowerBound(vertexA.x, vertexB.x, vertexC.x);
    }
    int Triangle::getMaxY() const
    {

        return upperBound(vertexA.y, vertexB.y, vertexC.y);
    }

    int Triangle::getMinY() const
    {

        return lowerBound(vertexA.y, vertexB.y, vertexC.y);
    }

    bool Triangle::isInside(float x, float y)
    {
        auto ab = isOnLeft(vertexA, vertexB, x, y);
        auto bc = isOnLeft(vertexB, vertexC, x, y);
        auto ca = isOnLeft(vertexC, vertexA, x, y);

        return ab && bc && ca;
    }
}

namespace Task4
{
    Color interpolation(Color c0, Color c1, float x)
    {
        Color delta = c1 + (c0 * -1);
        return c0 + delta * x;
    }
    Color bilinear(Color *colors, float x, float y)
    {
        Color c00 = colors[0];
        Color c10 = colors[1];
        Color c01 = colors[2];
        Color c11 = colors[3];
        Color bottom = interpolation(c00, c10, x);
        Color top = interpolation(c01, c11, x);
        return interpolation(bottom, top, y);
    }

    Color colorFromBuffer(const std::vector<unsigned char> &buffer, int index)
    {
        Color color;
        float inv255 = 1.0 / 255.0;
        color.r = buffer[index] * inv255;
        color.g = buffer[index + 1] * inv255;
        color.b = buffer[index + 2] * inv255;
        color.a = buffer[index + 3] * inv255;
        return color;
    }
    void bound(int &num, int min, int max)
    {
        if (num < min)
        {
            num = min;
        }
        else if (num > max)
        {
            num = max;
        }
    }

}