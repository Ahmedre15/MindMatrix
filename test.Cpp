#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <string>
using namespace std;

void clearScreen() {
    system("cls"); // for Windows
}

void delay(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

void typeText(const string &text, int speed = 25) {
    for (char c : text) {
        cout << c << flush;
        delay(speed);
    }
    cout << endl;
}

void matrixRain(int lines = 10, int speed = 60) {
    const string chars = "01#@$%&*";
    srand(time(0));
    for (int i = 0; i < lines; ++i) {
        for (int j = 0; j < 50; ++j)
            cout << chars[rand() % chars.size()];
        cout << endl;
        delay(speed);
    }
}

void spinningPortal(int rounds = 20, int speed = 90) {
    string frames[] = { "|", "/", "-", "\\" };
    for (int i = 0; i < rounds; ++i) {
        cout << "\r🧠 Initializing Matrix Core " << frames[i % 4] << flush;
        delay(speed);
    }
    cout << "\r🧠 Matrix Core Online! ✅          " << endl;
}

void waveDots(int count = 15, int speed = 120) {
    for (int i = 0; i < count; ++i) {
        cout << "\r🌌 Entering MindSpace";
        for (int j = 0; j <= i % 4; ++j) cout << ".";
        cout << flush;
        delay(speed);
    }
    cout << "\n🚪 Portal Unlocked!\n";
}

int main() {
    clearScreen();
    typeText("🌀 Initiating Neural Sync Protocol...");
    delay(500);
    matrixRain(12, 50);
    delay(600);
    typeText("\n⚡ System handshake established.");
    typeText("🔍 Scanning consciousness layers...");
    delay(500);
    spinningPortal();
    delay(400);
    waveDots();
    delay(400);
    typeText("\n💫 Welcome, Traveler — your mind frequency is now aligned with the Matrix.");
    delay(1000);
    typeText("\nPress ENTER to enter the MindMatrix Realm...");
    cin.get();
    clearScreen();
    typeText("🌐 Transitioning to the Realm of Gates...");
    delay(1000);
    matrixRain(15, 40);
    cout << "\n🚀 You’ve entered. Choose your gate wisely.\n";
}
