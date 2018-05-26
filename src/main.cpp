#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <array>
 

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
      float x;
      float y;
  };

  struct Vector3 : public Vector2
  {
      float z;
  };

  struct Vector4 : public Vector3
  {
      float w;
  };


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


  class Rect
  {
    public:
      static constexpr size_t SIZE = 4;

      static constexpr size_t top_left_index()
      {
        return 0;
      }
      static constexpr size_t top_right_index()
      {
        return 1;
      }
      static constexpr size_t bot_left_index()
      {
        return 2;
      }
      static constexpr size_t bot_right_index()
      {
        return 3;
      }
    
      static constexpr size_t size()
      {
        return SIZE;
      }
  };

  class Rect2 : public Rect, public Transformable2
  {
    public:
      Vector2& position(size_t index)
      {
        return _points[index];
      }

    private:
      std::array<Vector2, Rect::SIZE> _points;
  };

  class Rect3 : public Rect, public Transformable3
  {
    public:
      Vector3& position(size_t index)
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

      Vector2& position(size_t index)
      {
        return _points[index];
      }

      Vector2& first_position()
      {
        return _points.front();
      }

      Vector2& last_position()
      {
        return _points.back();
      }

      virtual void append()
      {
        _points.emplace_back(Vector2());
      }

      virtual void insert(size_t index)
      {
        _points.insert(_points.begin() + index, Vector2());
      }

      virtual void resize(size_t size)
      {
        _points.resize(size);
      }

      virtual void reserve(size_t size)
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

  struct Color
  {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  };

  class Triangle
  {
  };

  struct Point
  {
    geom::Vector2 pos;
    Color color;
  };


  class Drawable
  {
    public:
     virtual ~Drawable()
     {

     }

     virtual void draw() const = 0;
     virtual void colorize(const Color& color) = 0;
    
      uint16_t& layer()
      {
        return _layer;
      }

    private:
      uint16_t _layer;
  };


  
  class Rect : public geom::Rect2, public Drawable
  {
    public:
      Color& color(size_t index)
      {
        return _colors[index];
      }

      void draw() const override
      {

      }

      void colorize(const Color& color) override
      {
        for(Color& c : _colors)
        {
          c = color;
        }
      }


    private:
      std::array<Color, geom::Rect2::SIZE> _colors;
  };
  

  class Line : public geom::Line2, public Drawable
  {
    public:
      Line()
      {

      }

      Line(const geom::Line2& other)
      {
        resize(other.size());
      }

      Color& color(size_t index)
      {
        return _colors[index];
      }

      Color& first_color()
      {
        return _colors.front();
      }

      Color& last_color()
      {
        return _colors.back();
      }

      void append() override
      {
        geom::Line2::append();
        _colors.emplace_back(Color());
      }

       void insert(size_t index) override
      {
        geom::Line2::insert(index);
        _colors.insert(_colors.begin() + index, Color());
      }

      void draw() const override
      {
      
      }

      void colorize(const Color& color) override
      {
        for(Color& c : _colors)
        {
          c = color;
        }
      }

      float& thickness()
      {
        return _thickness;
      }

      void resize(size_t size) override
      {
        geom::Line2::resize(size);
        _colors.resize(size);
      }

      void reserve(size_t size) override
      {
        geom::Line2::reserve(size);
        _colors.reserve(size);
      }

    private:
      std::vector<Color> _colors;
      float _thickness;
  };

  void draw(const Drawable& drawable)
  {
    drawable.draw();
  }
}

int main(int, char**) {

  // Triangle t;
  // t.set_pos(p1, p2, p3);
  // t.set_color(c1, c2, c3)
  // t.draw();

  graph2d::Rect r;
  r.color(0) = graph2d::Color();
  r.position(3) = graph2d::Vector2();
  r.position(r.bot_right_index()) = r.position(r.top_left_index());
  r.transform().scale(5.f);

  r.draw();


  graph2d::Line l;
  l.resize(50);
  l.append();
  l.insert(0);
  l.last_position().y += 5;
  l.last_color() = graph2d::Color();

  l.position(0) = graph2d::Vector2();

  l.colorize(graph2d::Color());
  l.draw();


  
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
