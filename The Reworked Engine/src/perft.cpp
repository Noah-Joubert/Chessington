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

        perft(currDepth + 1, ChessBoard, count, maxDepth);

        ChessBoard.unmakeMove();
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

        perft(1, ChessBoard, count, maxDepth);

        ChessBoard.unmakeMove();
    }

    mtx.lock(); // lock as it's accessed by multiple threads
    nodeCount += count; // add the count to the global count
    mtx.unlock();
}
void startPerft(Board &perftBoard, int depth, bool useThreads, int numThreads) {
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
}