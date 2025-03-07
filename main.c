#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

#if defined __APPLE__
#  define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl3.h>
#else
#  include <GL/gl.h>
#  include <GLES3/gl3.h>
#endif

#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H


typedef struct { int x, y; }    int2;
typedef struct { int x, y, z; } int3;

typedef struct { float x, y; }    float2;
typedef struct { float x, y, z; } float3;

typedef float mat4[4][4];

struct character {
  unsigned int texture_id, advance_pixels;
  int2 size, bearing;
};


static struct {
  struct {
    size_t width, height;
  } window;

  struct {
    unsigned int vao, vbo;
    unsigned int vertex_shader, fragment_shader;
    unsigned int program;
    mat4 projection;
    struct character atlas[128];
  } font;
} app;


static const char *glyph_vertex_shader_program =
#include "shaders/glyph_vertex.glsl.inc"
;

static const char *glyph_fragment_shader_program =
#include "shaders/glyph_fragment.glsl.inc"
;

bool load_font_atlas(const char *font_path, int font_height_pt) {
  FT_Library library;
  FT_Face face;

  FT_Init_FreeType(&library);

  if (FT_New_Face(library, font_path, 0, &face)) {
    failure("Could not load font");
    return false;
  }

  FT_Set_Char_Size(face, 0, font_height_pt * 64, 300, 300);
  info("Font loaded");

  /* Disable byte alignment */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (size_t i = 0; i < 128; i++) {
    struct character c;

    if (FT_Load_Char(face, (char) i, FT_LOAD_RENDER))
      continue;

    glGenTextures(1, &c.texture_id);
    glBindTexture(GL_TEXTURE_2D, c.texture_id);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RED,
                 face->glyph->bitmap.width,
                 face->glyph->bitmap.rows,
                 0, GL_RED, GL_UNSIGNED_BYTE,
                 face->glyph->bitmap.buffer);

    /* Texture options */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    c.size.x = face->glyph->bitmap.width;
    c.size.y = face->glyph->bitmap.rows;

    c.bearing.x = face->glyph->bitmap_left;
    c.bearing.y = face->glyph->bitmap_top;

    c.advance_pixels = face->glyph->advance.x >> 6;

    app.font.atlas[i] = c;
  }

  info("Font atlas loaded");

  FT_Done_Face(face);
  FT_Done_FreeType(library);

  return true;
}

void orthographic(mat4 mat,
                  float left, float right,
                  float bottom, float top,
                  float near, float far) {
  mat[0][0] = 2.0f / (right - left);
  mat[1][0] = 0.0f;
  mat[2][0] = 0.0f;
  mat[3][0] = 0.0f;

  mat[0][1] = 0.0f;
  mat[1][1] = 2.0f / (top - bottom);
  mat[2][1] = 0.0f;
  mat[3][1] = 0.0f;

  mat[0][2] = 0.0f;
  mat[1][2] = 0.0f;
  mat[2][2] = -2.0f / (far - near);
  mat[3][2] = 0.0f;

  mat[0][3] = -(right + left) / (right - left);
  mat[1][3] = -(top + bottom) / (top - bottom);
  mat[2][3] = -(far + near) / (far - near);
  mat[3][3] = 1.0f;
}

