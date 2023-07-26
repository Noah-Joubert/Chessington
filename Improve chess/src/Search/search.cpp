//
// Created by Noah Joubert on 2021-05-09.
//
#include "SearchState.cpp"
#include "../misc.cpp"
#include "search.h"

int negaMax(SearchState &Board, int depth, long int &nodeCount, Move &bestMove) {
    /* Negamax */
    /* How does it work?
     * 1. The depth counts down to 0. When at zero, we return the static evaluation, which is relative to the current player (positive = good, negative = bad).
     * 2. We then generate moves, so we can check for checkmates/stalemates/three-folds. It returns a massive negative number in the case of check-mate (as it would be bad for the current player).
     * 3. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move.
     * 4. Return the score from the best move searched.
     * */

    nodeCount++; // count the number of nodes searched

    // 1. if at the end of the tree, return static evaluation
    if (depth == 0) {
        // return static evaluation
        return Board.evaluate();
    }


    // 2. generate all the moves ~ this needs to be done before checking for inCheckMate/ inStalemate
    MoveList moves = Board.getMoveList();
    if (Board.inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (Board.inStalemate() | Board.checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return STALEMATE;
    }

    // 3. now loop through all the moves
    Move subBestMove = 0; // stores the best move within the search
    int bestEval = -INFINITY; // stores the evaluation of the best move
    for (Move move: moves) {
        // do a full depth search
        Board.makeMove(move); // make the move
        int subEval = -negaMax(Board, depth - 1, nodeCount, subBestMove);
        Board.unMakeMove(); // unmake the move

        if (subEval > bestEval) {
            bestEval = subEval;
            bestMove = move;
        }
    }

    // 4.
    return bestEval; // return the evaluation for the best move
}

bool search(SearchState &SuperBoard, Move &bestMove, char &twice) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 1. Run negamax
     * 2. Check if the game has ended
     * 3. Make the best move
     * 4. Print out stats
     * */

    int eval = 0; // time taken for the evaluation

    searchDepth = 4;

    // 1.
    eval = negaMax(SuperBoard, searchDepth, negaNodes, bestMove); // run the search
    if (SuperBoard.getCurrentSide() == BLACK) {
        eval *= -1;
    }

    // 2.
    SuperBoard.getMoveList();
    if (SuperBoard.inCheckMate() || SuperBoard.inStalemate() || SuperBoard.checkThreefold()) {
        cout << "Game over.\n";
        return false;
    }

    // 3.
    SuperBoard.makeMove(bestMove);

    // 4.
    SuperBoard.printBoardPrettily();
    cout << "Eval: " << eval << " | " << "Nodes: " << negaNodes << "\n";
    printMoveBitboard(bestMove);

    return true;
}
