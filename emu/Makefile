app: app.c watch.h watch.c
	g++ -o app -fpermissive app.c watch.c -l SDL2 -lSDL2_image

emu: emu.cpp
	g++ emu.cpp -o emu -I /usr/include/SDL2/ -l SDL2 -lSDL2_image
