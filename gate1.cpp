// Gate1.cpp
// Full implementation of Gate 1: Loop of Echoes
// Requires Gate1.h (declare void runGate1();)
// Compile with: g++ main.cpp Gate1.cpp console_ui.cpp pattern_escape.cpp mindmatrix.cpp -o MindMatrix.exe -fexec-charset=UTF-8 -finput-charset=UTF-8 -lwinmm

#include "Gate1.h"

#include <functional>

#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <chrono>
#include <thread>
#include <algorithm>
#include <conio.h>
#include <windows.h>
#include <mutex>

using namespace std;

/* -------------------- Helpers -------------------- */

static void sleepMs(int ms) { this_thread::sleep_for(chrono::milliseconds(ms)); }

static void localType(const string &s, int ms = 20) {
    for (char c : s) { cout << c << flush; sleepMs(ms); }
}

static void localCenter(const string &s, int width = 78) {
    int pad = max(0, (width - (int)s.size()) / 2);
    cout << string(pad, ' ') << s << endl;
}

static string readLineTrim() {
    string s;
    getline(cin, s);
    // trim both ends
    auto l = s.find_first_not_of(" \t\r\n");
    if (l == string::npos) return "";
    auto r = s.find_last_not_of(" \t\r\n");
    return s.substr(l, r - l + 1);
}

static int readIntFromUser() {
    string line = readLineTrim();
    try { return stoi(line); } catch(...) { return INT_MIN; }
}

static void setColor(int c) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

// simple beep (Windows). freq in Hz, dur in ms
static void playBeep(int freq = 700, int dur = 140) {
    // Best-effort: Beep() works in most Windows terminals.
    // If you prefer PlaySound with WAV file, see commented suggestion below.
    Beep(freq, dur);
}

/* If you want WAV playback instead of Beep(), uncomment and use PlaySound:
   #include <mmsystem.h>
   PlaySound(TEXT("click.wav"), NULL, SND_FILENAME | SND_ASYNC);
   Compile with -lwinmm
*/

/* -------------------- Sequence & MCQ structures -------------------- */

struct Variant {
    string promptText;        // textual representation to show (sequence or code)
    int correctIntAnswer;     // the integer correct answer (for stage-specific meaning)
    string hintCode;          // small hint snippet
};

struct MCQ {
    string q;
    vector<string> opts;
    int correct; // 1-based index
};

/* -------------------- Random helpers & generators -------------------- */

static bool isPrime(int x) {
    if (x <= 1) return false;
    for (int i=2;i*i<=x;i++) if (x % i == 0) return false;
    return true;
}

static vector<int> genPrimes(int len) {
    vector<int> out; int n = 2;
    while ((int)out.size() < len) {
        if (isPrime(n)) out.push_back(n);
        ++n;
    }
    return out;
}

static vector<int> genFibo(int len) {
    vector<int> f;
    if (len >= 1) f.push_back(1);
    if (len >= 2) f.push_back(1);
    while ((int)f.size() < len) f.push_back(f[f.size()-1] + f[f.size()-2]);
    return f;
}

static vector<int> genAP(int len) {
    int a = 2 + (rand() % 5);
    int d = 2 + (rand() % 4);
    vector<int> out;
    for (int i=0;i<len;i++) out.push_back(a + i*d);
    return out;
}

/* -------------------- Safe recursion/password helper -------------------- */
// produce concatenation of n down to 1 (max length = n) ; n restricted to <=6 by design
static string passPrint(int n) {
    string out;
    for (int i = n; i >= 1; --i) out += to_string(i);
    return out;
}

/* -------------------- final small function for final lock -------------------- */
static long long finalFunc(int x) {
    // safe factorial for small x
    long long r = 1;
    for (int i=2;i<=x;i++) r *= i;
    return r;
}

/* -------------------- Data: per-stage variants and RT pools -------------------- */

static vector<Variant> variantsPerStage[6]; // 1..5
static vector<MCQ> rtPoolEasy[6];
static vector<MCQ> rtPoolMedium[6];

// used trackers (to avoid reusing same RT immediately)
static vector<int> usedRTIndex[6];
static vector<bool> variantUsedFlag[6]; // flags for whether a variant index was used for a stage

/* -------------------- Utility: pick unused variant (if exhausted, reset flags) -------------------- */

