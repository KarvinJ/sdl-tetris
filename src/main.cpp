#include "sdl_starter.h"
#include "sdl_assets_loader.h"
#include <time.h>
#include <string>

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_GameController *controller = nullptr;

Mix_Chunk *actionSound = nullptr;
Mix_Music *music = nullptr;

Sprite playerSprite;

const int PLAYER_SPEED = 600;

bool isGamePaused;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

int score;

TTF_Font *fontSquare = nullptr;

SDL_Rect ball = {SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 2, 32, 32};

int ballVelocityX = 400;
int ballVelocityY = 400;

int colorIndex;

SDL_Color colors[] = {
    {128, 128, 128, 0}, // gray
    {255, 255, 255, 0}, // white
    {255, 0, 0, 0},     // red
    {0, 255, 0, 0},     // green
    {0, 0, 255, 0},     // blue
    {255, 255, 0, 0},   // brown
    {0, 255, 255, 0},   // cyan
    {255, 0, 255, 0},   // purple
};

void quitGame()
{
    Mix_FreeMusic(music);
    Mix_FreeChunk(actionSound);
    SDL_DestroyTexture(playerSprite.texture);
    SDL_DestroyTexture(pauseTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
        {
            quitGame();
            exit(0);
        }

        // To handle key pressed more precise, I use this method for handling pause the game or jumping.
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, actionSound, 0);
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, actionSound, 0);
        }
    }
}

int rand_range(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

void update(float deltaTime)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    // keyboard
    if (currentKeyStates[SDL_SCANCODE_W] && playerSprite.bounds.y > 0)
    {
        playerSprite.bounds.y -= PLAYER_SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_S] && playerSprite.bounds.y < SCREEN_HEIGHT - playerSprite.bounds.h)
    {
        playerSprite.bounds.y += PLAYER_SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_A] && playerSprite.bounds.x > 0)
    {
        playerSprite.bounds.x -= PLAYER_SPEED * deltaTime;
    }

    else if (currentKeyStates[SDL_SCANCODE_D] && playerSprite.bounds.x < SCREEN_WIDTH - playerSprite.bounds.w)
    {
        playerSprite.bounds.x += PLAYER_SPEED * deltaTime;
    }

    // controller
    if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_UP) && playerSprite.bounds.y > 0)
    {
        playerSprite.bounds.y -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN) && playerSprite.bounds.y < SCREEN_HEIGHT - playerSprite.bounds.h)
    {
        playerSprite.bounds.y += PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT) && playerSprite.bounds.x > 0)
    {
        playerSprite.bounds.x -= PLAYER_SPEED * deltaTime;
    }

    else if (SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT) && playerSprite.bounds.x < SCREEN_WIDTH - playerSprite.bounds.w)
    {
        playerSprite.bounds.x += PLAYER_SPEED * deltaTime;
    }

    if (ball.x < 0 || ball.x > SCREEN_WIDTH - ball.w)
    {
        ballVelocityX *= -1;
        colorIndex = rand_range(0, 5);
    }

    else if (ball.y < 0 || ball.y > SCREEN_HEIGHT - ball.h)
    {
        ballVelocityY *= -1;
        colorIndex = rand_range(0, 5);
    }

    else if (SDL_HasIntersection(&playerSprite.bounds, &ball))
    {
        ballVelocityX *= -1;
        ballVelocityY *= -1;

        colorIndex = rand_range(0, 5);

        Mix_PlayChannel(-1, actionSound, 0);

        score++;

        std::string scoreString = "score: " + std::to_string(score);
        updateTextureText(scoreTexture, scoreString.c_str(), fontSquare, renderer);
    }

    ball.x += ballVelocityX * deltaTime;
    ball.y += ballVelocityY * deltaTime;
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, colors[colorIndex].r, colors[colorIndex].g, colors[colorIndex].b, 255);

    SDL_RenderFillRect(renderer, &ball);

    renderSprite(renderer, playerSprite);

    if (isGamePaused)
    {
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = 400;
    scoreBounds.y = scoreBounds.h / 2;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (startSDL(window, renderer) > 0)
    {
        return 1;
    }

    if (SDL_NumJoysticks() < 1)
    {
        SDL_Log("No game controllers connected!");
    }
    else
    {
        controller = SDL_GameControllerOpen(0);
        if (controller == NULL)
        {
            SDL_Log("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
            return 1;
        }
    }

    // load font
    fontSquare = TTF_OpenFont("res/fonts/square_sans_serif_7.ttf", 36);

    // load title
    updateTextureText(scoreTexture, "Score: 0", fontSquare, renderer);

    updateTextureText(pauseTexture, "Game Paused", fontSquare, renderer);

    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = SCREEN_WIDTH / 2 - pauseBounds.w / 2;
    pauseBounds.y = 100;
    // After I use the &pauseBounds.w, &pauseBounds.h in the SDL_QueryTexture.
    //  I get the width and height of the actual texture

    playerSprite = loadSprite(renderer, "res/sprites/alien_1.png", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

    actionSound = loadSound("res/sounds/magic.wav");

    // method to reduce the volume of the sound in half.
    Mix_VolumeChunk(actionSound, MIX_MAX_VOLUME / 2);

    // Load music file (only one data piece, intended for streaming)
    music = loadMusic("res/music/music.wav");

    // reduce music volume in half
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2);

    // Start playing streamed music, put -1 to loop indifinitely
    Mix_PlayMusic(music, -1);

    Uint32 previousFrameTime = SDL_GetTicks();
    Uint32 currentFrameTime = previousFrameTime;
    float deltaTime = 0.0f;

    while (true)
    {
        currentFrameTime = SDL_GetTicks();
        deltaTime = (currentFrameTime - previousFrameTime) / 1000.0f;
        previousFrameTime = currentFrameTime;

        SDL_GameControllerUpdate();

        handleEvents();

        if (!isGamePaused)
        {
            update(deltaTime);
        }

        render();

        // capFrameRate(currentFrameTime);
    }
}