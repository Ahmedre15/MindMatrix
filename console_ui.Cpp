// console_ui.cpp
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Print a divider line
void printDivider() {
    cout << "═══════════════════════════════════════════" << endl;
}

// Print title with padding
void printTitle(const string &title) {
    cout << "✨ " << title << " ✨" << endl << endl;
}

// Simulate loading animation
void loadingAnimation(const string &text, int seconds) {
    cout << text;
    cout.flush();
    for (int i = 0; i < seconds * 3; ++i) {
        this_thread::sleep_for(chrono::milliseconds(333));
        cout << ".";
        cout.flush();
    }
    cout << endl << endl;
}



// Pause and wait for user input
void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore();
}
