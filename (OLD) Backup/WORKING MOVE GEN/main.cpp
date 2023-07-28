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
#include <mutex>
#include <ctype.h>
#include "types.h"
#include "bitboards.cpp"
#include "movegen.cpp"
#include "position.cpp"
#include "misc.cpp"

using namespace std;
double moveTimer = 0;
long long int nodeCount = 0, enPassants = 0, captures = 0, castling = 0, check = 0, promo = 0;
const int numThreads = 3;
const bool useThreads = false;
int maxDepth = 6;

Position SuperBoard;

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
void splitMoveList(MoveList moves, vector<MoveList> &moveLists) {
    int length = moves.size();

    // create the move lists
    for (int i = 0; i < numThreads; i++) {
        MoveList l;
        moveLists.emplace_back(l);
    }

    int i = 0;
    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();

        moveLists.at(i).emplace_back(move);
        i = (i + 1) % numThreads;
    }
}
void test() {
    speedTest();
}
void init() {
    SuperBoard.init();
    initStaticMasks(); // used to create masks for knights/kings for move gen for each square
}

void reccursiveMoveCheck(int depth, Position &ChessBoard) {
    if (depth == maxDepth - 1) {
        MoveList moves = ChessBoard.getMoveList();
        nodeCount += moves.size();

        for (auto move: moves) {
            short from, to, promo, mcode, ftype, ttype;
            SuperBoard.decodeMove(move, from, to, promo, mcode, ftype, ttype);
            if (mcode == PROMOTION) {
                promo += 1;
            } else if (mcode == ENPASSANT) {
                enPassants += 1;
            } else if (ttype != EMPTY) {
                captures += 1;
            } else if (mcode == CASTLING) {
                castling += 1;
            }
        }

        return;
    }

    MoveList moves = ChessBoard.getMoveList();

    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();
//                    U64 BBs[9];
//                    for (int i = 0; i < 9; i ++) {
//                        BBs[i] = Board.getBB(i);
//                    }
        ChessBoard.makeMove(move);
        //            cout << "+++++++++++++++++++++++++++++++++++\n";
        //            Board.printBoardPrettily();
        //            Board.printMovesPrettily(moves);
        reccursiveMoveCheck(depth + 1, ChessBoard);

        ChessBoard.unMakeMove();

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
void startThread(MoveList moves, Position &ChessBoard) {
//    ChessBoard.init();
    if (maxDepth == 1) {
        nodeCount += moves.size();
        return;
    }

    int i = 0;
    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();

        ChessBoard.makeMove(move);

        reccursiveMoveCheck(1, ChessBoard);

        ChessBoard.unMakeMove();

        i++;
    }
}
void doReccursiveThings() {
    /* split the moves between the 4 threads */
    MoveList moves = SuperBoard.getMoveList();
    vector<MoveList> moveLists;
    vector<Position> positions;
    vector<int> counts; // stores the counts for each sub move

    nodeCount = 0, enPassants = 0, castling = 0, captures = 0, promo = 0;

    auto start = chrono::high_resolution_clock::now();
    if (useThreads) {
        vector<thread> threads;
        splitMoveList(moves, moveLists);

        /* create the threads */
        for (auto moveList: moveLists) {
            printMovesPrettily(moveList);
            positions.emplace_back(Position());
            positions.back().init();
            threads.emplace_back(thread(startThread, moveList, ref(positions.back())));
        }

        /* join the threads */
        for (auto &thread: threads) {
            thread.join();
        }
    } else {
        startThread(moves, SuperBoard);
    }

    printMovesPrettily(moves);

    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    moveTimer = elapsed.count();

    cout << "Nodes per second: " << nodeCount / moveTimer << "\n";
    cout << "Time: " << moveTimer << "\n";
    cout << "Nodes: " << nodeCount << "\n";
    cout << "En-passants: " << enPassants << "\n";
    cout << "Captures: " << captures + enPassants << "\n";
    cout << "Castles: " << castling << "\n";
    cout << "Promos: " << promo << "\n";
}

