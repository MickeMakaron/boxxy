#ifndef GEOM_HPP
#define GEOM_HPP

#include <type_traits>
#include <cstring>
#include <array>
#include <vector>

namespace geom
{
  struct Vector2
  {
    // Vector2()
    // {

    // }

    // Vector2(float xx, float yy)
    // : x(xx)
    // , y(yy)
    // {

    // }

    float x;
    float y;
  };
  static_assert(std::is_trivial<Vector2>::value);

  struct Vector3
  {
    float x;
    float y;
    float z;
  };
  static_assert(std::is_trivial<Vector3>::value);


  struct Vector4
  {
    float x;
    float y;
    float z;
    float w;
  };
  static_assert(std::is_trivial<Vector4>::value);



  class Transform2
  {
    public:
      void scale(float amount)
      {

      }

    private:
      // Matrix2
  };

  class Transform3
  {
    public:
      void scale(float amount)
      {

      }

    private:
      // Matrix3
  };

  class Transformable2
  {
    public:
      Transform2& transform()
      {
        return _transform;
      }


    private:
      Transform2 _transform;
  };
  class Transformable3
  {
    public:
      Transform3& transform()
      {
        return _transform;
      }


    private:
      Transform3& _transform;
  };

  class Triangle
  {
    public:
      static constexpr size_t SIZE = 3;

      static constexpr size_t size()
      {
        return SIZE;
      }
  };

  class Triangle2 : public Triangle, public Transformable2
  {
    public:
      Vector2& operator[](size_t index)
      {
        return _points[index];
      }

    private:
      std::array<Vector2, SIZE> _points;
  };

  class Triangle3 : public Triangle, public Transformable3
  {
    public:
      Vector3& operator[](size_t index)
      {
        return _points[index];
      }

    private:
      std::array<Vector3, SIZE> _points;
  };


  class Rect
  {
    public:
      static constexpr size_t SIZE = 4;


      static constexpr size_t size()
      {
        return SIZE;
      }
  };

  class Rect2 : public Rect, public Transformable2
  {
    public:
      Vector2& top_left()
      {
        return _points[0];
      }
      Vector2& top_right()
      {
        return _points[1];
      }
      Vector2& bot_left()
      {
        return _points[2];
      }
      Vector2& bot_right()
      {
        return _points[3];
      }

      Vector2& operator[](size_t index)
      {
        return _points[index];
      }

    private:
      std::array<Vector2, Rect::SIZE> _points;
  };

  class Rect3 : public Rect, public Transformable3
  {
    public:
      Vector3& top_left()
      {
        return _points[0];
      }
      Vector3& top_right()
      {
        return _points[1];
      }
      Vector3& bot_left()
      {
        return _points[2];
      }
      Vector3& bot_right()
      {
        return _points[3];
      }

      Vector3& operator[](size_t index)
      {
        return _points[index];
      }


    private:
      std::array<Vector3, Rect::SIZE> _points;
  };

  class Line2 : public Transformable2
  {
    public:
      virtual ~Line2()
      {

      }

      Vector2& operator[](size_t index)
      {
        return _points[index];
      }


      Vector2& front()
      {
        return _points.front();
      }

      Vector2& back()
      {
        return _points.back();
      }

      void push_back(const Vector2& position)
      {
        _points.push_back(position);
      }

      void insert(size_t index, const Vector2& position)
      {
        _points.insert(_points.begin() + index, position);
      }

      void resize(size_t size)
      {
        _points.resize(size);
      }

      void reserve(size_t size)
      {
        _points.reserve(size);
      }

      size_t size() const
      {
        return _points.size();
      }


    private:
      std::vector<Vector2> _points;
  };
}


#endif // GEOM_HPP
