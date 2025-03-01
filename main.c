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


static const char *vertex_shader_program =
#include "shaders/vertex.glsl.inc"
;

static const char *fragment_shader_program =
#include "shaders/fragment.glsl.inc"
;

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
  info("Resized OpenGL viewport");
}

GLFWwindow *initialise_window(int width, int height, const char *title) {
  GLFWwindow *window = glfwCreateWindow(width, height, title, NULL, NULL);

  if (!window) {
    failure("Could not initialise GLFW window");
    glfwTerminate();
    return NULL;
  }

  glfwMakeContextCurrent(window);

  resize_window_viewport(window, width, height);
  glfwSetFramebufferSizeCallback(window, resize_window_viewport);

  info("Initialised window");
  return window;
}

int main(void) {
  GLFWwindow *window;
  unsigned int vbo, vao, vertex_shader, fragment_shader, shader_program;

  initialise_OpenGL();
  window = initialise_window(600, 400,
                             "My first GLFW+OpenGL window");

  /* Have some data to send to the GPU */
  const float vertices[] = {
    -1.0,  0.0, 0,
     0.0, -1.0, 0,
     0.0,  0.0, 0,
  };

  /* We create a vertex array object that really all it does is point
     to our vertex buffer object that stores the data we sent to the
     GPU. */
  glGenBuffers(1, &vbo);
  glGenVertexArrays(1, &vao);

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  /* Create and bind a vertex buffer to OpenGL's context */
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  /* Copy `vertices' into the GPU  */
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),
               vertices, GL_STATIC_DRAW);

  /* We specify the layout of the data we sent to the GPU (which is at
     offset 0). We have to enable it, as it is disabled by default. */
  glVertexAttribPointer(0, 3, GL_FLOAT,
                        GL_FALSE, 3 * sizeof(float), NULL);
  glEnableVertexAttribArray(0);

  /* Create vertex shader */
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);

  /* Set and compile vertex shader source code */
  glShaderSource(vertex_shader, 1, &vertex_shader_program, NULL);
  glCompileShader(vertex_shader);

  /* Create fragment shader */
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

  /* Set and compile fragment shader (same as before) */
  glShaderSource(fragment_shader, 1, &fragment_shader_program, NULL);
  glCompileShader(fragment_shader);

  /* Our two shaders get linked into a program */
  shader_program = glCreateProgram();

  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);

  /* We activate our program object for OpenGL to use. OpenGL will
     send the input data from the VAO to the shader pipeline we just
     made. */
  glUseProgram(shader_program);
  glBindVertexArray(vao);
  glEnableVertexAttribArray(0);

  /* Freeing memory */
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.5, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    /* Display result of shader program */
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteProgram(shader_program);
  glfwTerminate();

  return EXIT_SUCCESS;
}
