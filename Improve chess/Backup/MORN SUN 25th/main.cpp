#include <iostream>
#include <chrono>  // for high_resolution_clock
#include <SDL2/SDL.h>
#include <string>
#include <iomanip>
#include <nmmintrin.h>
#include <immintrin.h>
#include <mmintrin.h>
#include <cstdlib>
#include <thread>
#include "types.h"
#include "bitboards.cpp"
#include "movegen.cpp"
#include "position.cpp"
#include "misc.cpp"

using namespace std;
double moveTimer = 0;
long long int nodeCount = 0, enPassants = 0, captures = 0, castling = 0, check = 0, promo = 0;

void speedTest() {
    int max = 1000000;
    double timer1=0, timer2=0;
    chrono::duration<double> elapsed;
    U64 num = C64(12142195);
    U64 num1 = C64(12412412421);
    unsigned short num2 = 34;
    unsigned short num3 = 24;

    auto start = chrono::high_resolution_clock::now();
    for (int i = 0; i < max; i++) {
        U64 num5 = num << 8;
        bool x = num == num1;
        cout << num5 << x;
    }
    auto finish = chrono::high_resolution_clock::now();
    elapsed = finish - start;
    timer1 = elapsed.count();

    start = chrono::high_resolution_clock::now();
    for (int i = 0; i < max; i++) {
        bool x= num2 == num3;
        cout << num2 << x;
    }
    finish = chrono::high_resolution_clock::now();
    elapsed = finish - start;
    timer2 = elapsed.count();

    cout << "\nNew:" << timer1 << "\n";
    cout << "Old:" << timer2 << "\n";
}
void test() {
    speedTest();
}
void reccursiveMoveCheck(int depth) {
    if (depth == 7) {
        //        Board.printEverything();
        nodeCount += 1;
        return;
    }

    MoveList moves = getMoveList();

    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();

        //            U64 BBs[9];
        //            for (int i = 0; i < 9; i ++) {
        //                BBs[i] = Board.getBB(i);
        //            }

        makeMove(move);

            //            cout << "+++++++++++++++++++++++++++++++++++\n";
        //            Board.printBoardPrettily();
        //            Board.printMovesPrettily(moves);

        reccursiveMoveCheck(depth + 1);

        unMakeMove();

        //            U64 newBBs[9];
        //            for (int i = 0; i < 9; i ++) {
        //                newBBs[i] = Board.getBB(i);
        //            }
        //            for (int i = 0; i < 9; i ++) {
        //                if (BBs[i] != newBBs[i]) {
        //                    cout << "+++++++++++++++++ Problem has been found! ++++++++++++++++++\n";
        //                    cout << "i: " << i << "\n";
        //                    cout << "Node count: " << nodeCount << "\n";
        //                    cout << "current depth: " << depth << "\n";
        //                    Board.printBoardPrettily();
        //                    Board.printMovesPrettily(moves);
        //                    Board.printMoveBitboard(move);
        //                    Board.printMoveHistory();
        //                    Board.printEverything();
        //                    cout << "\n\n";
        //                    Board.printBitboard(BBs[nBlack], nPiece);
        //                    Board.printBitboard(newBBs[nBlack], nPiece);
        //                    assert(0);
        //                }
        //            }
    }
}

void doReccursiveThings() {
    auto start = chrono::high_resolution_clock::now();
    reccursiveMoveCheck(1);
    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    moveTimer = elapsed.count();

    printBoardPrettily();

    cout << "Nodes per second: " << nodeCount / moveTimer << "\n";
    cout << "Time: " << moveTimer << "\n";
    cout << "Nodes: " << nodeCount << "\n";
    cout << "En-passants: " << enPassants << "\n";
    cout << "Captures: " << captures << "\n";
    cout << "Castles: " << castling << "\n";
    cout << "Checks: " << check << "\n";
    cout << "Promos: " << promo << "\n";
}

int main() {
    init();

    /* computer has two cores
     * two threads for each core
     * therefore 4 threads all together*/

    thread myThread(doReccursiveThings);
    myThread.join();

    return 0;
}

/* LOOK INTO MULTITHREADING */