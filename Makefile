
OS = $(shell uname)

ifeq ($(OS), Linux)
	CFLAGS += -lglfw -lGL -lX11 -lpthread -lXrandr -ldl
endif

ifeq ($(OS), Darwin)
	CFLAGS += -lglfw -framework OpenGL
endif

SHADER_FILES = $(wildcard shaders/*.glsl)
SHADER_INC_FILES = $(SHADER_FILES:shaders/%.glsl=shaders/%.glsl.inc)

# Format shader source files to be included with the C preprocessor
shaders/%.glsl.inc: shaders/%.glsl
	./shaders/process_shader.py $<

main.exe: main.c $(SHADER_INC_FILES)
	cc main.c $(CFLAGS) -o main.exe
