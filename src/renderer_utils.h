#ifndef CS248_RENDERER_UTILS
#define CS248_RENDERER_UTILS

#include "vector2D.h"
#include "matrix3x3.h"
#include "color.h"
#include <vector>

using namespace CS248;
namespace Task4
{
    Color bilinear(Color *colors, float x, float y);
    Color colorFromBuffer(const std::vector<unsigned char> &buffer, int index);
    void bound(int &num, int min, int max);
}

namespace utils
{
    int upperBound(float a, float b, float c);
    int lowerBound(float a, float b, float c);

    bool isOnLeft(const Vector2D &start, const Vector2D &end, int x, int y);

    uint8_t average(const std::vector<unsigned char> &vec, int begin, int end);

    class Triangle
    {
    public:
        Triangle(float x0, float y0, float x1, float y1, float x2, float y2) : vertexA(x0, y0), vertexB(x1, y1), vertexC(x2, y2) {}

        void makeCounterClockwise();

        int getMinX() const;
        int getMaxX() const;
        int getMinY() const;
        int getMaxY() const;

        bool isInside(float x, float y);

    private:
        bool isCounterClockwise();
        Vector2D vertexA;
        Vector2D vertexB;
        Vector2D vertexC;
    };
}

#endif