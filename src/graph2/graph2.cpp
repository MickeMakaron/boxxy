#include "graph2.hpp"

#include <atomic>

// API implementation
namespace graph2
{
  static_assert(std::is_trivial<Color>::value);
  static_assert(std::is_trivial<Point>::value);
  static_assert(std::is_trivial<Triangle>::value);
  static_assert(std::is_trivial<Rect>::value);
  static_assert(std::is_trivial<DrawConfig>::value);

  // Triangle
  Point& Triangle::a()
  {
    return operator[](0);
  }

  Point& Triangle::b()
  {
    return operator[](1);
  }

  Point& Triangle::c()
  {
    return operator[](2);
  }

  Triangle& Triangle::operator=(const std::array<Vector2, 3>& positions)
  {
    for(size_t i = 0; i < size(); i++)
    {
      operator[](i).position = positions[i];
    }
  }

  // Rect
  Point& Rect::top_left()
  {
    return operator[](0);
  }

  Point& Rect::top_right()
  {
    return operator[](1);
  }

  Point& Rect::bot_left()
  {
    return operator[](2);
  }

  Point& Rect::bot_right()
  {
    return operator[](3);
  }


  class Id
  {
    public:
      Id();


    private:
      uint64_t _id;
  };

  class Token
  {
    private:
      Id _id;
      uint64_t _version;
  };


  // Id
  Id::Id()
  {
    static std::atomic<uint64_t> id_pool = 0;
    _id = id_pool++;
  }


  DrawConfig& get_config(const Token& token)
  {
    static DrawConfig config;
    return config;
  }

  const DrawConfig& get_config_readonly(const Token& token)
  {
    return get_config(token);
  }

  Triangle& get_triangle(const Token& token)
  {
    static Triangle triangle;
    return triangle;
  }

  const Triangle& get_triangle_readonly(const Token& token)
  {
    return get_triangle(token);
  }

  Rect& get_rect(const Token& token)
  {
    static Rect rect;
    return rect;
  }

  const Rect& get_rect_readonly(const Token& token)
  {
    return get_rect(token);
  }

  Line& get_line(const Token& token)
  {
    static Line line;
    return line;
  }

  const Line& get_line_readonly(const Token& token)
  {
    return get_line(token);
  }

  void draw(const Token& token)
  {

  }

  void draw(const Triangle& triangle, const DrawConfig& config)
  {

  }

  void draw(const Rect& rect, const DrawConfig& config)
  {

  }

  void draw(const Line& line, const DrawConfig& config)
  {

  }

  void attach_config(const Token& from, const Token& to)
  {

  }

  void detach_config(const Token& attached)
  {

  }

  const Token& take_token()
  {
    static Token token;
    return token;
  }

  void return_token(const Token& token)
  {

  }



  // Graphic
  Graphic::Graphic()
  : _token(take_token())
  {

  }

  Graphic::~Graphic()
  {
    return_token(_token);
  }

  DrawConfig& Graphic::config() const
  {
    return get_config(_token);
  }

  const DrawConfig& Graphic::config_readonly() const
  {
    return get_config_readonly(_token);
  }

  void Graphic::attach_config(const Graphic& to) const
  {
    graph2::attach_config(_token, to._token);
  }

  void Graphic::detach_config() const
  {
    graph2::detach_config(_token);
  }

  void Graphic::draw() const
  {
    graph2::draw(_token);
  }

  const Token& Graphic::token() const
  {
    return _token;
  }

  // Graphics
  Triangle& TriangleGraphic::triangle()
  {
    return get_triangle(token());
  }

  Rect& RectGraphic::rect()
  {
    return get_rect(token());
  }

  Line& LineGraphic::line()
  {
    return get_line(token());
  }

  std::vector<TriangleGraphic> create_triangle_group()
  {
    return {};
  }

  std::vector<RectGraphic> create_rect_group()
  {
    return {};
  }

  std::vector<LineGraphic> create_line_group()
  {
    return {};
  }
}
