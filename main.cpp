#include <iostream>
#include <windows.h>
#include <conio.h>
#include <string>
#include "console_ui.h"
#include <windows.h>  // for SetConsoleTextAttribute
#include <iomanip>    // for setw

extern void clearScreen();

using namespace std;

// Typing effect
void typeText(const string &text, int speed = 25) {
    for (char c : text) {
        cout << c << flush;
        Sleep(speed);
    }
}

// Clear screen
// void clearScreen() {
//     system("cls");
// }

// Center text
void centerText(const string &text) {
    int width = 80;
    int pad = (width - text.size()) / 2;
    cout << string(max(0, pad), ' ') << text << endl;
}



// Updated loading effect with single-line progress and color
void loadingEffect(string msg = "🌀 Initiating Neural Sync Protocol...") {
    clearScreen();
    cout << msg << "\n\n";

    const int totalBlocks = 25;
    string milestones[5] = {
        "⚡ Neural link established!",
        "🔍 Scanning consciousness layers...",
        "🧠 Matrix Core Online!",
        "🌌 Entering MindSpace....",
        "🚪 Portal Unlocked! 💫 Traveler, your mind is aligned!"
    };

    int consoleWidth = 120; // adjust for your console width

    for (int i = 0; i <= totalBlocks; i++) {
        int progress = (i * 100) / totalBlocks;

        // Build bar
        string bar = "[";
        for (int j = 0; j < i; j++) bar += u8"▓";
        for (int j = i; j < totalBlocks; j++) bar += u8"░";
        bar += "] " + to_string(progress) + "%";

        // Milestone message
        string milestoneMsg = "";
        if (progress >= 20 && progress < 40) milestoneMsg = milestones[0];
        else if (progress >= 40 && progress < 60) milestoneMsg = milestones[1];
        else if (progress >= 60 && progress < 80) milestoneMsg = milestones[2];
        else if (progress >= 80 && progress < 100) milestoneMsg = milestones[3];
        else if (progress == 100) milestoneMsg = milestones[4];

        // Combine bar + milestone
        string fullLine = bar;
        if (!milestoneMsg.empty()) fullLine += "  " + milestoneMsg;

        // Pad with spaces to overwrite previous line completely
        if (fullLine.size() < consoleWidth) {
            fullLine += string(consoleWidth - fullLine.size(), ' ');
        }

        cout << "\r" << fullLine;
        cout.flush();
        Sleep(120);
    }

    cout << "\n\nPress ENTER to enter the MindMatrix Realm...";
    cin.ignore();
}





// Gate display
// void showGateDescriptions() {
//     clearScreen();
//     typeText("\n\n              ⚡ Welcome to the MindMatrix Realm ⚡\n\n", 40);
//     Sleep(400);

//     typeText("Each Gate tests a dimension of your mind...\n\n", 35);
//     Sleep(400);

//     cout << "═══════════════════════════════════════════════════════════════════\n\n";

//     typeText("🌀  Gate I — The Corridor of Logic\n", 25);
//     typeText("     A realm where code is poetry and logic is survival.\n", 20);
//     typeText("     Every choice echoes through recursion and reasoning.\n\n", 20);

//     typeText("🔥  Gate II — The Chamber of Patterns\n", 25);
//     typeText("     Hidden beneath its rhythm lies the pulse of algorithms.\n", 20);
//     typeText("     Sorting, searching, mapping — the melody of structure.\n\n", 20);

//     typeText("🌙  Gate III — The Mirror of Intuition\n", 25);
//     typeText("     It reflects your instincts — can your mind see beyond numbers?\n", 20);
//     typeText("     IQ and common sense entwined in illusions of logic.\n\n", 20);

//     typeText("⚔️  Gate IV — The Arena of Complexity\n", 25);
//     typeText("     Every move a function, every strike a data flow.\n", 20);
//     typeText("     Conquer time itself — optimize or perish.\n\n", 20);

//     cout << "═══════════════════════════════════════════════════════════════════\n\n";

//     typeText("🌌  Or, do you wish to retreat...\n", 35);
//     typeText("     Return to Reality without entering the Matrix of your Mind?\n\n", 35);

//     typeText("Choose wisely, wanderer.\n\n", 40);
// }

// Menu input
char gateMenu() {
    cout << "\nEnter your choice (1-4 for gates, 0 to quit): ";
    char choice = _getch();

    switch (choice) {
        case '1':
            clearScreen();
            typeText("🌀 You step into the Corridor of Logic...\n", 30);
            Sleep(800);
            break;
        case '2':
            clearScreen();
            typeText("🔥 You enter the Chamber of Patterns...\n", 30);
            Sleep(800);
            break;
        case '3':
            clearScreen();
            typeText("🌙 You gaze into the Mirror of Intuition...\n", 30);
            Sleep(800);
            break;
        case '4':
            clearScreen();
            typeText("⚔️ The Arena of Complexity awaits your courage...\n", 30);
            Sleep(800);
            break;
        case '0':
            clearScreen();
            typeText("🌌 You turn away from the Mind Matrix... returning to reality.\n", 30);
            Sleep(800);
            return '0'; // immediately return
        default:
            clearScreen();
            typeText("Invalid choice. The Matrix shifts away from your grasp...\n", 30);
            Sleep(800);
            return 'x'; // invalid choice
    }
    return choice;
}






int main() {
    clearScreen();
    centerText("🧠 Welcome, Wanderer of Thought 🧠\n");
    typeText("\nPress ENTER to access the Mind Matrix...\n", 30);
    cin.ignore();

    loadingEffect(); // dynamic loading bar

    // showGateDescriptions();
    char choice = gateMenu();  // get user input

    if (choice >= '1' && choice <= '4') {
        // typeText("\n\nThank you for exploring the MindMatrix Realm...\n", 25);
        // typeText("Your journey has just begun.\n\n", 25);
    }
}
