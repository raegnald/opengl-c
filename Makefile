
OS = $(shell uname)

ifeq ($(OS), Linux)
	CFLAGS += -lglfw -lGL -lX11 -lpthread -lXrandr -ldl
endif

ifeq ($(OS), Darwin)
	CFLAGS += -lglfw -framework OpenGL
endif

main.exe: main.c
	cc main.c $(CFLAGS) -o main.exe
