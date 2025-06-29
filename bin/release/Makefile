default:
	g++ -c ../../src/*.cpp -std=c++14 -O3 -m64 -I ../../include
	g++ *.o -o ../../bin/release/main -s -L ../../lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
	./main.exe