static int pickUnusedVariantIndex(int stage) {
    int poolSize = (int)variantsPerStage[stage].size();
    if (poolSize == 0) return -1;
    vector<int> candidates;
    for (int i=0;i<poolSize;i++) if (!variantUsedFlag[stage][i]) candidates.push_back(i);
    if (candidates.empty()) {
        // reset flags to allow reusing variants if all exhausted (so RT can loop)
        for (int i=0;i<poolSize;i++) variantUsedFlag[stage][i] = false;
        for (int i=0;i<poolSize;i++) candidates.push_back(i);
    }
    int pick = candidates[rand() % candidates.size()];
    variantUsedFlag[stage][pick] = true;
    return pick;
}

/* -------------------- Utility: pick unused RT index -------------------- */

static int pickUnusedRTIndex(int stage, bool useMedium) {
    vector<MCQ> &pool = useMedium ? rtPoolMedium[stage] : rtPoolEasy[stage];
    int poolSize = (int)pool.size();
    if (poolSize == 0) return -1;
    vector<int> cand;
    for (int i=0;i<poolSize;i++) {
        bool used = false;
        for (int u : usedRTIndex[stage]) if (u == i) { used = true; break; }
        if (!used) cand.push_back(i);
    }
    if (cand.empty()) {
        // reset used list if exhausted
        usedRTIndex[stage].clear();
        for (int i=0;i<poolSize;i++) cand.push_back(i);
    }
    int pick = cand[rand() % cand.size()];
    usedRTIndex[stage].push_back(pick);
    return pick;
}

/* -------------------- Present helpers -------------------- */

static void title(const string &t) {
    system("cls");
    setColor(11);
    localCenter("==================================================================", 80);
    localCenter("  " + t, 80);
    localCenter("==================================================================", 80);
    setColor(15);
    cout << endl;
}

/* -------------------- MCQ ask -------------------- */

static bool askMCQ(const MCQ &m) {
    localType("\n" + m.q + "\n", 15);
    for (int i=0;i<(int)m.opts.size(); ++i) {
        cout << "  " << (i+1) << ") " << m.opts[i] << "\n";
    }
    cout << "\nAnswer (1-" << m.opts.size() << "): ";
    int a = readIntFromUser();
    if (a == m.correct) { localType("\nCorrect.\n", 15); return true; }
    else { localType("\nNot correct.\n", 15); return false; }
}

/* -------------------- Stage-specific tryMain and tryRT implementations -------------------- */

// parameters array A1..A5 (index 1..5)
static int paramsA[6] = {0,0,0,0,0,0};

