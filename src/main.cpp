#include "sdl_starter.h"
#include "sdl_assets_loader.h"
#include <vector>
#include <string>
#include <map>

using std::vector;
using std::map;

SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_GameController *controller = nullptr;

Mix_Chunk *pauseSound = nullptr;
Mix_Music *music = nullptr;

TTF_Font *font = nullptr;

SDL_Texture *pauseTexture = nullptr;
SDL_Rect pauseBounds;

const int TOTAL_ROWS = 18;
const int TOTAL_COLUMNS = 10;
const int CELL_SIZE = 30;

int grid[TOTAL_ROWS][TOTAL_COLUMNS];

const int POSITION_OFFSET = 4;
const int CELL_OFFSET = 2;

bool isGamePaused;
bool isGameOver;

int score;

SDL_Texture *scoreTextTexture = nullptr;
SDL_Rect scoreTextBounds;

SDL_Texture *scoreTexture = nullptr;
SDL_Rect scoreBounds;

SDL_Texture *nextTexture = nullptr;
SDL_Rect nextBounds;

Mix_Chunk *rotateSound = nullptr;
Mix_Chunk *clearRowSound = nullptr;

typedef struct Vector2
{
    float x; 
    float y; 
} Vector2;

typedef struct
{
    int id;
    map<int, vector<Vector2>> cells;
    int rotationState;
    int columnOffset;
    int rowOffset;
} Block;

Block lBlock;
Block jBlock;
Block iBlock;
Block oBlock;
Block sBlock;
Block tBlock;
Block zBlock;
Block currentBlock;
Block nextBlock;

vector<Block> blocks;

vector<Vector2> getCellPositions(Block &block)
{
    // getting the reference of the vector instead of copying to create a new one.
    vector<Vector2> &blockTiles = block.cells[block.rotationState];

    vector<Vector2> movedTiles;
    movedTiles.reserve(blockTiles.size());

    for (Vector2 blockTile : blockTiles)
    {
        Vector2 newPosition = {blockTile.x + block.rowOffset, blockTile.y + block.columnOffset};
        movedTiles.push_back(newPosition);
    }

    return movedTiles;
}

bool isCellOutside(int cellRow, int cellColumn)
{
    if (cellRow >= 0 && cellRow < TOTAL_ROWS && cellColumn >= 0 && cellColumn < TOTAL_COLUMNS)
    {
        return false;
    }

    return true;
}

bool isBlockOutside(Block &block)
{
    vector<Vector2> blockTiles = getCellPositions(block);

    for (Vector2 blockTile : blockTiles)
    {
        if (isCellOutside(blockTile.x, blockTile.y))
        {
            return true;
        }
    }

    return false;
}

void undoRotation(Block &block)
{
    block.rotationState--;

    if (block.rotationState == -1)
    {
        block.rotationState = block.cells.size() - 1;
    }
}

bool isCellEmpty(int rowToCheck, int columnToCheck)
{
    if (grid[rowToCheck][columnToCheck] == 0)
    {
        return true;
    }

    return false;
}

bool blockFits(Block &block)
{
    auto blockCells = getCellPositions(block);

    // I need to write in the grid the id of the block that I'm going to lock
    for (Vector2 blockCell : blockCells)
    {
        if (!isCellEmpty(blockCell.x, blockCell.y))
        {
            return false;
        }
    }

    return true;
}

void rotateBlock(Block &block)
{
    block.rotationState++;

    if (block.rotationState == (int)block.cells.size())
    {
        block.rotationState = 0;
    }

    if (isBlockOutside(block) || !blockFits(currentBlock))
    {
        undoRotation(block);
    }
}

void moveBlock(Block &block, int rowsToMove, int columnsToMove)
{
    block.rowOffset += rowsToMove;
    block.columnOffset += columnsToMove;
}

int rand_range(int min, int max)
{
    return min + rand() / (RAND_MAX / (max - min + 1) + 1);
}

Block getRandomBlock()
{
    if (blocks.empty())
    {
        blocks = {lBlock, jBlock, iBlock, oBlock, sBlock, tBlock, zBlock};
    }

    int randomIndex = rand_range(0, blocks.size() - 1);

    Block actualBlock = blocks[randomIndex];
    blocks.erase(blocks.begin() + randomIndex);

    return actualBlock;
}

bool isRowFull(int rowToCheck)
{
    for (int column = 0; column < TOTAL_COLUMNS; column++)
    {
        if (grid[rowToCheck][column] == 0)
        {
            return false;
        }
    }

    return true;
}

