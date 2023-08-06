#include <iostream>
#include <chrono>  // for high_resolution_clock
#include <string>
#include <iomanip>
#include <thread>
#include <fstream>
#include "Search/SearchController.cpp"
#include "API.cpp"
#include "perft.cpp"

using namespace std;
const string logsPath = getCWDString() + "/logs/";

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
    while (SuperBoard.search(m, FENString, true)) {
        if (SuperBoard.getMoveNumber() % 2 == 0) {
            MatchString += to_string(SuperBoard.getMoveNumber() / 2) + ". ";
        }

        MatchString += moveToFEN(m, FENString) + " ";
        if (SuperBoard.getMoveNumber() % 2 == 1) {
            MatchString += "\n";
        }

        if (!SuperBoard.validateZobrist()) {
            cout << "Oh no :( \n";
            break;
        }

//        if (SuperBoard.getMoveNumber() > 100) break; // break if the game goes on too long
    };

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
            Move m;
            string t;
            SuperBoard.search(m, t, true);
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
            string number;
            cin >> number;
            cout << SuperBoard.
            SEE(stoi(number));
        }
    }
}

void init() {
    initStaticMasks(); // used to create various masks
}

int main() {
    /* computer has two cores
     * two threads for each core
     * therefore 4 threads all together*/
    init();

    /* Set the search parameters */
    SearchParameters searchParams;
    searchParams.ttParameters.TTSizeMb = 100; // use a big TT
    searchParams.minSearchTime = 0.01;

    mainLoop(searchParams);

//    debugMode(searchParams);

    return 0;
}
