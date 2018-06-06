#include <iostream>
#include <vector>
#include <sstream>
#include <random>
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>

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
    in vec4 colorIn;
    out vec4 color;
    void main()
    {
      gl_Position.xyz = vertexPosition_modelspace;
      gl_Position.w = 1.0;
      color = colorIn;
    }

  )EOF";

  static const char* fsSrc = R"EOF(

    #version 130
    out vec4 colorOut;
    in vec4 color;

    void main()
    {
      colorOut = color;
    }

  )EOF";

  compileShader(vs, vsSrc);
  compileShader(fs, fsSrc);

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

  glBindAttribLocation(prog, 0, "vertexPosition_modelspace");
  glBindAttribLocation(prog, 1, "colorIn");

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

struct Rectangle
{
  float x;
  float y;
  float width;
  float height;
};

struct Color
{
  float r;
  float g;
  float b;
  float a;
};


struct Vertex
{
  float x;
  float y;
  float z;
};

std::array<Vertex, 6> get_triangles(const Rectangle& rect)
{
  std::array<Vertex, 6> vertices;
  vertices[0] = {rect.x, rect.y};
  vertices[1] = {rect.x, rect.y - rect.height};
  vertices[2] = {rect.x + rect.width, rect.y - rect.height};
  vertices[3] = {rect.x, rect.y};
  vertices[4] = {rect.x + rect.width, rect.y - rect.height};
  vertices[5] = {rect.x + rect.width, rect.y};

  for(Vertex& v : vertices)
  {
    v.z = 1.f;
  }
  return vertices;
}

struct Texture
{
  std::vector<Color> pixels;
  size_t width;
  size_t height;
};

void draw_rectangle(const Rectangle& rect, const Texture& texture, const Rectangle& uv_rect = {0.f, 0.f, 1.f, 1.f})
{

}

void draw_rectangle(const Rectangle& rect, const Color& color)
{
  GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	std::array<Vertex, 6> vertices = get_triangles(rect);

	struct ColorVertex
	{
    Vertex v;
    Color c;
	};
  std::array<ColorVertex, 6> data;
  for(size_t i = 0; i < data.size(); i++)
  {
    data[i].v = vertices[i];
    data[i].c = color;
  }


	GLuint vbo;
	glGenBuffers(1, &vbo);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(ColorVertex), data.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), 0);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ColorVertex), (char*)0 + sizeof(Vertex));

  glDrawArrays(GL_TRIANGLES, 0, data.size());

  glDeleteBuffers(1, &vbo);
  glDeleteVertexArrays(1, &vao);
}

int main(int, char**)
{


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

  glViewport(0, 0, 800, 800);
//  glDisable(GL_DEPTH_TEST);


  createShaderProgram();
//  GLuint vao = createSquareVao();
  size_t width = 1024;
//  GLuint tex = createTexture(width);
//
//  GLuint buf;
//  glGenBuffers(1, &buf);
//
//  glBindTexture(GL_TEXTURE_2D, tex);
//  glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, buf);


  size_t bufIndex = 0;
  size_t height = width;
  size_t numPixels = width * height;
  size_t size = 4 * numPixels;
  size_t numBufferRanges = 3;
  size_t frameIndex = 0;
//  glBufferData(GL_PIXEL_UNPACK_BUFFER, size * numBufferRanges, 0, GL_STREAM_DRAW);
  throwGlError("failed buffer data");
  bool done = false;
  std::chrono::steady_clock::time_point time1 = std::chrono::steady_clock::now();
  double frameTime = 0.d;
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

//    glBindVertexArray(vao);
//    glBindTexture(GL_TEXTURE_2D, tex);
//    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, buf);
//
//
//    // copy pixels from PBO to texture object
//    // Use offset instead of ponter.
//    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
//                    GL_BGRA, GL_UNSIGNED_BYTE, (void*)(bufIndex * size));
//    bufIndex = (bufIndex + 1) % numBufferRanges;
//    throwGlError("failed gl tex");
//
//    struct Pixel
//    {
//      unsigned char r;
//      unsigned char g;
//      unsigned char b;
//      unsigned char a;
//    };
//    Pixel* ptr = (Pixel*)glMapBufferRange(GL_PIXEL_UNPACK_BUFFER,
//                                          bufIndex * size,
//                                          size,
//                                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
//    throwGlError("failed map buffer");
//
//
//
//    if(ptr)
//    {
//      //memset(ptr, 0, size);
//
//      // frameIndex = (frameIndex + 1) % numPixels;
//      // Pixel black = {0, 0, 0, 255};
//      // Pixel white = {255, 255, 255, 255};
//
//      // for(size_t i = 0; i < frameIndex; i++)
//      // {
//      //   ptr[i] = {rand() % 155 + 100, rand() % 155 + 100, rand() % 155 + 100, 255};
//      // }
//      // for(size_t i = frameIndex; i < numPixels; i++)
//      // {
//      //   ptr[i] = {rand() % 155, rand() % 155, rand() % 155, 255};
//      // }
//
//      glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER_ARB);
//      throwGlError("unmap buffer failed");
//    }
//    else
//    {
//      throwGlError("map buffer failed");
//    }
//
//    // it is good idea to release PBOs with ID 0 after use.
//    // Once bound with 0, all pixel operations are back to normal ways.
//    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
//
//    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    Rectangle rect;
    rect.x = -0.5f;
    rect.y = 0.5f;
    rect.width = 1.f;
    rect.height = 1.f;
    Color color = {1.f, 0, 1.f, 1.f};
    draw_rectangle(rect, color);
    SDL_GL_SwapWindow(window);

    // Rendering
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    const std::chrono::steady_clock::time_point time2 = std::chrono::steady_clock::now();
    double currentFrameTime = std::chrono::duration<double>(time2 - time1).count();
    frameTime = currentFrameTime * 0.1d + frameTime * 0.9d;
    std::cout << "FPS: " << 1.d / frameTime << std::endl;
    time1 = time2;
  }

  SDL_GL_DeleteContext(gl_context);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}