void clearRow(int rowToClear)
{
    for (int column = 0; column < TOTAL_COLUMNS; column++)
    {
        grid[rowToClear][column] = 0;
    }
}

void moveRowDown(int row, int totalRows)
{
    for (int column = 0; column < TOTAL_COLUMNS; column++)
    {
        grid[row + totalRows][column] = grid[row][column];
        grid[row][column] = 0;
    }
}

int clearFullRow()
{
    int completedRow = 0;
    for (int row = TOTAL_ROWS - 1; row >= 0; row--)
    {
        if (isRowFull(row))
        {
            clearRow(row);
            completedRow++;
            Mix_PlayChannel(-1, clearRowSound, 0);
        }
        else if (completedRow > 0)
        {
            moveRowDown(row, completedRow);
        }
    }

    return completedRow;
}

void lockBlock(Block &block)
{
    auto blockCells = getCellPositions(block);

    // I need to write in the grid the id of the block that I'm going to lock
    for (Vector2 blockCell : blockCells)
    {
        grid[(int)blockCell.x][(int)blockCell.y] = block.id;
    }

    // and then update the current and next blocks.
    block = nextBlock;

    if (!blockFits(block))
    {
        isGameOver = true;
    }

    nextBlock = getRandomBlock();

    int totalClearRows = clearFullRow();

    if (totalClearRows == 1)
    {
        score += 100;
    }

    else if (totalClearRows == 2)
    {
        score += 300;
    }

    else if (totalClearRows > 2)
    {
        score += 500;
    }
}

void initializeGrid()
{
    for (int row = 0; row < TOTAL_ROWS; row++)
    {
        for (int column = 0; column < TOTAL_COLUMNS; column++)
        {
            grid[row][column] = 0;
        }
    }
}

double lastUpdateTime = 0;

bool eventTriggered(float deltaTime, float intervalUpdate)
{
    lastUpdateTime += deltaTime;

    if (lastUpdateTime >= intervalUpdate)
    {
        lastUpdateTime = 0;

        return true;
    }

    return false;
}

void handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
        {
            exit(0);
        }

        if (isGameOver && (event.type == SDL_KEYDOWN || event.type == SDL_CONTROLLERBUTTONDOWN))
        {
            initializeGrid();
            isGameOver = false;
            score = 0;
            currentBlock = getRandomBlock();
            nextBlock = getRandomBlock();
        }

        // To handle key pressed more precise, I use this method for handling pause the game or jumping.
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, pauseSound, 0);
        }

        if (!isGameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_w)
        {
            rotateBlock(currentBlock);
            Mix_PlayChannel(-1, rotateSound, 0);
        }

        if (!isGameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_d)
        {
            moveBlock(currentBlock, 0, 1);

            if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
            {
                moveBlock(currentBlock, 0, -1);
            }
        }

        else if (!isGameOver && event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_a)
        {
            moveBlock(currentBlock, 0, -1);

            if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
            {
                moveBlock(currentBlock, 0, 1);
            }
        }

        // controller support
        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_START)
        {
            isGamePaused = !isGamePaused;
            Mix_PlayChannel(-1, pauseSound, 0);
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
        {
            rotateBlock(currentBlock);
            Mix_PlayChannel(-1, rotateSound, 0);
        }

        if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
        {
            moveBlock(currentBlock, 0, 1);

            if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
            {
                moveBlock(currentBlock, 0, -1);
            }
        }

        else if (event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
        {
            moveBlock(currentBlock, 0, -1);

            if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
            {
                moveBlock(currentBlock, 0, 1);
            }
        }
    }
}

void update(float deltaTime)
{
    const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

    if (!isGameOver && currentKeyStates[SDL_SCANCODE_S])
    {
        score++;
        moveBlock(currentBlock, 1, 0);

        if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
        {
            moveBlock(currentBlock, -1, 0);
            lockBlock(currentBlock);
        }
    }

    if (!isGameOver && SDL_GameControllerGetButton(controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN))
    {
        score++;
        moveBlock(currentBlock, 1, 0);

        if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
        {
            moveBlock(currentBlock, -1, 0);
            lockBlock(currentBlock);
        }
    }

    if (!isGameOver && eventTriggered(deltaTime, 0.5))
    {
        moveBlock(currentBlock, 1, 0);

        if (isBlockOutside(currentBlock) || !blockFits(currentBlock))
        {
            moveBlock(currentBlock, -1, 0);
            lockBlock(currentBlock);
        }
    }
}

