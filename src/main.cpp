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


class Transformable2
{
  public:

    void move(float x, float y)
    {

    }

    void scale(float amount)
    {

    }

  private:
    geom::Transform2 _transform;
};

class Rect2 : public Transformable2
{
  public:
    void set_center(float x, float y)
    {

    }

    void set_top_left(float x, float y)
    {

    }



  private:
};

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

namespace gui2
{
  class Button : public Rect2
  {
    public:

  };
}

int main(int, char**) 
{
  gui2::Button b;
  b.set_center(50, 50);
  // b.set_size(10, 10);
  b.set_top_left(20, 20);
  // b.set_bot_right(30, 30);
  b.move(10, 10);
  b.scale(10.f);


  graph2::TriangleGraphic triangle;
  triangle.triangle() = 
  {
    graph2::Vector2({-0.5f, -0.5f}),
    graph2::Vector2({0.f, 0.5f}),
    graph2::Vector2({0.5f, -0.5f}),
  };
  graph2::Color red = {255, 0, 0, 255};
  triangle.triangle() =
  {
    graph2::Point({{-0.5f, -0.5f}, red}),
    graph2::Point({{0.f, 0.5f}, red}),
    graph2::Point({{0.5f, -0.5f}, red}),
  };

  

  triangle.config().transform.scale(5.f);


  triangle.draw();


  graph2::LineGraphic line;
  graph2::Line& l = line.line();
  l.resize(50);
  for(size_t i = 0; i < l.size(); i++)
  {
    graph2::Point p;
    p.color.r = i;
    p.position.x = i * 2;
    l[i] = p;
  }

  graph2::DrawConfig& conf = line.config();
  conf.transform.scale(5.f);
  conf.layer = 50;

  line.draw();
  
  // graph2::Triangle t;
  // t[0].position.y = 1;
  // t[2].color.g = 0;
  // t.draw();


  // graph2::Rect r;
  // r[0].color = graph2::Color();
  // r[3].position = graph2::Vector2();
  // r.top_left().position = r.top_right().position;
  // r.transform().scale(5.f);

  // r.draw();


  // graph2::Line l;
  // l.resize(50);
  // l.push_back({graph2::Vector2(), graph2::Color()});
  // l.insert(0, l.back());
  // l.front().position.x += 5;
  // l.back().color = graph2::Color();
  // l[0].position = graph2::Vector2();

  // for(size_t i = 10; i < l.size(); i++)
  // {
  //   l[i].position.y = i;
  // }

  // l.colorize(graph2::Color());
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
