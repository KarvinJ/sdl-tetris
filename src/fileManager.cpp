#include "fileManager.h"

void saveScore(int score)
{
    std::ofstream highScoreFile("high-score.txt");

    highScoreFile << std::to_string(score);

    highScoreFile.close();
}

void saveHighScore(int &score, int &highScore)
{
    if (score > highScore)
    {
        highScore = score;
        saveScore(score);
        score = 0;
    }
}

int loadHighScore()
{
    std::string highScoreText;

    std::ifstream highScoreFile("high-score.txt");

    if (!highScoreFile.is_open())
    {
        saveScore(0);
        return 0;
    }

    getline(highScoreFile, highScoreText);

    highScoreFile.close();

    int highScore = stoi(highScoreText);

    return highScore;
}