#pragma once

#include <SDL2/SDL.h>

const int SCREEN_WIDTH = 960;
const int SCREEN_HEIGHT = 544;
const int FRAME_RATE = 60;

int startSDL(SDL_Window *window, SDL_Renderer *renderer);

void capFrameRate(Uint32 frameStartTime);