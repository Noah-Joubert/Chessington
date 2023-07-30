//
// Created by Noah Joubert on 29/07/2023.
//

#include "Search/SearchController.cpp"

string getStringInput(string promptText) {
    // get user input
    cout << promptText;
    string input;
    cin >> input;
    return input;
}

/* The main commandline loop */
void mainLoop() {
    SearchController SuperBoard;

    while (true) {
        string command = getStringInput("");

        if (command == "help") {
            cout << "- fen\n- move\n";
        } else if (command == "fen") {
            string FEN = getStringInput("\nEnter FEN String: ");
            SuperBoard.readFEN(FEN);
            cout << "\tFEN read successfully.";


        } else if (command == "move") {
            string moveNumber = getStringInput("");
            Move move = atoi(moveNumber.c_str());
            SuperBoard.makeMove(move);
        } else if (command == "search") {
            Move move;
            char t;
            SuperBoard.search(move, t);
            cout << move << "\n";
        } else if (command == "end") {
            SuperBoard.printBoardPrettily();
            return;
        } else if (command == "status") {
            SuperBoard.getMoveList();
            cout << !(SuperBoard.checkThreefold() || SuperBoard.inStalemate() || SuperBoard.inCheckMate()) << "\n";
        }
    }
}