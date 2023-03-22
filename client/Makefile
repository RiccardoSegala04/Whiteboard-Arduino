wb: main.o whiteboard.o sdl2_lib.o serial.o
	cc main.o whiteboard.o serial.o sdl2_lib.o -lSDL2 -lm -o wb

whiteboard.o: src/whiteboard.c include/whiteboard.h
	cc -c src/whiteboard.c -Iinclude -o whiteboard.o

serial.o: src/serial.c include/serial.h
	cc -c src/serial.c -Iinclude -o serial.o

sdl2_lib.o: src/SDL2_gfxPrimitives.c include/SDL2_gfxPrimitives.h
	cc -c src/SDL2_gfxPrimitives.c -Iinclude -o sdl2_lib.o

main.o: src/main.c
	cc -c src/main.c -Iinclude -o main.o



clean:
	rm *.o