// Try MAIN variant for a given stage. Returns true if solved (and set paramsA[stage]), false otherwise.
// -------------------- REPLACE entire tryMainStageAdjusted with this updated version --------------------
static bool tryMainStage(int stage) {
    int vid = pickUnusedVariantIndex(stage);
    if (vid < 0) return false;
    Variant &v = variantsPerStage[stage][vid];

    // compute safe derived values early
    int A1 = paramsA[1];
    if (A1 == 0) A1 = 2; // safety fallback in case something went wrong earlier

    // compute dependent correct answers BEFORE generating options/presentation
    if (stage == 2) {
        // compute A2 from A1 and the numeric result this snippet would print
        int A2 = (abs(A1) % 5) + 2;            // A2 in [2..6]
        int correctValue = computeStage2VariantAnswer(v, A1); // numeric output
        // Build three numeric options (including correctValue and two distractors)
        vector<int> optVals;
        optVals.push_back(correctValue);
        // create two plausible distractors (avoid duplicates)
        int d1 = max(1, correctValue + (rand()%3 + 1)); // correct + [1..3]
        int d2 = max(1, correctValue - (rand()%2 + 1)); // correct - [1..2]
        if (d2 == correctValue) d2 = correctValue + 2;
        if (d1 == correctValue) d1 = correctValue + 3;
        optVals.push_back(d1);
        optVals.push_back(d2);
        // shuffle options
        vector<int> idx = {0,1,2};
        random_device rd; mt19937 g(rd());
        shuffle(idx.begin(), idx.end(), g);
        // present
        title("Control Board — Lights");
        localType("A machine uses your previous insight internally.\n", 18);
        localType("Evaluate the snippet and choose the correct numeric output.\n\n", 16);
        setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
        localType("Options:\n", 15);
        for (int i = 0; i < 3; ++i) {
            cout << "  " << (i+1) << ") " << optVals[idx[i]] << "\n";
        }
        cout << "\nAnswer (1-3): ";
        int choice = readIntFromUser();
        if (choice >= 1 && choice <= 3) {
            int chosenVal = optVals[idx[choice-1]];
            if (chosenVal == correctValue) {
                paramsA[2] = A2;            // store derived A2
                paramsA[stage] = correctValue; // store stage numeric result too
                localType("\nCorrect.\n", 15);
                playBeep(800,120);
                sleepMs(300);
                return true;
            } else {
                localType("\nNot correct.\n", 15);
                return false;
            }
        } else {
            localType("\nInvalid input.\n", 15);
            return false;
        }
    }
    else if (stage == 3) {
        // Stage 3: password — derive A2 safely and compute concatenated password string
        int A2 = (abs(paramsA[1]) % 5) + 2;   // keep same mapping as elsewhere
        if (A2 < 1) A2 = 1;
        if (A2 > 6) A2 = 6;                  // keep password short
        string pwd = computeStage3VariantAnswerAsString(v, A2);
        // store the numeric form in v.correctIntAnswer for later reuse (we use string compare)
        try {
            v.correctIntAnswer = stoi(pwd);
        } catch(...) {
            v.correctIntAnswer = 0;
        }
        // present
        title("Locker — Recursion Password");
        localType("A rusty locker displays a short recursive routine.\n", 18);
        localType("Remember the previous insight. Enter the lock's output (digits only, no spaces).\n\n", 14);
        setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
        localType("Enter the exact output: ");
        string attempt = readLineTrim();
        string correctPwd = to_string(v.correctIntAnswer);
        if (attempt == correctPwd) {
            localType("\nCorrect — the locker accepts the password.\n", 18);
            paramsA[3] = v.correctIntAnswer; // numeric representation stored
            playBeep(900,120);
            sleepMs(200);
            return true;
        } else {
            localType("\nWrong password.\n", 18);
            return false;
        }
    }
    else if (stage == 4) {
        // Stage 4: key selection - compute correct key value based on A3 numeric
        int A3_val = paramsA[3];
        if (A3_val == 0) {
            // fallback: derive a small key from A2
            A3_val = (abs(paramsA[2]) % 5) + 2;
            paramsA[3] = A3_val;
        }
        int correctKeyVal = computeStage4VariantAnswer(v, A3_val);
        // build three candidate keys (numeric), shuffle and present. User picks index (1..3)
        vector<int> keys;
        keys.push_back(correctKeyVal);
        // distractors: +/- small amounts but avoid duplicates and keep positive
        int k1 = max(1, correctKeyVal + (rand()%3 + 1));
        int k2 = max(1, correctKeyVal - (rand()%2 + 1));
        if (k2 == correctKeyVal) k2 = correctKeyVal + 2;
        keys.push_back(k1);
        keys.push_back(k2);
        vector<int> order = {0,1,2};
        random_device rd; mt19937 g(rd());
        shuffle(order.begin(), order.end(), g);
        title("Key Choice — Which Key?");
        localType("A tray holds three keys. Choose the key whose engraving matches the function output.\n\n", 16);
        setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
        cout << "Which key do you pick?\n";
        for (int i=0;i<3;i++) {
            cout << "  " << (i+1) << ") Key #" << (i+1) << " (engraving: " << keys[order[i]] << ")\n";
        }
        cout << "\nChoose key (1-3): ";
        int pick = readIntFromUser();
        if (pick >=1 && pick <=3) {
            int chosenVal = keys[order[pick-1]];
            if (chosenVal == correctKeyVal) {
                paramsA[4] = chosenVal; // store chosen key value for final stage
                localType("\nYou pick the correct key. It hums with power.\n", 18);
                playBeep(820,120);
                sleepMs(300);
                return true;
            } else {
                localType("\nThat key is not right.\n", 18);
                return false;
            }
        } else {
            localType("\nInvalid input.\n", 15);
            return false;
        }
    }
    else if (stage == 5) {
        // Stage 5 final numeric evaluation; compute expected result from A4
        int A4_val = paramsA[4];
        if (A4_val <= 0) {
            localType("\nInternal error: no key value found. Failing.\n", 15);
            return false;
        }
        int expected = computeStage5VariantAnswer(v, A4_val);
        // Present
        title("Final Lock — The Last Recursion");
        localType("A final mechanism requests a computed value. Use your key result.\n\n", 16);
        setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
        localType("Enter the exact integer output: ");
        int ans = readIntFromUser();
        if (ans == expected) {
            localType("\nA deep lock clicks open. The door groans.\n", 18);
            playBeep(1000,150);
            sleepMs(300);
            return true;
        } else {
            localType("\nNot correct.\n", 15);
            return false;
        }
    }
    else {
        // fallback to stage 1 behavior (pattern)
        title("Silence the Echo — Pattern Challenge");
        localType("Pattern visible on the wall:\n\n", 18);
        setColor(14); cout << "   " << v.promptText << "\n\n"; setColor(15);
        localType("Hint code (inspect for the rule):\n", 15);
        setColor(10); cout << "   " << v.hintCode << "\n\n"; setColor(15);
        localType("Type the CORRECT value that should replace the corrupted element to STOP the sound: ", 18);
        int ans = readIntFromUser();
        if (ans == v.correctIntAnswer) {
            paramsA[1] = v.correctIntAnswer;
            localType("\nThe echo subsides.\n", 18);
            playBeep(900,120);
            sleepMs(400);
            return true;
        } else {
            localType("\nNot correct.\n", 15);
            return false;
        }
    }
}


