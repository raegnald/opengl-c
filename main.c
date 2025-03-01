#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "log.h"

#if defined __APPLE__
#  define GL_SILENCE_DEPRECATION
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <GLFW/glfw3.h>



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

  initialise_OpenGL();
  window = initialise_window(600, 400,
                             "My first GLFW+OpenGL window");

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.5, 0.4, 0.6, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glfwTerminate();
  return EXIT_SUCCESS;
}
