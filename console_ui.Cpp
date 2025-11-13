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

// Display the MindMatrix intro and gates
void showMindMatrixUI() {
    clearScreen();
    printTitle("Welcome to the MindMatrix Realm");
    cout << "Each Gate tests a dimension of your mind..." << endl << endl;
    printDivider();
    cout << "🚪 Gate I — The Corridor of Logic" << endl;
    cout << "     A realm where code is poetry and logic is survival." << endl;
    cout << "     Every choice echoes through recursion and reasoning." << endl << endl;

    cout << "🧩 Gate II — The Chamber of Patterns" << endl;
    cout << "     Hidden beneath its rhythm lies the pulse of algorithms." << endl;
    cout << "     Sorting, searching, mapping — the melody of structure." << endl << endl;

    cout << "🔮 Gate III — The Mirror of Intuition" << endl;
    cout << "     It reflects your instincts — can your mind see beyond numbers?" << endl;
    cout << "     IQ and common sense entwined in illusions of logic." << endl << endl;

    cout << "⚔️ Gate IV — The Arena of Complexity" << endl;
    cout << "     Where chaos meets control. Only the focused survive." << endl << endl;
    printDivider();
    cout << "Will you step beyond and enter the Matrix of your Mind? (Press Enter)" << endl;
    cin.ignore();
}

// Pause and wait for user input
void pauseScreen() {
    cout << "\nPress Enter to continue...";
    cin.ignore();
}