// Try RT for a given stage. Return true if RT passed (allow backtrack), false if failed (exit gate).
static bool tryRTStage(int stage) {
    bool useMedium = (stage >= 2); // make RT slightly harder after stage 1
    int rtIdx = pickUnusedRTIndex(stage, useMedium);
    if (rtIdx < 0) return false;

    MCQ &m = useMedium ? rtPoolMedium[stage][rtIdx] : rtPoolEasy[stage][rtIdx];

    title("Clarification — Side Chamber");
    localType("A small panel appears with a clarification.\n", 16);
    bool ok = askMCQ(m);
    if (ok) {
        localType("\nYou solved the clarification. Returning to the challenge with a new variant.\n", 18);
        // playBeep(950,120);
        sleepMs(500);
        return true;
    } else {
        localType("\nYou failed the clarification.\n", 18);
        // playBeep(300,220);
        sleepMs(300);
        return false;
    }
}

/* -------------------- Gate Matrix (recursive controller) -------------------- */

// This is the true recursive matrix: goes deeper on success, goes right (RT) and then back to same stage on RT success
static bool gateMatrix(int stage) {
    if (stage > 5) return true; // cleared all stages

    // Present MAIN
    bool okMain = tryMainStage(stage);
    if (okMain) {
        // proceed deeper
        return gateMatrix(stage + 1);
    } else {
        // MAIN failed -> go to RT
        bool okRT = tryRTStage(stage);
        if (!okRT) return false; // failing RT means gate exit
        // RT succeeded -> back to same stage with a new variant (recursive call)
        return gateMatrix(stage);
    }
}

/* -------------------- Preparing variants & RT pools for all 5 stages -------------------- */

