emu.wasm emu.js: Makefile emu.c app.c watch.h watch.c
	emcc -s ASYNCIFY=1 -O3 -s EXPORTED_FUNCTIONS='["_incoming_event", "_main"]' -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' -o emu.html emu.c app.c watch.c