void initializeBlocks()
{
    // defining Blocks 4 rotations with a map id and vector2 2
    lBlock.id = 1;
    lBlock.cells[0] = {{0, 2}, {1, 0}, {1, 1}, {1, 2}};
    lBlock.cells[1] = {{0, 1}, {1, 1}, {2, 1}, {2, 2}};
    lBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 0}};
    lBlock.cells[3] = {{0, 0}, {0, 1}, {1, 1}, {2, 1}};
    // for all the block to start in the midle of the grid, I need to move to the (0, 3)
    moveBlock(lBlock, 0, 3);

    jBlock.id = 2;
    jBlock.cells[0] = {{0, 0}, {1, 0}, {1, 1}, {1, 2}};
    jBlock.cells[1] = {{0, 1}, {0, 2}, {1, 1}, {2, 1}};
    jBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 2}};
    jBlock.cells[3] = {{0, 1}, {1, 1}, {2, 0}, {2, 1}};

    moveBlock(jBlock, 0, 3);

    iBlock.id = 3;
    iBlock.cells[0] = {{1, 0}, {1, 1}, {1, 2}, {1, 3}};
    iBlock.cells[1] = {{0, 2}, {1, 2}, {2, 2}, {3, 2}};
    iBlock.cells[2] = {{2, 0}, {2, 1}, {2, 2}, {2, 3}};
    iBlock.cells[3] = {{0, 1}, {1, 1}, {2, 1}, {3, 1}};

    moveBlock(iBlock, -1, 3);

    // I don't need rotaion with this block
    oBlock.id = 4;
    oBlock.cells[0] = {{0, 0}, {0, 1}, {1, 0}, {1, 1}};

    moveBlock(oBlock, 0, 4);

    sBlock.id = 5;
    sBlock.cells[0] = {{0, 1}, {0, 2}, {1, 0}, {1, 1}};
    sBlock.cells[1] = {{0, 1}, {1, 1}, {1, 2}, {2, 2}};
    sBlock.cells[2] = {{1, 1}, {1, 2}, {2, 0}, {2, 1}};
    sBlock.cells[3] = {{0, 0}, {1, 0}, {1, 1}, {2, 1}};

    moveBlock(sBlock, 0, 3);

    tBlock.id = 6;
    tBlock.cells[0] = {{0, 1}, {1, 0}, {1, 1}, {1, 2}};
    tBlock.cells[1] = {{0, 1}, {1, 1}, {1, 2}, {2, 1}};
    tBlock.cells[2] = {{1, 0}, {1, 1}, {1, 2}, {2, 1}};
    tBlock.cells[3] = {{0, 1}, {1, 0}, {1, 1}, {2, 1}};

    moveBlock(tBlock, 0, 3);

    zBlock.id = 7;
    zBlock.cells[0] = {{0, 0}, {0, 1}, {1, 1}, {1, 2}};
    zBlock.cells[1] = {{0, 2}, {1, 1}, {1, 2}, {2, 1}};
    zBlock.cells[2] = {{1, 0}, {1, 1}, {2, 1}, {2, 2}};
    zBlock.cells[3] = {{0, 1}, {1, 0}, {1, 1}, {2, 0}};

    moveBlock(zBlock, 0, 3);

    blocks.reserve(7);
    blocks = {lBlock, jBlock, iBlock, oBlock, sBlock, tBlock, zBlock};

    currentBlock = getRandomBlock();
    nextBlock = getRandomBlock();
}

SDL_Color getColorByIndex(int index)
{
    const SDL_Color lightGrey = {80, 80, 80, 255};
    const SDL_Color green = {47, 230, 23, 255};
    const SDL_Color red = {232, 18, 18, 255};
    const SDL_Color orange = {226, 116, 17, 255};
    const SDL_Color yellow = {237, 234, 4, 255};
    const SDL_Color purple = {166, 0, 247, 255};
    const SDL_Color cyan = {21, 204, 209, 255};
    const SDL_Color blue = {13, 64, 216, 255};

    SDL_Color colors[] = {lightGrey, green, red, orange, yellow, purple, cyan, blue};

    return colors[index];
}

