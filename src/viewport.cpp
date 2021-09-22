#include "viewport.h"

#include "CS248.h"

namespace CS248
{

  void ViewportImp::set_viewbox(float x, float y, float span)
  {

    // Task 3 (part 2):
    // Set svg to normalized device coordinate transformation. Your input
    // arguments are defined as SVG canvans coordinates.

    this->x = x;
    this->y = y;
    this->span = span;

    double toOriginVec[] = {1, 0, -x, 0, 1, -y, 0, 0, 1};
    Matrix3x3 toOrigin = Matrix3x3(toOriginVec);

    double scaleVec[] = {0.5 / span, 0, 0, 0, 0.5 / span, 0, 0, 0, 1};
    Matrix3x3 scale = Matrix3x3(scaleVec);

    double toFinalVec[] = {1, 0, 0.5, 0, 1, 0.5, 0, 0, 1};
    Matrix3x3 toFinal = Matrix3x3(toFinalVec);

    svg_2_norm = toFinal * scale * toOrigin;
  }

  void ViewportImp::update_viewbox(float dx, float dy, float scale)
  {

    this->x -= dx;
    this->y -= dy;
    this->span *= scale;
    set_viewbox(x, y, span);
  }

} // namespace CS248
