
SHADER_FILES = $(wildcard shaders/*.glsl)
SHADER_INC_FILES = $(SHADER_FILES:shaders/%.glsl=shaders/%.glsl.inc)

# Format shader source files to be included with the C preprocessor
shaders/%.glsl.inc: shaders/%.glsl
	./shaders/process_shader.py $<

main.exe: main.c $(SHADER_INC_FILES)
	cc main.c -lglfw -framework OpenGL -o main.exe
