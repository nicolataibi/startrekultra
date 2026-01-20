CC = gcc
CFLAGS = -Wall -Iinclude -lm -std=c23
GL_LIBS = -lglut -lGLU -lGL

all: startrekultra ultra_view

startrekultra: src/main.c
	$(CC) src/main.c -o startrekultra $(CFLAGS)

ultra_view: src/ultra_view.c
	$(CC) src/ultra_view.c -o ultra_view $(CFLAGS) $(GL_LIBS)

clean:
	rm -f startrekultra ultra_view