static void prepareVariantsAndPools() {
    // seed randomness once
    srand((unsigned)time(nullptr) ^ (unsigned)GetTickCount64());
    random_device rd; mt19937 g(rd());

    // Clean any previous data
    for (int s=1;s<=5;s++) {
        variantsPerStage[s].clear();
        rtPoolEasy[s].clear();
        rtPoolMedium[s].clear();
        usedRTIndex[s].clear();
        variantUsedFlag[s].assign(3, false); // we'll have 3 variants per stage
    }

    // Stage 1: three pattern variants (we will store correctIntAnswer as the numeric correct value)
    // generate 3 different pattern variants (primes, fibo, AP)
    {
        Variant v1;
        vector<int> p = genPrimes(5);
        int wrongIdx = rand() % 5;
        int correctVal = p[wrongIdx];
        // corrupt
        int corrupt = correctVal + (rand()%2 ? -1 : 1);
        if (corrupt <= 1) corrupt = correctVal + 2;
        p[wrongIdx] = corrupt;

        // build prompt string
        string sPrompt;
        for (int i=0;i<5;i++) {
            sPrompt += to_string(p[i]);
            if (i<4) sPrompt += ", ";
        }
        v1.promptText = sPrompt;
        v1.correctIntAnswer = correctVal;
        v1.hintCode = "bool isPrime(int n){ for(int i=2;i*i<=n;i++) if(n%i==0) return false; return true; }";
        variantsPerStage[1].push_back(v1);

        Variant v2;
        vector<int> f = genFibo(5);
        int wIdx = 1 + rand() % 4;
        int corr = f[wIdx];
        f[wIdx] = corr + (rand()%2 ? -1 : 1);
        string s2;
        for (int i=0;i<5;i++) { s2 += to_string(f[i]); if (i<4) s2 += ", "; }
        v2.promptText = s2;
        v2.correctIntAnswer = corr;
        v2.hintCode = "int f(int n){ if(n<=2) return 1; return f(n-1)+f(n-2); }";
        variantsPerStage[1].push_back(v2);

        Variant v3;
        vector<int> a = genAP(5);
        int wi = rand() % 5;
        int corr3 = a[wi];
        int d = a[1] - a[0];
        a[wi] = a[wi] + (rand()%2 ? -d : d-1);
        string s3;
        for (int i=0;i<5;i++) { s3 += to_string(a[i]); if (i<4) s3 += ", "; }
        v3.promptText = s3;
        v3.correctIntAnswer = corr3;
        v3.hintCode = "int g(int n){ return a + n*d; }";
        variantsPerStage[1].push_back(v3);
    }

    // Stage 2: three variants - code snippet that uses A1 but we will store the correct MCQ option number as correctIntAnswer
    // We'll represent promptText as the snippet and the correctIntAnswer as the exact integer the snippet would print (so tryMainStage compares ans to that).
    for (int i=0;i<3;i++) {
        Variant v;
        // We'll show code that internally uses "A2" (derived in runtime from A1) but to present we display code that describes operation
        // For variant diversity: use different loop styles (for/while/div)
        if (i==0) {
            v.promptText = "int count=0;\nfor(int k=1; k<=A2; k += 2) count++;\ncout<<count;";
            // correctIntAnswer will be computed at runtime by mapping A1->A2 and then formula; but store placeholder -1 and compute later in tryMainStage
            v.correctIntAnswer = -1;
            v.hintCode = "iterates over odd indices up to A2";
        } else if (i==1) {
            v.promptText = "int ct=0; int t=A2; while(t>0){ ct++; t -= 3; } cout<<ct;";
            v.correctIntAnswer = -1;
            v.hintCode = "counts how many 3-step reductions fit into A2";
        } else {
            v.promptText = "int x=A2; int ct=0; for(int i=1;i<=x;i++) if(i%2==0) ct++; cout<<ct;";
            v.correctIntAnswer = -1;
            v.hintCode = "counts evens from 1..A2";
        }
        variantsPerStage[2].push_back(v);
    }

    // Stage 3: locker recursion variants -> these must produce a concatenated-digit password of length <= 6
    // We'll generate variants that describe the recursion and also precompute correct answer (depending on runtime A2 mapping).
    // We'll store a small code description in promptText and set correctIntAnswer to the integer representing the concatenated digits (e.g., 321 -> 321)
    for (int i=0;i<3;i++) {
        Variant v;
        if (i==0) {
            v.promptText = "void pass(int n){ if(n<=0) return; cout<<n; pass(n-1); } // output: n n-1 ... 1 (concatenated)";
            v.hintCode = "prints n down to 1 concatenated (no spaces)";
        } else if (i==1) {
            v.promptText = "void pass2(int n){ if(n==0) return; cout<<n%10; pass2(n-1); } // variant small";
            v.hintCode = "prints last digit of each descending number";
        } else {
            v.promptText = "void pass3(int n){ for(int i=n;i>=1;i--) cout<<i; }";
            v.hintCode = "simple descending print";
        }
        // correctIntAnswer will be computed at runtime after A2 determined; place holder
        v.correctIntAnswer = -1;
        variantsPerStage[3].push_back(v);
    }

    // Stage 4: key selection variants - ask small code and offer numeric outputs (keys)
    for (int i=0;i<3;i++) {
        Variant v;
        // Prompt describes a small function using A3 (or A2) and asks "which is the correct output?" We'll compute runtime.
        v.promptText = "int keyFunc(int x){ if(x%2==0) return x/2; else return x+1; } // compute key value";
        v.hintCode = "simple parity-based mapping";
        v.correctIntAnswer = -1;
        variantsPerStage[4].push_back(v);
    }

    // Stage 5: final lock variants - small recursion using A4
    for (int i=0;i<3;i++) {
        Variant v;
        if (i==0) {
            v.promptText = "int unlock(int n){ if(n<=1) return 1; return n * unlock(n-1); } // factorial(n)";
            v.hintCode = "factorial of chosen key value";
        } else if (i==1) {
            v.promptText = "int s(int n){ if(n==0) return 0; return n + s(n-1); } // sum 1..n";
            v.hintCode = "sum from 1 to n";
        } else {
            v.promptText = "int p(int n){ if(n<=1) return 1; return p(n-1) + n*n; } // sum of squares variant";
            v.hintCode = "sum of squares (small n)";
        }
        v.correctIntAnswer = -1;
        variantsPerStage[5].push_back(v);
    }

    // RT pools: fill with a variety of MCQs (some GK, some DSA). We'll put several per stage.
    for (int s=1;s<=5;s++) {
        // easy pool
        rtPoolEasy[s] = {
            {"Which is NOT a prime?", {"2","9","7"}, 2},
            {"FIFO stands for?", {"Stack","Queue","Tree"}, 2},
            {"2^5 = ?", {"32","16","64"}, 1},
            {"Which is even?", {"7","9","12"}, 3}
        };
        // medium pool
        rtPoolMedium[s] = {
            {"Binary search time complexity?", {"O(n)","O(log n)","O(n log n)"}, 2},
            {"Which sort is stable?", {"Quick sort","Merge sort","Heap sort"}, 2},
            {"BFS uses which DS?", {"Stack","Queue","Priority Queue"}, 2},
            {"What is push/pop on a stack?", {"FIFO","LIFO","Parallel"}, 2}
        };
    }
}

