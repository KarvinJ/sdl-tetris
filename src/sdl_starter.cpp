#include "sdl_starter.h"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

int startSDL(SDL_Window *window, SDL_Renderer *renderer)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        SDL_LogCritical(1, "SDL crashed. Error: ");
        SDL_Quit();
        return 1;
    }

    if (window == nullptr)
    {
        SDL_LogCritical(1, "Failed to create window: ");
        SDL_Quit();
        return 1;
    }

    if (renderer == nullptr)
    {
        SDL_LogCritical(1, "Failed to create renderer: ");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    if (!IMG_Init(IMG_INIT_PNG))
    {
        SDL_LogCritical(1, "SDL_image crashed. Error: ");
        return 1;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        SDL_LogCritical(1, "SDL_mixer could not initialize!");
        return 1;
    }

    if (TTF_Init() == -1)
    {
        SDL_LogCritical(1, "SDL_ttf could not initialize!");
        return 1;
    }

    return 0;
}

void capFrameRate(Uint32 frameStartTime)
{
    Uint32 frameTime = SDL_GetTicks() - frameStartTime;

    if (frameTime < 1000 / FRAME_RATE)
    {
        SDL_Delay(1000 / FRAME_RATE - frameTime);
    }
}