#include "console_ui.h"
#include <iostream>
#include <windows.h>
using namespace std;

void clearScreen() {
    system("cls");
}

void printTitle(const string& title) {
    setColor(11); // light cyan
    cout << "\n==================== " << title << " ====================\n";
    setColor(7);
}

void printDivider() {
    cout << "------------------------------------------------------------\n";
}

void setColor(int colorCode) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, colorCode);
}

void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore();
    cin.get();
}
