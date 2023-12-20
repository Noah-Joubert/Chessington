//
// Created by Noah Joubert on 29/07/2023.
//
#include "Board/board.cpp"
#include <thread>
#include <fstream>

mutex mtx;

long int nodeCount = 0;

/* Perft stuff. Don't need to touch this */
void splitMoveList(MoveList moves, vector<MoveList> &moveLists, int numThreads) {
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
void perft(int currDepth, Board &ChessBoard, int &count, int maxDepth) {
    if (currDepth == maxDepth - 1) {
        MoveList moves = ChessBoard.genMoves();

        count += moves.size();

        return;
    }

    MoveList moves = ChessBoard.genMoves();

    for (Move move: moves) {

        ChessBoard.makeMove(move);

        int subCount = 0;
        perft(currDepth + 1, ChessBoard, subCount, maxDepth);
        count += subCount;

        ChessBoard.unmakeMove();

        if (currDepth == 0) {
            DecodedMove decodedMove(move);
            cout << SquareStrings[decodedMove.from] << SquareStrings[decodedMove.to] << ": " << subCount << "\n";
        }
    }
}
void startThread(MoveList moves, Board ChessBoard, int maxDepth) {
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

        int subCount = 0;
        perft(1, ChessBoard, subCount, maxDepth);
        count += subCount;

        ChessBoard.unmakeMove();
    }

    mtx.lock(); // lock as it's accessed by multiple threads
    nodeCount += count; // add the count to the global count
    mtx.unlock();
}
long recursivePerft(Board &perftBoard, int depth, bool useThreads, int numThreads) {
    double timeSpent;
    nodeCount = 0;

    auto start = chrono::high_resolution_clock::now();

    MoveList moves = perftBoard.genMoves();
    vector<MoveList> moveLists;
    vector<Board> positions;
    vector<int> counts; // stores the counts for each sub move

    if (useThreads) {
        /* split the move list evenly */
        splitMoveList(moves, moveLists, numThreads);

        /* create the threads */
        vector<thread> threads;
        for (MoveList mList: moveLists) {
            threads.emplace_back(thread(startThread, mList, perftBoard, depth));
        }

        for (thread &T: threads) {
            T.join();
        }
    } else {
        startThread(moves, perftBoard, depth);
    }

    auto finish = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = finish - start;
    timeSpent = elapsed.count();

    // write the result to file
    string fileName ;
    if (useThreads) {
        fileName = "/Users/Noah/CLionProjects/Improve chess/logs/multi-thread.txt";
    } else {
        fileName = "/Users/Noah/CLionProjects/Improve chess/logs/single-thread.txt";
    }

    // open, write, close the file
    ofstream File;
    File.open(fileName, ios::app);
    File << "\n" << nodeCount << " " << timeSpent;
    File.close();

    cout << "Nodes per second: " << nodeCount / timeSpent << "\n";
    cout << "Time: " << timeSpent << "\n";
    cout << "Nodes: " << nodeCount << "\n";

    return nodeCount;
}
void debugPerft(Board &perftBoard) {
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

            // run the perft
            int count = 0;

            auto start = chrono::high_resolution_clock::now();
            perft(0, perftBoard, count, depth);
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
            for (Move move: perftBoard.genMoves()) {
                DecodedMove decodedMove(move);
                if (decodedMove.from == from && to == decodedMove.to) {
                    perftBoard.makeMove(move);
                }
            }
        } else if (command == "print") {
            perftBoard.debugPrint();
        } else if (command == "unmove") {
            perftBoard.unmakeMove();
        } else if (command == "fen") {
            string FEN, FEN2;
            cout << "       Enter FEN: ";
            cin >> FEN;
            getline(cin, FEN2);
            perftBoard.readFEN(FEN + FEN2);
        } else if (command == "test") {
            printBitboard(perftBoard.getBitboards().getSideBB(WHITE));
            printBitboard(perftBoard.getBitboards().getSideBB(BLACK));
            printBitboard(perftBoard.getBitboards().getPieceBB(PAWN));
        }
    }
}