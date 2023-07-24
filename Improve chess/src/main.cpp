#include <iostream>
#include <chrono>  // for high_resolution_clock
#include <string>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <mutex>
#include <ctype.h>
#include <fstream>
#include "types.h"
#include "Board/bitboards.cpp"
#include "Board/movegen.cpp"
#include "Search/SearchState.cpp"
#include "misc.cpp"
#include "evaluation.cpp"
#include "Search/search.cpp"

using namespace std;
const string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const string logsPath = "/Users/noahjoubert/CLionProjects/Chessington/Improve chess/logs/";

double moveTimer = 0;
long long int nodeCount = 0;
int numThreads = 5;
bool useThreads = true;
bool parallel = true;
bool useLogs = true;
int maxDepth = 6;

SearchState SuperBoard;
mutex mtx;

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
void reccursiveMoveCheck(int depth, SearchState &ChessBoard, int &count) {
    if (depth == maxDepth - 1) {
        MoveList moves = ChessBoard.getMoveList();

        count += moves.size();

        return;
    }

    MoveList moves = ChessBoard.getMoveList();

    for (Move move: moves) {

        ChessBoard.makeMove(move);

        reccursiveMoveCheck(depth + 1, ChessBoard, count);

        ChessBoard.unMakeMove();
    }
}
void startThread(MoveList moves, SearchState ChessBoard) {
    /* note we pass by value here to allow the same super-board to be used for different threads */
    if (maxDepth == 1) {
        nodeCount += moves.size();
        return;
    }

    int count = 0; // count variable for this thread

    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();

        ChessBoard.makeMove(move);

        reccursiveMoveCheck(1, ChessBoard, count);

        ChessBoard.unMakeMove();
    }

    mtx.lock(); // lock as it's accessed by multiple threads
    nodeCount += count; // add the count to the global count
    mtx.unlock();
}
float perft() {
    auto start = chrono::high_resolution_clock::now();

    MoveList moves = SuperBoard.getMoveList();
    vector<MoveList> moveLists;
    vector<SearchState> positions;
    vector<int> counts; // stores the counts for each sub move

    nodeCount = 0;

    if (useThreads) {
        /* split the move list evenly */
        splitMoveList(moves, moveLists);

        /* create the threads */
        if (!parallel) {
            int i = 0;
            cout << "{";
            for (MoveList mList: moveLists) {
                i++;

                /* create and join the thread */
                thread t(startThread, mList, SuperBoard);
                t.join();
                cout << "\n\tThread " << i << ": " << nodeCount << "";
            }
            cout << "\n}\n";
        } else {
            vector<thread> threads;
            for (MoveList mList: moveLists) {
                threads.emplace_back(thread(startThread, mList, SuperBoard));
            }

            for (thread &T: threads) {
                T.join();
            }
        }
    } else {
        startThread(moves, SuperBoard);
    }

    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    moveTimer = elapsed.count();

    if (useLogs) {
        // write the result to file
        string fileName;
        if (useThreads) {
            fileName = "/Users/Noah/CLionProjects/Improve chess/logs/multi-thread.txt";
        } else {
            fileName = "/Users/Noah/CLionProjects/Improve chess/logs/single-thread.txt";
        }

        // open, write, close the file
        ofstream File;
        File.open(fileName, ios::app);
        File << "\n" << nodeCount << " " << moveTimer;
        File.close();
    }

    return moveTimer;
}
void doReccursiveThings() {
    moveTimer = perft();

    cout << "Nodes per second: " << nodeCount / moveTimer << "\n";
    cout << "Time: " << moveTimer << "\n";
    cout << "Nodes: " << nodeCount << "\n";
}

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
void multiVsSingle() {
    int num = 50;
    maxDepth = 5;
    useThreads = true;

    // open the file to both read and write
    ifstream file;
    file.open("/Users/Noah/CLionProjects/Improve chess/logs/threads-nps.txt", ios::in);
    float arr[20];
    int count;
    string countstr;
    getline(file, countstr);
    count = stoi(countstr) + 1;
    for (numThreads = 1; numThreads <= 20; numThreads ++) {
        float time = 0, nodes = 0, nps;
        for (int i = 0; i < num; i++) {
            time += perft();
            nodes += nodeCount;
        }
        string l;
        string parts[2];
        float prevNPS = 0;
        nps = (nodes / time);
        if (getline(file, l)) {
            cout << "File nps: " << l << " ";
            prevNPS = stof(l) * count;
            nps += prevNPS;
            nps /= (count + 1);
        }
        arr[numThreads - 1] = nps;
        cout << "Threads: " << numThreads << "\t\t\tNPS: " << nps << "\n";
    }
    file.close();

    ofstream file1;
    file1.open("/Users/Noah/CLionProjects/Improve chess/logs/threads-nps.txt", ios::out);
    file1 << count << "\n";
    for (int i = 0; i < 20; i++) {
        file1 << arr[i] << (i == 19 ? "" : "\n");
        float hundreths = arr[i] / 10000000;
        hundreths -= 2.6;
        hundreths *= 25;
        hundreths = int(hundreths);
        cout << "Threads: " << i + 1 << "\t\t";
        for (int i = 0; i < hundreths; i++) {
            cout << "=";
        }
        cout << "\n";
    }
}
void doGame() {
    // see what game 'number' this is
    string fileName = logsPath + "games/num.txt";

    // open the file
    ifstream inputFile;
    inputFile.open(fileName, ios::in);

    // get the number
    string numStr;
    getline(inputFile, numStr);
    int num = stoi(numStr) + 1;

    // close the input file
    inputFile.close();

    // open the write file
    ofstream outputFile;
    outputFile.open(fileName, ios::out);

    // write the number
    outputFile << num;
    outputFile.close();

    int moveNum = 1;
    int side = WHITE;

    while (true) {
        ofstream gameFile;
        gameFile.open(logsPath + "games/game-" + to_string(num) + ".txt", ios::app);

        // do the search and get the best move
        Move bestMove;
        char twice = '-';
        bool keepPlaying = search(SuperBoard, bestMove, twice);

        // break if the game is over
        if (!keepPlaying) {
            break;
        }

        // decode the move
        short fromSq, toSq, promo, flag, fromPc, toPc;
        decodeMove(bestMove, fromSq, toSq, promo, flag, fromPc, toPc);

        string moveString = "";
        if (side == WHITE) {
            moveString += to_string(moveNum) + ". ";
        }

        if (flag == CASTLING) {
            // castle queenside
            if (toSq == A1 || toSq == A8) {
                moveString += "O-O-O";
            } else {
                moveString += "O-O";
            }
        } else if (flag == PROMOTION) {
            string promoStr;
            if (promo == QUEENPROMO) {
                promoStr = "Q";
            } else if (promo == BISHOPPROMO) {
                promoStr = "B";
            } else if (promo == KNIGHTPROMO) {
                promoStr = "N";
            } else if (promo == ROOKPROMO) {
                promoStr = "R";
            }

            // get the start file of the pawn
            char startFile = char(int('a') + fromSq % 8);

            // see if this was a capture
            if (toPc != EMPTY) {
                // capture
                moveString.push_back(startFile);
                moveString += "x" + SquareStrings[toSq] + "=" + promoStr;
            } else {
                moveString += SquareStrings[toSq] + "=" + promoStr;
            }
        } else if (flag == ENPASSANT) {
            char startFile = char(int('a') + fromSq % 8);

            moveString.push_back(startFile);
            moveString += "x" + SquareStrings[toSq];
        } else {
            // see if it is a pawn move
            if (fromPc == PAWN) {
                char startFile = char(int('a') + fromSq % 8);

                if (toPc != EMPTY) {
                    // capture
                    moveString.push_back(startFile);
                    moveString += "x" + SquareStrings[toSq];
                } else {
                    // not capture
                    moveString += SquareStrings[toSq];
                }
            } else {
                // get the piece string
                string pcString;
                cout << fromPc << "\n";
                if (fromPc == KNIGHT) {
                    pcString = "N";
                } else if (fromPc == BISHOP) {
                    pcString = "B";
                } else if (fromPc == ROOK) {
                    pcString = "R";
                } else if (fromPc == KING) {
                    pcString = "K";
                } else {
                    pcString = "Q";
                }

                moveString += pcString;

                if (twice != '-') moveString.push_back(twice);

                // see if it was a capture
                if (toPc != EMPTY) {
                    // capture
                    moveString += "x" + SquareStrings[toSq];
                } else {
                    moveString += SquareStrings[toSq];
                }
            }
        }

        if (side == BLACK) {
            moveNum ++;
            side = WHITE;
            moveString += " ";
        } else {
            side = BLACK;
            moveString += " ";
        }

        gameFile << moveString;

        gameFile.close();
    }
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
            SuperBoard.readFEN(FEN + FEN2);
            moves = SuperBoard.getMoveList();
        } else if (command == "checkThreefold") {
            if (SuperBoard.checkThreefold()) {
                cout << "Three fold!\n";
            } else {
                cout << "Not there fold!\n";
            }
        } else if (command == "threads") {
            string response;
            cout << "Use threads? (true or false): ";
            cin >> response;
            if (response == "true") {
                useThreads = true;
            } else {
                useThreads = false;
            }
            cout << "       Use threads: " << (useThreads ? "True" : "False") << "\n";
        } else if (command == "parallel") {
            string response;
            cout << "Process in parallel? (true or false): ";
            cin >> response;
            if (response == "true") {
                parallel = true;
            } else {
                parallel = false;
            }
            cout << "       Parallel: " << (parallel ? "True" : "False") << "\n";
        } else if (command == "search") {
            Move m;
            char t;
            search(SuperBoard, m, t);
            moves = SuperBoard.getMoveList();
        } else if (command == "movecode") {
            string fromS, toS;
            cout << "   From: " << "\n";
            cin >> fromS;
            cout << "   From: " << "\n";
            cin >> toS;

            short from, to;
            for (int i = 0; i < 64; i++) {
                if (fromS == SquareStrings[i]) from = i;
                if (toS == SquareStrings[i]) to = i;
            }
            for (Move move: moves) {
                short f, t, x;
                decodeMove(move, f, t, x, x, x, x);
                if (f == from && t == to) {
                    SuperBoard.makeMove(move);
                    SuperBoard.printBoardPrettily();
                    moves = SuperBoard.getMoveList();
                }
            }
        } else if (command == "tt") {
            string s;
            useTT = !useTT;
            cout << "   Use Transposition Table (y/n): ";
            cin >> s;
            if (s == "y") {
                useTT = true;
            } else {
                useTT = false;
            }
            cout << "   Result: " << useTT << "\n";
        } else if (command == "play") {
            doGame();
        } else if (command == "searchtime") {
            cout << "       Enter new min search time: ";
            cin >> minSearchTime;
            cout << "       New min search time: " << minSearchTime << "\n";
        } else if (command == "qmoves") {
            printMovesPrettily(SuperBoard.getQMoveList());
            SuperBoard.getMoveList();
        }
    }
}

void init() {
    initZobrist();
    SuperBoard.init();
    initStaticMasks(); // used to create various masks
    initEval();
    /* create initSearch function */
    TT.clear();
    SuperBoard.readFEN(initialFEN);
}

int main() {
    /* computer has two cores
     * two threads for each core
     * therefore 4 threads all together*/
    init();

    debugMode();

    return 0;
}
