//
// Created by Noah Joubert on 03/09/2023.
//

#include <iostream>
#include <string>
#include <sstream>
#include "Search/SearchController.cpp"

/* This code implements the UCI chess engine communication protocol */
using namespace std;

bool useDebugMode = false;

SearchParameters _s;
SearchController UCIBoard(_s);

template <typename T> T popFront(vector<T> &vec) {
    if (vec.empty()) return T();

    T toReturn = vec.front();
    vec.erase(vec.begin());
    return toReturn;
}
vector<string> commandToVector(string inputString) {
    // takes a command eg. "debug     mode" and returns a vector that ignores spaces eg. ["debug", "mode"]
    istringstream stream(inputString);
    string command;
    vector<string> commands;

    while (stream >> command) {
        commands.push_back(command);
    }

    return commands;
}
vector<string> getUserInput() {
    // get user input
    string input;

    getline(cin, input);

    vector commands = commandToVector(input);

    if (useDebugMode) {
        cout << "[";

        int numCommands = commands.size();
        for (int i = 0; i < numCommands - 1; i ++) {
            cout << commands[i] << ",";
        }

        cout << commands[numCommands - 1];

        cout << "]\n";
    }

    return commands;
}
void sendCommandString(string s) {
    cout << s << "\n";
}
string moveToFENLong(Move m) {
    short fromSq, toSq, promo, flag, fromPc, toPc;
    decodeMove(m, fromSq, toSq, promo, flag, fromPc, toPc);

    string moveString;

    if (flag == CASTLING) {
        // castling
        short newKing, newRook;
        getCastleSquares(toBB(toSq), newRook, newKing);

        moveString = SquareStrings[fromSq] + SquareStrings[newKing];
    } else {
        // normal move
        moveString = SquareStrings[fromSq] + SquareStrings[toSq];

        // see if we are promoting
        if (flag == PROMOTION) {
            moveString += tolower(getPieceString(getPromoPiece(promo))[0]);
        }
    }

    return moveString;
}
Move FENLongToMove(string s) {
    // get the too and from squares
    string fromString = s.substr(0, 2);
    string toString = s.substr(2, 2);

    // find what square number they correspond to
    short from, to;
    for (int i = 0; i < 64; i++) {
        if (fromString == SquareStrings[i]) {from = i;}
        if (toString == SquareStrings[i]) {to = i;}
    }

    // see what move it is
    MoveList moves = UCIBoard.getMoveList();
    for (Move move: moves) {
        short f, t, promo, flag, fromType, toType;
        decodeMove(move, f, t, promo, flag, fromType, toType);
        if ((f == from) && (t == to)) {
            return move;
        }


        // castling is encoded slightly differently internally. check for that here
        if ((flag == CASTLING) && (from == f)
            && (
                    ((t == A1) && (to == C1)) || ((t == A8) && (to == C8)) ||
                    ((t == H1) && (to == G1)) || ((t == H8) && (to == G8))
                    )
            ) {
            return move;
        }
    }

    assert(true); // uh oh!
}

/* Outputs */
void id() {
    sendCommandString("id name Chessington author Noah Joubert");
}
void option() {
    //TODO

}
void uciok(){
    sendCommandString("uciok");
}
void readyok() {
    sendCommandString("readyok");
}
void bestmove(string FEN) {
    sendCommandString("bestmove " + FEN);
}

/* Inputs */
void uci() {
    id();
    option();
    uciok();
}
void isready() {
    readyok();
}
void debug(vector<string> &commandQueue) {
    // toggles debug mode on or off
    string command = popFront(commandQueue);

    if (command == "on") {
        useDebugMode = true;
        return;
    } else if (command == "off") {
        useDebugMode = false;
        return;
    }

}
void setoption(vector<string> &commandQueue) {
    // used to set an option for the engine


    // get the name command
    string __ = popFront(commandQueue);

    //TODO this
}
void _register(vector<string> &commandQueue) {

}
void ucinewgame(vector<string> &commandQueue) {

}
void position(vector<string> &commandQueue) {
    string command = popFront(commandQueue);

    // first set the position
    if (command == "startpos") {
        // we are starting from the initial position
        UCIBoard.readFEN(initialFEN);
    } else if (command == "fen") {
        // we are taking a fen input

        static int FENStringComponents = 4; // an FEN string is made of 6 components

        int numFENInputs = 0;
        string FEN = "";
        for (int i = 0; i < FENStringComponents; i ++) {
            if (commandQueue.empty()) {
                break;
            }

            numFENInputs ++;
            FEN += popFront(commandQueue) + " ";
        }

        if (numFENInputs == FENStringComponents) {
            UCIBoard.readFEN(FEN);
        }
    }

    // now make any moves that are left over
    if (commandQueue.empty()) return;
    popFront(commandQueue); // pop the 'move' command

    while (!commandQueue.empty()) {
        UCIBoard.makeMove(FENLongToMove(popFront(commandQueue)));
    }
}
void go(vector<string> &commandQueue) {

    while (!commandQueue.empty()) {
        string command = popFront(commandQueue);

        if (command == "searchmoves") {
            // TODO this
        } else if (command == "ponder") {
            // TODO this
        } else if (command == "depth") {
            // TODO this as well i guess
        }
    }

    Move bestMove;
    string twice;

    //TODO yeah
//    UCIBoard.search(bestMove, twice, false);
    string fen = moveToFENLong(bestMove);

    bestmove(fen);
}

void mainLoopUCI(SearchParameters searchParams) {
    UCIBoard = SearchController(searchParams);

    /* Now go into the mainloop */
    bool mainLoopRunning = true;
    vector<string> commandQueue;

    enum Stage {
        startUp = 0, mainLoop = 1
    };
    Stage stage = startUp;

    while (mainLoopRunning) {
        /* We receive commands from the user, and go through them one at a time */
        if (commandQueue.empty()) {
            commandQueue = getUserInput();
        }

        string command = popFront(commandQueue);

        if (command == "uci") {
            uci();
        } else if (command == "isready") {
            stage = mainLoop;
            isready();
        } else if (command == "setoption") {
            stage = mainLoop;
        } else if (command == "quit") {
            mainLoopRunning = false;
            break;
        }

        /* We must receive certain commands to initiate the engine */
        if (stage == startUp) {
            continue;
        }

        if (command == "debug") {
            debug(commandQueue);
        } else if (command == "setoption") {
            setoption(commandQueue);
        } else if (command == "register") {
            _register(commandQueue);
        } else if (command == "ucinewgame") {
            ucinewgame(commandQueue);
        } else if (command == "position") {
            position(commandQueue);
        } else if (command == "go") {
            go(commandQueue);
        }

        // my custom commands
        if (command == "print") {
            UCIBoard.printBoardPrettily();
        }
    }
}