/* -------------------- Helper: compute runtime-correct answers for variants that depend on earlier params -------------------- */

static int computeStage2VariantAnswer(const Variant &v, int A1) {
    // map A1 -> A2 safely in [2..6]
    int A2 = (abs(A1) % 5) + 2;
    // detect which snippet variant by checking hint text
    if (v.hintCode.find("odd") != string::npos || v.promptText.find("k+=2") != string::npos) {
        // count odds from 1 to A2 inclusive: number of odd integers <= A2 = (A2+1)/2
        return (A2 + 1) / 2;
    } else if (v.promptText.find("t -= 3") != string::npos) {
        int t = A2, ct=0;
        while (t > 0) { ct++; t -= 3; }
        return ct;
    } else if (v.promptText.find("i%2==0") != string::npos) {
        int ct=0;
        for (int i=1;i<=A2;i++) if (i%2==0) ct++;
        return ct;
    }
    // fallback
    return (A2 + 1) / 2;
}

static string computeStage3VariantAnswerAsString(const Variant &v, int A2) {
    // We design Stage3 variants so outputs are concatenated n..1 ; keep A2 <=6
    if (v.hintCode.find("descending") != string::npos || v.promptText.find("cout<<n; pass(n-1)") != string::npos ||
        v.promptText.find("n n-1") != string::npos) {
        return passPrint(A2);
    }
    // fallback
    return passPrint(A2);
}

static int computeStage4VariantAnswer(const Variant &v, int A3_value) {
    // v.promptText used parity example
    if (v.promptText.find("if(x%2==0)") != string::npos) {
        if (A3_value % 2 == 0) return A3_value/2;
        else return A3_value + 1;
    }
    // fallback: return A3_value
    return A3_value;
}

static int computeStage5VariantAnswer(const Variant &v, int A4_value) {
    if (v.promptText.find("factorial") != string::npos || v.promptText.find("unlock") != string::npos) {
        long long r = finalFunc(A4_value);
        // ensure result fits int (A4_value small)
        return (int)r;
    } else if (v.promptText.find("sum 1..n") != string::npos) {
        int n = A4_value;
        return n * (n+1) / 2;
    } else if (v.promptText.find("sum of squares") != string::npos) {
        int n = A4_value;
        int s = 0; for (int i=1;i<=n;i++) s += i*i;
        return s;
    }
    return finalFunc(A4_value);
}

/* -------------------- tryMainStage overload adjustment: compute dependent correct answers before prompting -------------------- */

