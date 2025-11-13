#ifndef CONSOLE_UI_H
#define CONSOLE_UI_H
#include <string>

void clearScreen();
void printDivider();
void printTitle(const std::string &title);
void loadingAnimation(const std::string &text, int seconds);
void showMindMatrixUI();
void pauseScreen();

#endif
