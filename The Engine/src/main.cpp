#include <iostream>
#include <string>
#include <iomanip>
#include <fstream>
#include "Search/SearchController.cpp"
#include "(OLD) API.cpp"
#include "perft.cpp"
#include "UCI.cpp"

using namespace std;
const string logsPath = getCWDString() + "/logs/";

void printSearchResults(SearchResults results, SearchController SuperBoard) {
    SearchStats searchStats = results.stats;
    TranspositionTable *TT = SuperBoard.getTT();

    cout << "---------------------=+ Search Results " << SuperBoard.getMoveNumber() - 1 << ". +=---------------------\n";
    printMovesPrettily(results.principleVariation);
    cout << "Results: \n";
    cout << "\tMove: " << moveToFEN(results.bestMove, "-") << " | ";
    cout << "Eval: " << results.evaluation << "\n";
    cout << "\tDepth: " << results.depth << " | ";
    cout << "Time: " << results.searchTime << "s | ";
    cout << "Nodes: " << (float) searchStats.totalNodesSearched / 1000000 << " million | ";
    cout << "Nodes per second: " << (float) searchStats.totalNodesSearched / 1000000 / results.searchTime << " million | ";
    cout << "Quiescence proportion: " << (float) searchStats.totalQuiescenceSearched / searchStats.totalNodesSearched * 100 << "% | ";
    cout << "Non-capture Q proportion: " << (float) searchStats.totalNonCaptureQSearched / searchStats.totalQuiescenceSearched * 100 << "% | ";
    cout << "\n";
    cout << "Transposition Table: \n";
    cout << "\tFill rate: " << (float)TT->totalUniqueNodes / TT->getSize() * 100 << "% | ";
    cout << "Absolute size: " << (float)TT->totalUniqueNodes * sizeof(TTNode) / 1000000 << "mb\n";
    cout << "\tProbe hit rate: " << (float)TT->totalProbeFound / TT->totalProbeCalls * 100 << "% | ";
    cout << "{Exact Probe Rate: " << (float)TT->totalProbeExact / TT->totalProbeFound * 100 << "% | ";
    cout << "Upper Probe Rate: " << (float)TT->totalProbeUpper / TT->totalProbeFound * 100 << "% | ";
    cout << "Lower Probe Rate: " << (float)TT->totalProbeLower / TT->totalProbeFound * 100 << "%}\n";
    cout << "\tNode set rate: " << (float)TT->totalNodesSet / TT->totalSetCalls * 100 << "% | ";
    cout << "{Overwrite proportion: " << (float)TT->totalOverwrittenNodesSet / TT->totalNodesSet * 100 << "% | ";
    cout << "Collision proportion: " << (float)TT->totalCollisionsSet / TT->totalNodesSet * 100 << "% | ";
    cout << "New node proportion: " << (float)TT->totalNewNodesSet / TT->totalNodesSet * 100 << "%}\n";
    cout << "\tReturned move validation rate: " << (float) TT->totalTTMovesInMoveList / TT->totalTTMovesFound * 100 << "%\n";
    SuperBoard.printBoardPrettily();
}
string getFENPrefix(SearchController SuperBoard) {
    string FENFlag = "";

    Move move = SuperBoard.getMoveHistory().back();
    SuperBoard.unMakeMove();
    MoveList moveList = SuperBoard.getMoveList();
    SuperBoard.makeMove(move);

    // * 2.
    short fromSq, toSq, promo, flag, fromPc, toPc;
    FENFlag = "-";
    decodeMove(move, fromSq, toSq, promo, flag, fromPc, toPc);
    for (Move m: moveList) {
        if (m == move) continue;

        short fromSq1, toSq1, promo1, flag1, fromPc1, toPc1;
        decodeMove(move, fromSq1, toSq1, promo1, flag1, fromPc1, toPc1);

        if (toSq == toSq1 && fromPc == fromPc1) {
            // get the files
            int file = fromSq % 8, file1 = fromSq1 % 8;
            int rank = fromSq / 8, rank1 = fromSq1 / 8;

            if (file != file1) {
                FENFlag = 'a' + file;
            } else if (rank != rank1) {
                FENFlag = '0' + (8 - rank);
            }
        }
    }

    return FENFlag;
}
void engineAgainstSelf(SearchController SuperBoard) {

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

    string MatchString = "";

    // run the game, and generate the PGN string
    Move m;
    string FENString;
    SearchResults results;
    while (true) {
        results = search(SuperBoard);
        if (!results.searchCompleted) break;

        SuperBoard.makeMove(results.bestMove);
        printSearchResults(results, SuperBoard);
        FENString = getFENPrefix(SuperBoard);

        if (SuperBoard.getMoveNumber() % 2 == 0) {
            MatchString += to_string(SuperBoard.getMoveNumber() / 2) + ". ";
        }

        MatchString += moveToFEN(m, FENString) + " ";
        if (SuperBoard.getMoveNumber() % 2 == 1) {
            MatchString += "\n";
        }

        if (!SuperBoard.validateZobrist()) {
            cout << "Oh no :( Zobrist error!? \n";
        }
    }

    // write the PGN string to the file
    ofstream gameFile;
    gameFile.open(logsPath + "games/game-" + to_string(num) + ".txt", ios::app);
    gameFile << MatchString;

    gameFile.close();
}
void debugMode(SearchParameters params) {
    SearchController SuperBoard(params);

    MoveList moves = SuperBoard.getMoveList();
    while (true) {
        cout << "\nWaiting for command: ";
        string command;
        cin >> command;
        if (command == "exit") {
            break;
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
            perftBoard.readFEN(FEN + FEN2);
            moves = SuperBoard.getMoveList();
        }  else if (command == "search") {
            string t;
            SearchResults results = search(SuperBoard);
            SuperBoard.makeMove(results.bestMove);
            printSearchResults(results, SuperBoard);
            moves = SuperBoard.getMoveList();
        } else if (command == "eval") {
            cout << "Board evaluation: " << SuperBoard.relativeLazy() << "\n";
        } else if (command == "play") {
            engineAgainstSelf(SuperBoard);
        } else if (command == "zobrist") {
            cout << "Current Zobrist key: " << SuperBoard.getZobristState() << "\n";
            cout << "Calculate Zobrist key: " << SuperBoard.calculateZobristHash() << "\n";
            cout << "All good: " << SuperBoard.validateZobrist() << "\n";
        } else if (command == "unmove") {
            SuperBoard.unMakeMove();
            moves = SuperBoard.getMoveList();
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
        } else if (command == "threefold") {
            cout << "Threefold?: " << SuperBoard.checkThreefold() << "\n";
        } else if (command == "test") {
            string move;
            cin >> move;
            cout << move << ".\n";
            printMoveBitboard(FENLongToMove(move));
        } else if (command == "movecode") {
            string fromS, toS;
            cout << "   From: ";
            cin >> fromS;
            cout << "   To: ";
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
        }
    }
}

void init() {
    initStaticMasks(); // used to create various masks
}

int main() {
    init();

    /* Set the search parameters */
    SearchParameters searchParams;
    searchParams.ttParameters.TTSizeMb = 99; // use a big TT

//    mainLoop(searchParams);

    debugMode(searchParams);

//    mainLoopUCI(searchParams);

    return 0;
}