static bool tryMainStageAdjusted(int stage) {
    int vid = pickUnusedVariantIndex(stage);
    if (vid < 0) return false;
    Variant &v = variantsPerStage[stage][vid];

    // Before presenting, compute v.correctIntAnswer for variants whose answer depends on earlier params
    if (stage == 2) {
        // compute A2 from A1 and then compute the value for this variant
        int A1 = paramsA[1];
        int correct = computeStage2VariantAnswer(v, A1);
        v.correctIntAnswer = correct;
    } else if (stage == 3) {
        // compute password string for A2 then store as integer concatenation
        int A2 = (abs(paramsA[1]) % 5) + 2;
        string pwd = computeStage3VariantAnswerAsString(v, A2);
        // store numeric representation (no leading zeros guaranteed)
        try {
            v.correctIntAnswer = stoi(pwd);
        } catch(...) {
            v.correctIntAnswer = 0;
        }
    } else if (stage == 4) {
        // stage4 depends on A3 (which for our design is numeric); we stored paramsA[3] earlier.
        int A3 = paramsA[3];
        int correct = computeStage4VariantAnswer(v, A3);
        v.correctIntAnswer = correct;
    } else if (stage == 5) {
        int A4 = paramsA[4];
        int correct = computeStage5VariantAnswer(v, A4);
        v.correctIntAnswer = correct;
    }

    // Now present the variant using tryMainStage logic but with updated correctIntAnswer
    // For stage 3 (password) we want the user to enter the digits only; we stored correctIntAnswer as numeric concatenation
    // Slight UI differences handled in tryMainStage: but we'll call it after temporarily replacing the prompt's prepared values.

    // Present accordingly:
    switch(stage) {
        case 1:
            title("Silence the Echo — Pattern Challenge");
            localType("Pattern visible on the wall:\n\n", 18);
            setColor(14); cout << "   " << v.promptText << "\n\n"; setColor(15);
            localType("Hint code (inspect for the rule):\n", 15);
            setColor(10); cout << "   " << v.hintCode << "\n\n"; setColor(15);
            localType("Type the CORRECT value that should replace the corrupted element to STOP the sound: ", 18);
            break;
        case 2:
            title("Control Board — Lights");
            localType("A machine uses your previous insight internally.\n", 18);
            localType("Evaluate the snippet and choose the correct output option.\n\n", 16);
            setColor(10);
            cout << "   " << v.promptText << "\n\n";
            setColor(15);
            break;
        case 3:
            title("Locker — Recursion Password");
            localType("A rusty locker displays a short recursive routine.\n", 18);
            localType("Note: We do NOT display the call value. Remember your previous answer.\n\n", 14);
            setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
            localType("Enter the exact output (digits only, no spaces): ");
            break;
        case 4:
            title("Key Choice — Which Key?");
            localType("A tray holds three keys. Choose the key that matches the correct function output.\n\n", 16);
            setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
            break;
        case 5:
            title("Final Lock — The Last Recursion");
            localType("A final mechanism requests a computed value. Use your key result.\n\n", 16);
            setColor(10); cout << "   " << v.promptText << "\n\n"; setColor(15);
            localType("Enter the exact integer output: ");
            break;
        default:
            return false;
    }
if(stage == 2){
    localType("Options:\n",15);
    vector<string> opts = {"1","2","3","4","5","6"}; // depending on max A2
    for(int i=0;i<opts.size();i++) cout << " " << (i+1) << ") " << opts[i] << "\n";
    cout << "\nAnswer (1-" << opts.size() << "): ";
}

    // Now read and compare:
    if (stage == 3) {
        string attempt = readLineTrim();
        string correctPwd = to_string(v.correctIntAnswer);
        if (attempt == correctPwd) {
            localType("\nCorrect — the locker accepts the password.\n", 18);
            paramsA[3] = v.correctIntAnswer; // store numeric representation of the password as A3
            // playBeep(900,120);
            return true;
        } else {
            localType("\nWrong password.\n", 18);
            return false;
        }
    } else {
        int ans = readIntFromUser();
        if (ans == v.correctIntAnswer) {
            paramsA[stage] = v.correctIntAnswer;
            localType("\nCorrect.\n", 15);
            // playBeep(800,120);
            sleepMs(300);
            return true;
        } else {
            localType("\nNot correct.\n", 15);
            return false;
        }
    }
}

