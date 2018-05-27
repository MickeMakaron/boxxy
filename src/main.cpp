#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <array>
#include <atomic>
 

#include <GL/glew.h>
#include <SDL.h>


void printShaderLog(GLuint shader)
{
  int logLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0)
  {
		std::vector<char> msg(logLength + 1);
		glGetShaderInfoLog(shader, logLength, NULL, &msg[0]);
		std::cout << msg.data() << std::endl;
	}
}

void printProgramLog(GLuint prog)
{
  int logLength = 0;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0)
  {
		std::vector<char> msg(logLength + 1);
		glGetProgramInfoLog(prog, logLength, NULL, &msg[0]);
		std::cout << msg.data() << std::endl;
	}
}

void compileShader(GLuint shader, const char* src)
{
  glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
  printShaderLog(shader);
}

void throwGlError(const char* msg)
{
  GLenum error = glGetError();
  if (error != GL_NO_ERROR)
  {
    std::stringstream sstream;
    sstream << "GL ERROR " << error << ": \"" << msg << "\"" << std::endl;
    throw std::runtime_error(sstream.str());
  }
}

void createShaderProgram()
{
	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);

	static const char* vsSrc = R"EOF(

    #version 130
    in vec3 vertexPosition_modelspace;
    in vec2 uvIn;
    out vec2 uv;
    void main()
    {
      gl_Position.xyz = vertexPosition_modelspace;
      gl_Position.w = 1.0;
      uv = uvIn;
    }

  )EOF";

  static const char* fsSrc = R"EOF(

    #version 130
    out vec3 color;
    in vec2 uv;

    uniform sampler2D textureSampler;

    void main()
    {
      color = texture(textureSampler, uv).rgb;
    }

  )EOF";

  compileShader(vs, vsSrc);
  compileShader(fs, fsSrc);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

  glBindAttribLocation(prog, 0, "vertexPosition_modelspace");
  glBindAttribLocation(prog, 1, "uvIn");

	glLinkProgram(prog);

	printProgramLog(prog);

	glDetachShader(prog, vs);
	glDetachShader(prog, fs);
	glDeleteShader(vs);
	glDeleteShader(fs);

	glUseProgram(prog);
	throwGlError("Failed to create shader program");
}

GLuint createVao(const std::vector<GLfloat>& data)
{
  GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (char*)(sizeof(GLfloat) * 3));

  throwGlError("Failed to create square VAO");
  return vao;
}

GLuint createSquareVao()
{
  return createVao
  ({
    -1.f,  -1.f,  0.0f, 0.f, 0.f,
    1.f,   -1.f,  0.0f, 1.f, 0.f,
    -1.f,  1.f,   0.0f, 0.f, 1.f,
    1.f,   1.f,   0.0f, 1.f, 1.f
  });
}

GLuint createRectangleVao(float uvTop, float uvHeight)
{
  float uvBot = uvTop - uvHeight;
  return createVao
  ({
    -1.f,  -1.f,  0.0f, 0.f, uvBot,
    1.f,   -1.f,  0.0f, 1.f, uvBot,
    -1.f,  1.f,   0.0f, 0.f, uvTop,
    1.f,   1.f,   0.0f, 1.f, uvTop
  });
}


struct Pixel
{
  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;
};


std::vector<Pixel> generatePixels(size_t count)
{

	std::vector<Pixel> pixels(count);

  std::random_device rnd_device;
  std::mt19937 mersenne(rnd_device());
  std::uniform_int_distribution<unsigned char> dist(0, 255);
  for(Pixel& p : pixels)
  {
    p = {dist(mersenne), dist(mersenne), dist(mersenne)};
  }

  return pixels;
}



GLuint createTexture(size_t resolution)
{
  std::vector<Pixel> pixels = generatePixels(resolution * resolution);
  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, resolution, resolution, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

  throwGlError("Failed to create texture.");
  return tex;
}

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
  static_assert(std::is_pod<Vector2>::value);

  struct Vector3
  {
    float x;
    float y;
    float z;
  };
  static_assert(std::is_pod<Vector3>::value);
  

  struct Vector4
  {
    float x;
    float y;
    float z;
    float w;
  };
  static_assert(std::is_pod<Vector4>::value);
  


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

namespace graph2d
{
  using Vector2 = geom::Vector2;
  using Vector3 = geom::Vector3;
  using Vector4 = geom::Vector4;
  using Transform = geom::Transform2;

