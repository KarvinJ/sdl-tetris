#pragma once

#include <iostream>
#include <fstream>

void saveScore(int score);

void saveHighScore(int &score, int &highScore);

int loadHighScore();