/* -------------------- Run entry for Gate1 -------------------- */

void runGate1() {
    // Prepare data
    prepareVariantsAndPools();

    // Intro
    title("GATE I — LOOP OF ECHOES");
    localType("You step into a metallic chamber. A harsh repeating sound fills the air.\n", 18);
    localType("A voice whispers: \"Break the pattern. Stabilize the echo.\"\n\n", 22);
    for (int i=0;i<7;i++){ playBeep(700 + i*50,600); sleepMs(100); }

    // Clear any previous params
    for (int i=0;i<5;i++) paramsA[i] = 0;

    // Stage 1 special: we need to present pattern variants and use the user's correct numeric answer as A1
    // We'll use our recursive controller gateMatrix, but we must ensure that Stage 1's tryMain uses stage-specific mechanics
    // For that, adapt tryMainStageAdjusted to fill in computed answers that depend on earlier params

    // Replace tryMainStage with tryMainStageAdjusted by using a small wrapper that maps stage 1 special behavior and others to adjusted:
    auto tryMainWrapper = [&](int stage) -> bool {
        if (stage == 1) {
            // present stage 1 as previously constructed (variantsPerStage[1] already has correctIntAnswer set)
            // We'll reuse tryMainStage but ensure the variant we pick is consistent
            int vid = pickUnusedVariantIndex(1);
            if (vid < 0) return false;
            Variant &v = variantsPerStage[1][vid];
            title("Silence the Echo — Pattern Challenge");
            localType("Pattern visible on the wall:\n\n", 18);
            setColor(14); cout << "   " << v.promptText << "\n\n"; setColor(15);
            localType("Hint code (inspect for the rule):\n", 15);
            setColor(10); cout << "   " << v.hintCode << "\n\n"; setColor(15);
            localType("Type the CORRECT value that should replace the corrupted element to STOP the sound: ", 18);
            int ans = readIntFromUser();
            if (ans == v.correctIntAnswer) {
                paramsA[1] = v.correctIntAnswer;
                localType("\nThe echo subsides.\n", 18);
                // playBeep(900,700);
               
                sleepMs(400);
                return true;
            } else {
                localType("\nNot correct.\n", 15);
                return false;
            }
        } else {
            // For stages 2..5 use adjusted logic (compute correct answers from earlier params)
            return tryMainStageAdjusted(stage);
        }
    };

    // We will adapt gateMatrix to call tryMainWrapper and tryRTStage accordingly.
    // Local recursive lambda:
    function<bool(int)> matrix = [&](int stage)->bool {
        if (stage > 5) return true;
        bool ok = tryMainWrapper(stage);
        if (ok) {
            // Special behaviors: after stage 1 success we must compute A2 mapping (A2 derived) and continue
            if (stage == 1) {
                // compute safe A2 in [2..6]
                int A1 = paramsA[1];
                int A2 = (abs(A1) % 5) + 2;
                paramsA[2] = A2;
            }
            // after stage 3 (locker) we need to turn the stored numeric password into a usable A3 value (already set)
            // proceed deeper
            return matrix(stage + 1);
        } else {
            bool okrt = tryRTStage(stage);
            if (!okrt) return false;
            // RT succeeded -> back to same stage with a new variant
            return matrix(stage);
        }
    };

    // Run the matrix
    bool cleared = matrix(1);

    // If cleared, we still must perform the "key selection" stage flow (because our variants may not have set keys)
    if (!cleared) {
        localType("\nYou could not clear Gate 1. Return to the Nexus.\n", 20);
        localType("\nPress ENTER to continue..."); readLineTrim();
        return;
    }

    // At this point stages 1..3 were solved during recursion, but stage 4 (key selection) and stage 5 (final) should have been processed too
    // However, depending on how tryMainStageAdjusted ran, paramsA[4] and paramsA[5] might be set.
    // If final cleared:
    title("GATE 1 — RESULT");
    localType("\nYou have emerged from the Loop of Echoes.\n", 20);
    setColor(10); localCenter("🔓 DOOR UNLOCKED — You escaped Gate 1", 80); setColor(15);
    localType("\nGate 1 cleared. Recursion Insight +2, Logic Pulse +1\n", 18);
    localType("\nPress ENTER to continue..."); readLineTrim();
}
