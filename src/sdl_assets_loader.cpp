#include "sdl_assets_loader.h"

Sprite loadSprite(SDL_Renderer *renderer, const char *filePath, int positionX, int positionY)
{
    SDL_Rect bounds = {positionX, positionY, 0, 0};

    SDL_Texture *texture = IMG_LoadTexture(renderer, filePath);

    if (texture != nullptr)
    {
        SDL_QueryTexture(texture, NULL, NULL, &bounds.w, &bounds.h);
    }

    Sprite sprite = {texture, bounds};

    return sprite;
}

void renderSprite(SDL_Renderer *renderer, Sprite &sprite)
{
    SDL_RenderCopy(renderer, sprite.texture, NULL, &sprite.bounds);
}

Mix_Chunk *loadSound(const char *filePath)
{
    Mix_Chunk *sound = nullptr;

    sound = Mix_LoadWAV(filePath);
    if (sound == nullptr)
    {
        SDL_Log("Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return sound;
}

Mix_Music *loadMusic(const char *filePath)
{
    Mix_Music *music = nullptr;

    music = Mix_LoadMUS(filePath);
    if (music == nullptr)
    {
        SDL_Log("Failed to load music! SDL_mixer Error: %s\n", Mix_GetError());
    }

    return music;
}

void updateTextureText(SDL_Texture *&texture, const char *text, TTF_Font *&fontSquare, SDL_Renderer *renderer)
{
    SDL_Color fontColor = {255, 255, 255};

    if (fontSquare == nullptr)
    {
        SDL_Log("TTF_OpenFont fontSquare: %s\n", TTF_GetError());
    }

    SDL_Surface *surface = TTF_RenderUTF8_Blended(fontSquare, text, fontColor);
    if (surface == nullptr)
    {
        SDL_Log("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create text surface! SDL Error: %s\n", SDL_GetError());
        exit(3);
    }

    SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == nullptr)
    {
        SDL_Log("TTF_OpenFont: %s\n", TTF_GetError());
        SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unable to create texture from surface! SDL Error: %s\n", SDL_GetError());
    }

    SDL_FreeSurface(surface);
}