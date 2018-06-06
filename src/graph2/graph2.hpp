#ifndef GRAPH2_HPP
#define GRAPH2_HPP

#include "geom/geom.hpp"


// API declaration
namespace graph2
{
  using Vector2 = geom::Vector2;
  using Vector3 = geom::Vector3;
  using Vector4 = geom::Vector4;
  using Transform = geom::Transform2;

  struct Color
  {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  };

  struct Point
  {
    Vector2 position;
    Color color;
  };

  struct Triangle : public std::array<Point, 3>
  {
    Point& a();
    Point& b();
    Point& c();

    Triangle& operator=(const std::array<Vector2, 3>& positions);
  };

  struct Rect : public std::array<Point, 4>
  {
    Point& top_left();
    Point& top_right();
    Point& bot_left();
    Point& bot_right();
  };

  struct Line : public std::vector<Point>
  {
    float thickness;
  };

  struct DrawConfig
  {
    Transform transform;
    uint16_t layer;
    Vector4 color_multiplier;
  };

  class Token;
  class Graphic
  {
    public:
      Graphic();
      ~Graphic();
      DrawConfig& config() const;
      const DrawConfig& config_readonly() const;
      void attach_config(const Graphic& to) const;
      void detach_config() const;
      void draw() const;

    protected:
      const Token& token() const;

    private:
      const Token& _token;
  };

  class TriangleGraphic : public Graphic
  {
    public:
      Triangle& triangle();
  };

  class RectGraphic : public Graphic
  {
    public:
      Rect& rect();
  };

  class LineGraphic : public Graphic
  {
    public:
      Line& line();
  };

  std::vector<TriangleGraphic> create_triangle_group();
  std::vector<RectGraphic> create_rect_group();
  std::vector<LineGraphic> create_line_group();
}

#endif // GRAPH2_HPP
