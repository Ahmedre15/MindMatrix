#include <iostream>
#include <chrono>
#include <thread>
using namespace std;

void runMindMatrix() {
    system("cls"); // Clear screen
    cout << "\n\n\n";
    cout << "🌀 Initiating Neural Sync Protocol...\n\n";

    // Simulate loading bar
    const int total = 25;
    for (int i = 0; i <= total; i++) {
        cout << "\r[";
        for (int j = 0; j < total; j++) {
            if (j < i) cout << "▓";
            else cout << "░";
        }
        cout << "] " << (i * 4) << "%";
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(100)); // speed of loading
    }

    // Transition after loading
    this_thread::sleep_for(chrono::milliseconds(500));
    cout << "\n\n⚡ System handshake established.";
    this_thread::sleep_for(chrono::milliseconds(600));
    cout << "\n🔍 Scanning consciousness layers...";
    this_thread::sleep_for(chrono::milliseconds(600));
    cout << "\n🧠 Matrix Core Online!";
    this_thread::sleep_for(chrono::milliseconds(600));
    cout << "\n🌌 Entering MindSpace....";
    this_thread::sleep_for(chrono::milliseconds(800));
    cout << "\n🚪 Portal Unlocked!";
    this_thread::sleep_for(chrono::milliseconds(1000));

    cout << "\n\n💫 Welcome, Traveler — your mind frequency is now aligned with the Matrix.";
    cout << "\n\nPress ENTER to enter the MindMatrix Realm...";
    cin.ignore();
    cin.get();
}

