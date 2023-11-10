CFLAGS+= -O2 -Wall -lm
CC= gcc

engine.o: engine.c engine.h
engine-ocl.o: engine-ocl.c engine-ocl.h engine.h
sdl2: CFLAGS+= -lSDL2 -lpthread -lOpenCL -m64 -std=c99
sdl2: engine-ocl.o sdl2.c
# sdl2: CFLAGS+= -lSDL2 -lpthread -lm
# sdl2: engine.o sdl2.c
ov: engine.o ovrun.c
