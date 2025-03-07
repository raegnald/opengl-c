
OS = $(shell uname)

# OpenGL, GLFW
ifeq ($(OS), Linux)
	LDFLAGS += -lglfw -lGL -lX11 -lpthread -lXrandr -ldl
else ifeq ($(OS), Darwin)
	LDFLAGS += -lglfw -framework OpenGL
endif

# Free Type
CFLAGS += $(shell pkg-config --cflags freetype2)
LDFLAGS += $(shell pkg-config --libs freetype2)

SHADER_FILES = $(wildcard shaders/*.glsl)
SHADER_INC_FILES = $(SHADER_FILES:shaders/%.glsl=shaders/%.glsl.inc)

main.exe: main.o
	cc main.o $(LDFLAGS) -o main.exe

# Format shader source files to be included with the C preprocessor
shaders/%.glsl.inc: shaders/%.glsl
	./shaders/process_shader.py $<

main.o: main.c $(SHADER_INC_FILES)
	cc -c main.c $(CFLAGS) -o main.o
