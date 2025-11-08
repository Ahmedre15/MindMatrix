#include <iostream>
#include "console_ui.h"
#include "pattern_escape.h"
#include "mindmatrix.h"
using namespace std;

int main() {
    int choice;
    do {
        clearScreen();
        printTitle("MindX Matrix");
        cout << "1. Play Pattern Escape\n";
        cout << "2. Try Mind Matrix IQ Test\n";
        cout << "3. Exit\n";
        printDivider();
        cout << "Enter choice: ";
        cin >> choice;

        switch (choice) {
        case 1:
            runPatternEscape();
            break;
        case 2:
            runMindMatrix();
            break;
        case 3:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Invalid choice!\n";
        }
    } while (choice != 3);

    return 0;
}
