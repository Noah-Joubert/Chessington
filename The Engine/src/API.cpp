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

        if (command == "fen") {
            string FEN = getStringInput("");
            SuperBoard.readFEN(FEN);

        } else if (command == "move") {
            string moveNumber = getStringInput("");
            Move move = atoi(moveNumber.c_str());
            SuperBoard.makeMove(move);
        } else if (command == "search") {
            Move move;
            string t;
            SuperBoard.search(move, t, false);
            cout << move << "\n";
        } else if (command == "print") {
            SuperBoard.printBoardPrettily();
        } else if (command == "status") {
            SuperBoard.getMoveList();
            cout << !(SuperBoard.checkThreefold() || SuperBoard.inStalemate() || SuperBoard.inCheckMate()) << "\n";
        } else if (command == "end") {
            return;
        }
    }
}