void display_text(const char *text, float2 pos, float scale, float3 colour) {
  glUseProgram(app.font.program);

  glUniform1i(glGetUniformLocation(app.font.program, "text"), 0);
  glUniform3f(glGetUniformLocation(app.font.program, "textColour"),
              colour.x, colour.y, colour.z);

  glBindVertexArray(app.font.vao);

  while (*text != '\0') {
    struct character c = app.font.atlas[*(text++)];

    float xpos = pos.x + c.bearing.x * scale;
    float ypos = pos.y - (c.size.y - c.bearing.y) * scale;

    float w = c.size.x * scale;
    float h = c.size.y * scale;

    // Adjusted texture coordinates to flip V component
    float vertices[6][4] = {
      { xpos,     ypos + h,   0.0f, 0.0f }, // Top-left (s=0, t=0)
      { xpos,     ypos,       0.0f, 1.0f }, // Bottom-left (s=0, t=1)
      { xpos + w, ypos,       1.0f, 1.0f }, // Bottom-right (s=1, t=1)

      { xpos,     ypos + h,   0.0f, 0.0f }, // Top-left
      { xpos + w, ypos,       1.0f, 1.0f }, // Bottom-right
      { xpos + w, ypos + h,   1.0f, 0.0f }  // Top-right (s=1, t=0)
    };

    glActiveTexture(GL_TEXTURE0); // Activate texture unit 0
    glBindTexture(GL_TEXTURE_2D, c.texture_id);

    glBindBuffer(GL_ARRAY_BUFFER, app.font.vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    pos.x += c.advance_pixels * scale;
  }

  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

void initialise_OpenGL(void) {
  glfwInit();

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  glfwWindowHint(GLFW_OPENGL_PROFILE,
                 GLFW_OPENGL_CORE_PROFILE);
#if defined __APPLE__
  info("Initialising OpenGL for Apple system");
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,
                 GLFW_TRUE);
#endif

  info("Initialised OpenGL");
}

void resize_window_viewport(GLFWwindow *_window, int width, int height) {
  glViewport(0, 0, width, height);

  app.window.width = width;
  app.window.height = height;

  info("Resized OpenGL viewport");
}

GLFWwindow *initialise_window(int width, int height, const char *title) {
  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

  if (!window) {
    failure("Could not initialise GLFW window");
    return NULL;
  }

  glfwMakeContextCurrent(window);

  glfwSetFramebufferSizeCallback(window, resize_window_viewport);
  resize_window_viewport(window, width, height);

  info("Initialised window");
  return window;
}

int compile_shader(GLenum shader_type,
                   const char *shader_source,
                   GLuint *shader) {
  static char shader_compilation_log_buffer[1024];
  int success;

  *shader = glCreateShader(shader_type);

  glShaderSource(*shader, 1, &shader_source, NULL);
  glCompileShader(*shader);

  glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);

  if (!success) {
    glGetShaderInfoLog(*shader, 1024, NULL,
                       shader_compilation_log_buffer);
    printf(ANSI_RED "%s\n" ANSI_RESET, shader_compilation_log_buffer);
  }

  else {
    info("Shader compiled successfully");
  }

  return success;
}

void initialise_font_rendering(void) {
  /* glEnable(GL_CULL_FACE); */
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /* Compiling shaders for font rendering */
  compile_shader(GL_VERTEX_SHADER, glyph_vertex_shader_program,
                 &app.font.vertex_shader);
  compile_shader(GL_FRAGMENT_SHADER, glyph_fragment_shader_program,
                 &app.font.fragment_shader);

  app.font.program = glCreateProgram();

  glAttachShader(app.font.program, app.font.vertex_shader);
  glAttachShader(app.font.program, app.font.fragment_shader);
  glLinkProgram(app.font.program);

  glDeleteShader(app.font.vertex_shader);
  glDeleteShader(app.font.fragment_shader);

  glUseProgram(app.font.program);

  /* Setting up the orthographic projection matrix */
  orthographic(app.font.projection,
               0, app.window.width,
               0, app.window.height,
               0.0, 1.0);

  /* Send projection matrix */
  glUniformMatrix4fv(glGetUniformLocation(app.font.program, "projection"),
                     1, GL_FALSE, (const float *) app.font.projection);

  glUseProgram(0);

  /* Creating and binding a VBO and VAO */
  glGenVertexArrays(1, &app.font.vao);
  glGenBuffers(1, &app.font.vbo);
  glBindVertexArray(app.font.vao);
  glBindBuffer(GL_ARRAY_BUFFER, app.font.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

int main(void) {
  bool failure = false;

  GLFWwindow *window;

  const char *font_path = "Inter-Regular.ttf";
  const int font_height_pt = 20;  /* in points */


  initialise_OpenGL();
  window = initialise_window(600, 400,
                             "Window");

  if (!window) {
    failure = true;
    goto terminate;
  }

  if (!load_font_atlas(font_path, font_height_pt)) {
    failure = true;
    goto terminate_window;
  }

  initialise_font_rendering();

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.5, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    display_text("Text!", (float2){100, 100}, 2.0, (float3) {1.0, 1.0, 1.0});

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

 terminate_window:
  glfwTerminate();

 terminate:
  if (failure)
    failure("Program exits with failure");
  else
    info("Program exits fine");

  return failure ? EXIT_FAILURE : EXIT_SUCCESS;
}
