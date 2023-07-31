//
// Created by Noah Joubert on 29/07/2023.
//
#include "Search/SearchController.cpp"

double moveTimer = 0;
long long int nodeCount = 0;
int numThreads = 5;
bool useThreads = true;
bool parallel = true;
bool useLogs = true;
int maxDepth = 6;

SearchParameters perftParams; // the default values for SearchParameters are all zero. This is suitable for a perft instance
SearchController perftBoard(perftParams);
mutex mtx;

/* Perft stuff. Don't need to touch this */
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
void reccursiveMoveCheck(int depth, SearchController &ChessBoard, int &count) {
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
void startThread(MoveList moves, SearchController ChessBoard) {
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

    MoveList moves = perftBoard.getMoveList();
    vector<MoveList> moveLists;
    vector<SearchController> positions;
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
                thread t(startThread, mList, perftBoard);
                t.join();
                cout << "\n\tThread " << i << ": " << nodeCount << "";
            }
            cout << "\n}\n";
        } else {
            vector<thread> threads;
            for (MoveList mList: moveLists) {
                threads.emplace_back(thread(startThread, mList, perftBoard));
            }

            for (thread &T: threads) {
                T.join();
            }
        }
    } else {
        startThread(moves, perftBoard);
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