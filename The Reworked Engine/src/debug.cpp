#include "perft.cpp"

class Timer {
    chrono::time_point<chrono::high_resolution_clock> startTime, endTime;
public:
    Timer() {
        startTime = chrono::high_resolution_clock::now();
    }
    float end() {
        endTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = endTime - startTime;
        return elapsed.count();
    }
};

void debugMoveGeneration() {
    Board board;
    while (true) {
        string command;
        cout << "Enter command (exit, perft, move, print, moves, fen): ";
        cin >> command;

        if (command == "exit") {
            break;
        } else if (command == "perft") {
            int depth;
            cout << "\tEnter depth: ";
            cin >> depth;


            auto start = chrono::high_resolution_clock::now();
            int count = perft(0, board, depth, true);
            auto finish = chrono::high_resolution_clock::now();

            double timeSpent = (finish - start).count();
            cout << "\tNodes per second: " << count / timeSpent << "\n";
            cout << "\tTime: " << timeSpent << "\n";
            cout << "\tNodes: " << count << "\n";
        } else if (command == "move") {
            string fromS, toS;
            cout << "\tFrom: ";
            cin >> fromS;
            cout << "\tTo: ";
            cin >> toS;

            short from, to;
            for (int i = 0; i < 64; i++) {
                if (fromS == SquareStrings[i]) from = i;
                if (toS == SquareStrings[i]) to = i;
            }
            for (Move move: board.genMoves()) {
                DecodedMove decodedMove(move);
                if (decodedMove.from == from && to == decodedMove.to) {
                    board.makeMove(move);
                }
            }
        } else if (command == "print") {
            board.debugPrint();
        } else if (command == "unmove") {
            board.unmakeMove();
        } else if (command == "fen") {
            string FEN, FEN2;
            cout << "       Enter FEN: ";
            cin >> FEN;
            getline(cin, FEN2);
            board.readFEN(FEN + FEN2);
        } else if (command == "test") {
            printBitboard(board.getBitboards().getSideBB(WHITE));
            printBitboard(board.getBitboards().getSideBB(BLACK));
            printBitboard(board.getBitboards().getPieceBB(PAWN));
        }
    }
}
void testMoveGenerationSpeed() {
    Board board;

    Timer t;
    long nodeCount = perft(0, board, 6, false);
    double elapsedTime = t.end();

    cout << "Nodes per second: " << nodeCount / elapsedTime << "\n";
    cout << "Time: " << elapsedTime << "\n";
    cout << "Nodes: " << nodeCount << "\n";
}