short getPieceCode(char c) {
    c = tolower(c);
    switch (c) {
        case 'p':
            return PAWN;
        case 'n':
            return KNIGHT;
        case 'b':
            return BISHOP;
        case 'r':
            return ROOK;
        case 'q':
            return QUEEN;
        case 'k':
            return KING;
    }
    return EMPTY;
}
void splitString(string str, string *arr, string brk, int num) {
    for (int i = 0; i < num; i ++) {
        int space = str.find(brk);
        arr[i] = str.substr(0, space);
        str = str.substr(space + 1, -1);
    }
}
void readFEN(string FEN) {
    /* reset the board */
    for (int i = 0; i < 9; i++) SuperBoard.pieceBB[i] = 0;
    SuperBoard.occupiedSquares = 0;
    SuperBoard.emptySquares = ~0;

    /* split the FEN into it's components */
    string sections[6];
    splitString(FEN, sections, " ", 6);

    /* split up the position into ranks */
    string ranks[8];
    splitString(sections[0], ranks, "/", 8);
    for (int r = 0; r < 8; r ++) {
        // reset the 8 different piece bitboards ~ 6 pawns + 2 colours
        string rank = ranks[r]; // get the rank
        // loop through the characters in the string
        int index = 0;
        for (char c: rank) {
            if (isdigit(c)) {
                index += int(c) - 48;
            } else {
                // add the piece to the board
                short piece = getPieceCode(c);
                short square = r * 8 + index;
                short side = isupper(c) ? nWhite : nBlack;
                SuperBoard.setSquare(piece, side, square);
                index ++;
            }
        }
    }

    /* see who's turn it is */
    short side = sections[1] == "w" ? WHITE : BLACK;
    if (side != SuperBoard.currentSide) SuperBoard.switchSide();

    /* now deal with castling rights */
    // start of by setting castling rights to 0
    SuperBoard.CstleRights = 0;
    for (char c: sections[2]) {
        switch (c) {
            case 'K':
                // white can castle king side
                SuperBoard.CstleRights |= 2;
            case 'Q':
                SuperBoard.CstleRights |= 1;
            case 'k':
                SuperBoard.CstleRights |= 8;
            case 'q':
                SuperBoard.CstleRights |= 4;
        }
    }

    /* now deal with en-passant rights */
    string enpassSquare = sections[3];
    U64 enPassantRights = 0;
    if (enpassSquare != "-") {
        enPassantRights = int(enpassSquare[0]) - int('a');
        enPassantRights = toBB(enPassantRights);
    }
    SuperBoard.enPassantRights = enPassantRights;

    /* the other sections aren't needed for now */
}
void debugMode() {
    MoveList moves = SuperBoard.getMoveList();
    while (true) {
        cout << "\nWaiting for command: ";
        string command;
        cin >> command;
        if (command == "exit") {
            break;
        } else if (command == "move") {
            cout << "       Enter the move ID: ";
            int moveID;
            cin >> moveID;

            if (moveID < moves.size() && moveID >= 0) {
                SuperBoard.makeMove(moves.at(moveID));
                moves = SuperBoard.getMoveList();
            } else {
                cout << "       Invalid move";
            }
        } else if (command == "unmove") {
            SuperBoard.unMakeMove();
            moves = SuperBoard.getMoveList();
        } else if (command == "print") {
            SuperBoard.printBoardPrettily();
            printMovesPrettily(moves);
        } else if (command == "perft") {
            cout << "       Enter depth: ";
            cin >> maxDepth;
            doReccursiveThings();
        } else if (command == "fen") {
            string FEN, FEN2;
            cout << "       Enter FEN: ";
            cin >> FEN;
            getline(cin, FEN2);
            readFEN(FEN + FEN2);
            moves = SuperBoard.getMoveList();
        }
    }
}

int main() {
    /* computer has two cores
     * two threads for each core
     * therefore 4 threads all together*/
    init();

    readFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    debugMode();

//    doReccursiveThings();

    return 0;
}

/* LOOK INTO MULTITHREADING */