  struct Color
  {
    // Color()
    // {

    // }

    // Color(unsigned char rr, 
    //       unsigned char gg, 
    //       unsigned char bb, 
    //       unsigned char aa = 255)
    // : r(rr)
    // , g(gg)
    // , b(bb)
    // , a(aa)
    // {

    // }

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  };
  static_assert(std::is_pod<Color>::value);




  struct Point
  {
    Vector2 position;
    Color color;
  };
  static_assert(std::is_pod<Point>::value);

  

  struct Triangle : public std::array<Point, 3>
  {
    Point& a()
    {
      return operator[](0);
    }

    Point& b()
    {
      return operator[](1);
    }

    Point& c()
    {
      return operator[](2);
    }

    Triangle& operator=(const std::array<Vector2, 3>& positions)
    {
      for(size_t i = 0; i < size(); i++)
      {
        operator[](i).position = positions[i];
      }
    }
  };
  static_assert(std::is_pod<Triangle>::value);

  struct Rect : public std::array<Point, 4>
  {
    Point& top_left()
    {
      return operator[](0);
    }

    Point& top_right()
    {
      return operator[](1);
    }

    Point& bot_left()
    {
      return operator[](2);
    }

    Point& bot_right()
    {
      return operator[](3);
    }
  };
  static_assert(std::is_pod<Rect>::value);

  struct Line : public std::vector<Point>
  {
  };
  
  struct DrawConfig
  {
    Transform transform;
    uint16_t layer;
    Vector4 color_multiplier;
  };

  class Id
  {
    public:
      Id()
      {
        static std::atomic<uint64_t> id_pool = 0;
        _id = id_pool++;
      }


    private:
      uint64_t _id;
  };

  class Graphic
  {
    public:
      DrawConfig& config()
      {
        return _config;
      }

      void draw() const
      {

      }

    protected:
      void set_dirty()
      {
        _version++;
      }

      //virtual void draw_internal() const = 0;

    private:
      DrawConfig _config;
      Id _id;
      uint64_t _version;
  };

  class TriangleGraphic : public Graphic
  {
    public:
      Triangle& triangle()
      {
        set_dirty();
        return _triangle;
      }

    private:
      Triangle _triangle;
  };

  class RectGraphic : public Graphic
  {
    public:
      Rect& rect()
      {
        set_dirty();
        return _rect;
      }


    private:
      Rect _rect;
  };

  class LineGraphic : public Graphic
  {
    public:
      Line& line()
      {
        set_dirty();
        return _line;
      }

    private:
      Line _line;
  };

  // class Triangle : public Drawable
  // {
  //   public:
  //     Point& operator[](size_t index) override
  //     {
  //       return _points[index];
  //     }

  //     void draw() const override
  //     {
        
  //     }

  //     size_t size() const override
  //     {
  //       return _points.size();
  //     }


  //   private:
  //     std::array<Point, geom::Triangle2::SIZE> _points;
  // };
  
  // class Rect : public Drawable
  // {
  //   public:
  //     Point& top_left()
  //     {
  //       return operator[](0);
  //     }
  //     Point& top_right()
  //     {
  //       return operator[](1);
  //     }
  //     Point& bot_left()
  //     {
  //       return operator[](2);
  //     }
  //     Point& bot_right()
  //     {
  //       return operator[](3);
  //     }

  //     Point& operator[](size_t index) override
  //     {
  //       return _points[index];
  //     }

  //     void draw() const override
  //     {

  //     }

      

  //     size_t size() const override
  //     {
  //       return _points.size();
  //     }


  //   private:
  //     std::array<Point, geom::Rect2::SIZE> _points;
  // };
  

  // class Line : public Drawable
  // {
  //   public:
  //     Line()
  //     {

  //     }

  //     Line(const geom::Line2& other)
  //     {
  //       resize(other.size());
  //     }

  //     Point& operator[](size_t index)
  //     {
  //       return {_positions[index], _colors[index]};
  //     }
      
  //     Point& front()
  //     {
  //       return {_positions.front(), _colors.front()};
  //     }

  //     Point& back()
  //     {
  //       return {_positions.back(), _colors.back()};
  //     }

  //     void push_back(const Point& point)
  //     {
  //       _positions.push_back(point.position);
  //       _colors.push_back(point.color);
  //     }

  //     void insert(size_t index, const Point& point)
  //     {
  //       _positions.insert(index, point.position);
  //       _colors.insert(_colors.begin() + index, point.color);
  //     }