void drawGrid()
{
    for (int row = 0; row < TOTAL_ROWS; row++)
    {
        for (int column = 0; column < TOTAL_COLUMNS; column++)
        {
            int cellValue = grid[row][column];

            SDL_Color cellColor = getColorByIndex(cellValue);
            SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, cellColor.a);

            SDL_Rect rect = {column * CELL_SIZE + POSITION_OFFSET, row * CELL_SIZE + POSITION_OFFSET, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void drawBlock(Block &block, int offsetX, int offsetY)
{
    vector<Vector2> blockTiles = getCellPositions(block);

    for (Vector2 blockTile : blockTiles)
    {
        SDL_Color cellColor = getColorByIndex(block.id);
        SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, cellColor.a);

        SDL_Rect rect = {(int)blockTile.y * CELL_SIZE + offsetX, (int)blockTile.x * CELL_SIZE + offsetY, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void drawBlock(Block &block)
{
    vector<Vector2> blockTiles = getCellPositions(block);

    for (Vector2 blockTile : blockTiles)
    {
        SDL_Color cellColor = getColorByIndex(block.id);
        SDL_SetRenderDrawColor(renderer, cellColor.r, cellColor.g, cellColor.b, cellColor.a);

        SDL_Rect rect = {(int)blockTile.y * CELL_SIZE + POSITION_OFFSET, (int)blockTile.x * CELL_SIZE + POSITION_OFFSET, CELL_SIZE - CELL_OFFSET, CELL_SIZE - CELL_OFFSET};
        SDL_RenderFillRect(renderer, &rect);
    }
}

void render()
{
    SDL_SetRenderDrawColor(renderer, 29, 29, 27, 255);
    SDL_RenderClear(renderer);

    drawGrid();

    drawBlock(currentBlock);

    SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);

    SDL_RenderCopy(renderer, scoreTextTexture, NULL, &scoreTextBounds);

    SDL_Rect scorePlaceHolderRect = {315, 55, 170, 60};
    SDL_RenderFillRect(renderer, &scorePlaceHolderRect);

    updateTextureText(scoreTexture, std::to_string(score).c_str(), font, renderer);

    SDL_QueryTexture(scoreTexture, NULL, NULL, &scoreBounds.w, &scoreBounds.h);
    scoreBounds.x = 365;
    scoreBounds.y = 65;
    SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreBounds);

    SDL_RenderCopy(renderer, nextTexture, NULL, &nextBounds);

    SDL_Rect nextBlockPlaceHolderRect = {315, 215, 170, 180};
    SDL_RenderFillRect(renderer, &nextBlockPlaceHolderRect);

    if (nextBlock.id == 3)
    {
        drawBlock(nextBlock, 255, 290);
    }

    else if (nextBlock.id == 4)
    {
        drawBlock(nextBlock, 255, 280);
    }

    else
    {
        drawBlock(nextBlock, 275, 270);
    }

    if (isGameOver)
    {
        updateTextureText(pauseTexture, "Game Over", font, renderer);
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    if (isGamePaused)
    {
        updateTextureText(pauseTexture, "Game Pause", font, renderer);
        SDL_RenderCopy(renderer, pauseTexture, NULL, &pauseBounds);
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *args[])
{
    // SCREEN_WIDTH 10 * 30 = 300 + 200 = 500
    // SCREEN_HEIGHT 18 * 30 = 540 + 4 = 544
    // need to give a extra offset of 200 width and 20 heigt for the ui
    window = SDL_CreateWindow("My Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TOTAL_COLUMNS * CELL_SIZE + 200, TOTAL_ROWS * CELL_SIZE + 4, SDL_WINDOW_SHOWN);
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

    font = TTF_OpenFont("res/fonts/monogram.ttf", 36);

    updateTextureText(scoreTexture, "0", font, renderer);

    updateTextureText(scoreTextTexture, "Score", font, renderer);
    SDL_QueryTexture(scoreTextTexture, NULL, NULL, &scoreTextBounds.w, &scoreTextBounds.h);
    scoreTextBounds.x = 365;
    scoreTextBounds.y = 15;

    updateTextureText(nextTexture, "Next", font, renderer);
    SDL_QueryTexture(nextTexture, NULL, NULL, &nextBounds.w, &nextBounds.h);
    nextBounds.x = 370;
    nextBounds.y = 175;

    updateTextureText(pauseTexture, "Game Paused", font, renderer);
    SDL_QueryTexture(pauseTexture, NULL, NULL, &pauseBounds.w, &pauseBounds.h);
    pauseBounds.x = 330;
    pauseBounds.y = 450;

    pauseSound = loadSound("res/sounds/okay.wav");
    music = loadMusic("res/music/music.mp3");

    clearRowSound = loadSound("res/sounds/clear.mp3");
    rotateSound = loadSound("res/sounds/rotate.mp3");

    Mix_PlayMusic(music, -1);

    initializeGrid();
    initializeBlocks();

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

        // capping the game at 60
        capFrameRate(currentFrameTime);
    }

    Mix_FreeMusic(music);
    Mix_FreeChunk(pauseSound);
    SDL_DestroyTexture(pauseTexture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}