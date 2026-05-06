main: main.c engine.c engine.h
	gcc main.c engine.c -o "$@"