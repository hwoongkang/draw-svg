#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"
#include "renderer_utils.h"

using namespace std;

namespace CS248
{

  // Implements SoftwareRenderer //

  // fill a sample location with color
  void SoftwareRendererImp::fill_sample(int sx, int sy, const Color &color)
  {
  }

  // fill samples in the entire pixel specified by pixel coordinates
  void SoftwareRendererImp::fill_pixel(int x, int y, const Color &color)
  {

    // Task 2: Re-implement this function

    // check bounds
    if (x < 0 || x >= target_w)
      return;
    if (y < 0 || y >= target_h)
      return;

    Color pixel_color;
    float inv255 = 1.0 / 255.0;
    pixel_color.r = render_target[4 * (x + y * target_w)] * inv255;
    pixel_color.g = render_target[4 * (x + y * target_w) + 1] * inv255;
    pixel_color.b = render_target[4 * (x + y * target_w) + 2] * inv255;
    pixel_color.a = render_target[4 * (x + y * target_w) + 3] * inv255;

    pixel_color = ref->alpha_blending_helper(pixel_color, color);

    render_target[4 * (x + y * target_w)] = (uint8_t)(pixel_color.r * 255);
    render_target[4 * (x + y * target_w) + 1] = (uint8_t)(pixel_color.g * 255);
    render_target[4 * (x + y * target_w) + 2] = (uint8_t)(pixel_color.b * 255);
    render_target[4 * (x + y * target_w) + 3] = (uint8_t)(pixel_color.a * 255);
  }

  void SoftwareRendererImp::draw_svg(SVG &svg)
  {

    // set top level transformation
    transformation = canvas_to_screen;

    // draw all elements
    for (size_t i = 0; i < svg.elements.size(); ++i)
    {
      draw_element(svg.elements[i]);
    }

    // draw canvas outline
    Vector2D a = transform(Vector2D(0, 0));
    a.x--;
    a.y--;
    Vector2D b = transform(Vector2D(svg.width, 0));
    b.x++;
    b.y--;
    Vector2D c = transform(Vector2D(0, svg.height));
    c.x--;
    c.y++;
    Vector2D d = transform(Vector2D(svg.width, svg.height));
    d.x++;
    d.y++;

    rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
    rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
    rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
    rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

    // resolve and send to render target
    resolve();
  }

  void SoftwareRendererImp::set_sample_rate(size_t sample_rate)
  {

    // Task 2:
    // You may want to modify this for supersampling support
    this->sample_rate = sample_rate;
    supersample_target.resize(4 * target_w * target_h * sample_rate * sample_rate);
  }

  void SoftwareRendererImp::set_render_target(unsigned char *render_target,
                                              size_t width, size_t height)
  {

    // Task 2:
    // You may want to modify this for supersampling support
    this->render_target = render_target;
    this->target_w = width;
    this->target_h = height;
    supersample_target.resize(4 * width * height * sample_rate * sample_rate);
  }

