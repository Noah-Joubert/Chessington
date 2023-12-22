#ifndef PERFT_CPP
#define PERFT_CPP

#include "Board/board.cpp"

/* Perft stuff. Don't need to touch this */
int perft(int currDepth, Board &ChessBoard, int maxDepth, bool debugMode) {
    int count = 0;
    if (currDepth == maxDepth - 1) {
        MoveList moves = ChessBoard.genMoves();

        count += moves.size();

        return count;
    }

    MoveList moves = ChessBoard.genMoves();

    for (Move move: moves) {

        ChessBoard.makeMove(move);
        count += perft(currDepth + 1, ChessBoard, maxDepth, false);
        ChessBoard.unmakeMove();

        if (currDepth == 0 && debugMode) {
            DecodedMove decodedMove(move);
            cout << SquareStrings[decodedMove.from] << SquareStrings[decodedMove.to] << ": " << count << "\n";
        }
    }

    return count;
}

#endif