  //     void push_back(const PointRef& point)
  //     {
  //       _positions.push_back(point.position);
  //       _colors.push_back(point.color);
  //     }

  //     void insert(size_t index, const PointRef& point)
  //     {
  //       _positions.insert(index, point.position);
  //       _colors.insert(_colors.begin() + index, point.color);
  //     }

      

  //     void draw() const override
  //     {
      
  //     }

  //     size_t size() const override
  //     {
  //       return _colors.size();
  //     }

  //     float& thickness()
  //     {
  //       return _thickness;
  //     }

  //     void resize(size_t size)
  //     {
  //       _positions.resize(size);
  //       _colors.resize(size);
  //     }

  //     void reserve(size_t size)
  //     {
  //       _positions.reserve(size);
  //       _colors.reserve(size);
  //     }

  //   private:
  //     geom::Line2 _positions;
  //     std::vector<Color> _colors;
  //     float _thickness;
  // };

  void draw(const Graphic& graphic)
  {
    graphic.draw();
  }
}


int main(int, char**) 
{
  graph2d::TriangleGraphic triangle;
  triangle.triangle() = 
  {
    graph2d::Vector2({-0.5f, -0.5f}),
    graph2d::Vector2({0.f, 0.5f}),
    graph2d::Vector2({0.5f, -0.5f}),
  };
  graph2d::Color red = {255, 0, 0, 255};
  triangle.triangle() =
  {
    graph2d::Point({{-0.5f, -0.5f}, red}),
    graph2d::Point({{0.f, 0.5f}, red}),
    graph2d::Point({{0.5f, -0.5f}, red}),
  };

  triangle.config().transform.scale(5.f);


  triangle.draw();


  graph2d::LineGraphic line;
  graph2d::Line& l = line.line();
  l.resize(50);
  for(size_t i = 0; i < l.size(); i++)
  {
    graph2d::Point p;
    p.color.r = i;
    p.position.x = i * 2;
    l[i] = p;
  }

  graph2d::DrawConfig& conf = line.config();
  conf.transform.scale(5.f);
  conf.layer = 50;

  line.draw();
  
  // graph2d::Triangle t;
  // t[0].position.y = 1;
  // t[2].color.g = 0;
  // t.draw();


  // graph2d::Rect r;
  // r[0].color = graph2d::Color();
  // r[3].position = graph2d::Vector2();
  // r.top_left().position = r.top_right().position;
  // r.transform().scale(5.f);

  // r.draw();


  // graph2d::Line l;
  // l.resize(50);
  // l.push_back({graph2d::Vector2(), graph2d::Color()});
  // l.insert(0, l.back());
  // l.front().position.x += 5;
  // l.back().color = graph2d::Color();
  // l[0].position = graph2d::Vector2();

  // for(size_t i = 10; i < l.size(); i++)
  // {
  //   l[i].position.y = i;
  // }

  // l.colorize(graph2d::Color());
  // l.draw();


  
  // r.set_middle(5, 2);
  // r.set_size(10, 20);
  // r.set_color(c1, 0);
  // r.set_color(c)

// Setup SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
  {
    printf("Error: %s\n", SDL_GetError());
    return -1;
  }

  // Setup window
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
                      SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
  SDL_DisplayMode current;
  SDL_GetCurrentDisplayMode(0, &current);
  SDL_Window *window =
      SDL_CreateWindow("ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED,
                       SDL_WINDOWPOS_CENTERED, 800, 800,
                       SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
  SDL_GLContext gl_context = SDL_GL_CreateContext(window);
  SDL_GL_SetSwapInterval(1); // Enable vsync
  glewInit();

  glEnable(GL_DEPTH_TEST);


  createShaderProgram();
  GLuint vao = createSquareVao();
  GLuint tex = createTexture(256);

  bool done = false;
  while (!done)
  {
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
      switch (event.type)
      {
      case SDL_QUIT:
        done = true;
        break;
      case SDL_MOUSEBUTTONDOWN:
        if (event.button.button == SDL_BUTTON_RIGHT)
        break;
      case SDL_KEYDOWN:
      {
        break;
      }
      case SDL_KEYUP:
      {
        break;
      }
      }
    }

    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, tex);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    SDL_GL_SwapWindow(window);
   
    // Rendering
    glViewport(0, 0, 800, 800);
    glClearColor(255, 0, 255, 255);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