  void SoftwareRendererImp::draw_element(SVGElement *element)
  {

    // Task 3 (part 1):
    // Modify this to implement the transformation stack

    switch (element->type)
    {
    case POINT:
      draw_point(static_cast<Point &>(*element));
      break;
    case LINE:
      draw_line(static_cast<Line &>(*element));
      break;
    case POLYLINE:
      draw_polyline(static_cast<Polyline &>(*element));
      break;
    case RECT:
      draw_rect(static_cast<Rect &>(*element));
      break;
    case POLYGON:
      draw_polygon(static_cast<Polygon &>(*element));
      break;
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse &>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image &>(*element));
      break;
    case GROUP:
      draw_group(static_cast<Group &>(*element));
      break;
    default:
      break;
    }
  }

  // Primitive Drawing //

  void SoftwareRendererImp::draw_point(Point &point)
  {

    Vector2D p = transform(point.position);
    rasterize_point(p.x, p.y, point.style.fillColor);
  }

  void SoftwareRendererImp::draw_line(Line &line)
  {

    Vector2D p0 = transform(line.from);
    Vector2D p1 = transform(line.to);
    rasterize_line(p0.x, p0.y, p1.x, p1.y, line.style.strokeColor);
  }

  void SoftwareRendererImp::draw_polyline(Polyline &polyline)
  {

    Color c = polyline.style.strokeColor;

    if (c.a != 0)
    {
      int nPoints = polyline.points.size();
      for (int i = 0; i < nPoints - 1; i++)
      {
        Vector2D p0 = transform(polyline.points[(i + 0) % nPoints]);
        Vector2D p1 = transform(polyline.points[(i + 1) % nPoints]);
        rasterize_line(p0.x, p0.y, p1.x, p1.y, c);
      }
    }
  }

  void SoftwareRendererImp::draw_rect(Rect &rect)
  {

    Color c;

    // draw as two triangles
    float x = rect.position.x;
    float y = rect.position.y;
    float w = rect.dimension.x;
    float h = rect.dimension.y;

    Vector2D p0 = transform(Vector2D(x, y));
    Vector2D p1 = transform(Vector2D(x + w, y));
    Vector2D p2 = transform(Vector2D(x, y + h));
    Vector2D p3 = transform(Vector2D(x + w, y + h));

    // draw fill
    c = rect.style.fillColor;
    if (c.a != 0)
    {
      rasterize_triangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c);
      rasterize_triangle(p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c);
    }

    // draw outline
    c = rect.style.strokeColor;
    if (c.a != 0)
    {
      rasterize_line(p0.x, p0.y, p1.x, p1.y, c);
      rasterize_line(p1.x, p1.y, p3.x, p3.y, c);
      rasterize_line(p3.x, p3.y, p2.x, p2.y, c);
      rasterize_line(p2.x, p2.y, p0.x, p0.y, c);
    }
  }

  void SoftwareRendererImp::draw_polygon(Polygon &polygon)
  {

    Color c;

    // draw fill
    c = polygon.style.fillColor;
    if (c.a != 0)
    {

      // triangulate
      vector<Vector2D> triangles;
      triangulate(polygon, triangles);

      // draw as triangles
      for (size_t i = 0; i < triangles.size(); i += 3)
      {
        Vector2D p0 = transform(triangles[i + 0]);
        Vector2D p1 = transform(triangles[i + 1]);
        Vector2D p2 = transform(triangles[i + 2]);
        rasterize_triangle(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c);
      }
    }

    // draw outline
    c = polygon.style.strokeColor;
    if (c.a != 0)
    {
      int nPoints = polygon.points.size();
      for (int i = 0; i < nPoints; i++)
      {
        Vector2D p0 = transform(polygon.points[(i + 0) % nPoints]);
        Vector2D p1 = transform(polygon.points[(i + 1) % nPoints]);
        rasterize_line(p0.x, p0.y, p1.x, p1.y, c);
      }
    }
  }

  void SoftwareRendererImp::draw_ellipse(Ellipse &ellipse)
  {

    // Extra credit
  }

  void SoftwareRendererImp::draw_image(Image &image)
  {

    Vector2D p0 = transform(image.position);
    Vector2D p1 = transform(image.position + image.dimension);

    rasterize_image(p0.x, p0.y, p1.x, p1.y, image.tex);
  }

  void SoftwareRendererImp::draw_group(Group &group)
  {

    for (size_t i = 0; i < group.elements.size(); ++i)
    {
      draw_element(group.elements[i]);
    }
  }

  // Rasterization //

  // The input arguments in the rasterization functions
  // below are all defined in screen space coordinates

  void SoftwareRendererImp::rasterize_point(float x, float y, Color color)
  {

    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= target_w)
      return;
    if (sy < 0 || sy >= target_h)
      return;

    // fill sample - NOT doing alpha blending!
    // TODO: Call fill_pixel here to run alpha blending
    render_target[4 * (sx + sy * target_w)] = (uint8_t)(color.r * 255);
    render_target[4 * (sx + sy * target_w) + 1] = (uint8_t)(color.g * 255);
    render_target[4 * (sx + sy * target_w) + 2] = (uint8_t)(color.b * 255);
    render_target[4 * (sx + sy * target_w) + 3] = (uint8_t)(color.a * 255);
  }

  void SoftwareRendererImp::rasterize_line(float x0, float y0,
                                           float x1, float y1,
                                           Color color)
  {

    // Extra credit (delete the line below and implement your own)
    ref->rasterize_line_helper(x0, y0, x1, y1, target_w, target_h, color, this);
  }

  void SoftwareRendererImp::rasterize_triangle(float x0, float y0,
                                               float x1, float y1,
                                               float x2, float y2,
                                               Color color)
  {
    // Task 1:
    // Implement triangle rasterization (you may want to call fill_sample here)

    auto triangle = utils::Triangle(x0, y0, x1, y1, x2, y2);

    triangle.makeCounterClockwise();

    // Triangle check optimization: Check only for bounding box
    // https://www.scratchapixel.com/lessons/3d-basic-rendering/rasterization-practical-implementation/overview-rasterization-algorithm
    int minX = triangle.getMinX();
    int maxX = triangle.getMaxX();
    int minY = triangle.getMinY();
    int maxY = triangle.getMaxY();

    int squared = sample_rate * sample_rate;
    float d = 1.0 / sample_rate;

    for (int sx = minX; sx <= maxX; sx++)
    {
      for (int sy = minY; sy <= maxY; sy++)
      {

        // screen check
        if (sx < 0 || sx >= target_w)
        {
          continue;
        }
        if (sy < 0 || sy >= target_h)
        {
          continue;
        }
        // sample rate 3
        // x = 1
        // y = 0
        // 36~44: supersampled red
        // this goes to 4 of render target
        for (int super_x = 0; super_x < sample_rate; super_x++)
        {
          for (int super_y = 0; super_y < sample_rate; super_y++)
          {
            float x = (float)sx + d * super_x + d / 2;
            float y = (float)sy + d * super_y + d / 2;
            if (!triangle.isInside(x, y))
            {
              continue;
            }

            int screen_index = sx + sy * target_w;

            int red_render = 4 * screen_index;
            int green_render = 4 * screen_index + 1;
            int blue_render = 4 * screen_index + 2;
            int alpha_render = 4 * screen_index + 3;

            int super_offset = super_x + super_y * sample_rate;

            int red_super = squared * red_render + super_offset;
            int green_super = squared * green_render + super_offset;
            int blue_super = squared * blue_render + super_offset;
            int alpha_super = squared * alpha_render + super_offset;

            supersample_target[red_super] = (uint8_t)(color.r * 255);
            supersample_target[green_super] = (uint8_t)(color.g * 255);
            supersample_target[blue_super] = (uint8_t)(color.b * 255);
            supersample_target[alpha_super] = (uint8_t)(color.a * 255);
          }
        }
      }
    }
  }

  void SoftwareRendererImp::rasterize_image(float x0, float y0,
                                            float x1, float y1,
                                            Texture &tex)
  {
    // Task 4:
    // Implement image rasterization (you may want to call fill_sample here)
  }

  // resolve samples to render target
  void SoftwareRendererImp::resolve(void)
  {

    // Task 2:
    // Implement supersampling
    // You may also need to modify other functions marked with "Task 2".

    // sample rate 3
    // x = 1
    // y = 0
    // 36~44: supersampled red
    // this goes to 4 of render target
    int squared = sample_rate * sample_rate;
    for (int sx = 0; sx < target_w; sx++)
    {
      for (int sy = 0; sy < target_h; sy++)
      {

        int screen_index = sx + sy * target_w;

        for (int rgba = 0; rgba < 4; rgba++)
        {
          int render_index = 4 * screen_index + rgba;
          int supersample_index = squared * render_index;
          uint8_t color_value = utils::average(supersample_target, supersample_index, supersample_index + squared);
          if (color_value == 255)
            continue;
          render_target[render_index] = color_value;
        }
      }
    }

    // flush
    std::fill(supersample_target.begin(), supersample_target.end(), 255);
    return;
  }

  Color SoftwareRendererImp::alpha_blending(Color pixel_color, Color color)
  {
    // Task 5
    // Implement alpha compositing
    return pixel_color;
  }

} // namespace CS248
