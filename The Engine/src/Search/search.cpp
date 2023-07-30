//
// Created by Noah Joubert on 2021-05-09.
//
#include "../misc.cpp"
#include "search.h"

#define STALEMATE 1000

int SearchController::negaMax(int alpha, int beta, int depth, Move &bestMove) {
    /* Negamax */
    /* How does it work?
     * 1. The depth counts down to 0. When at zero, we return the static evaluation, which is relative to the current player (positive = good, negative = bad).
     * 2. We then generate moves, so we can check for checkmates/stalemates/three-folds. It returns a massive negative number in the case of check-mate (as it would be bad for the current player).
     * 3. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move. I use alpha beta pruning
     * 3. a. Beta represents the maximum score that the minimising player is assured of. So if the evaluation is greater than beta, the minimising player won't take this path.
     * 3. b. Alpha represents the minimum score that the maximising player is assured of. So if the evaluation is greater than alpha, this becomes new alpha!
     * 4. Return the score from the best move searched.
     * */

    searchedNodes++; // count the number of nodes searched

    // 1. if at the end of the tree, return static evaluation
    if (depth == 0) {
        // return static evaluation
        return evaluate();
    }


    // 2. generate all the moves ~ this needs to be done before checking for inCheckMate/ inStalemate
    MoveList moves = getMoveList();
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return STALEMATE;
    }

    // 3. now loop through all the moves
    Move subBestMove = 0; // stores the best move within the search
    for (Move move: moves) {
        // do a full depth search
        makeMove(move); // make the move
        int subEval = -negaMax(-beta, -alpha, depth - 1, subBestMove);
        unMakeMove(); // unmake the move

        // 3. a. Fail hard beta cut off.
        if (subEval >= beta) {
            return beta;
        }
        // 3. b. New Best Move
        if (subEval > alpha) {
            alpha = subEval;
            bestMove = move;
        }
    }

    // 4.
    return alpha; // return the evaluation for the best move
}

bool SearchController::search(Move &bestMove, char &FENFlag) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 1. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
     * 2. Run negamax
     * 3. Check if the game has ended
     * 4. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
     * 5. See if the move could have been done by another piece for PGN notation
     * 6. Make the best move
     * 7. Print out stats
     * */

    int eval = 0; // evaluation for this position
    float searchTime = 0; // time taken for the search
    int searchDepth = startingDepth; // the depth at which we search
    searchedNodes = 0; // the number of nodes searched

    // 1.
    while (searchTime < minSearchTime) {
        Timer timer; // start the timer

        // 2.
        int alpha = -INFINITY, beta = INFINITY;
        eval = negaMax(alpha, beta, searchDepth, bestMove); // run the search
        if (getCurrentSide() == BLACK) {
            eval *= -1;
        }

        // 3.
        getMoveList();
        if (inCheckMate() || inStalemate() || checkThreefold()) {
            cout << "game over";
            return false;
        }

        // 4.
        searchTime = timer.end(); // end the timer
        searchDepth = searchDepth + 1;

        // this breaks the iterative deepening if whatever reason the search is super duper quick.
        if (searchDepth > 50) {
            break;
        }
    }

    // see if the move could have been done by another piece for PGN notation
    short fromSq, toSq, promo, flag, fromPc, toPc;
    decodeMove(bestMove, fromSq, toSq, promo, flag, fromPc, toPc);
    for (Move move: getMoveList()) {
        if (move == bestMove) continue;

        short fromSq1, toSq1, promo1, flag1, fromPc1, toPc1;
        decodeMove(move, fromSq1, toSq1, promo1, flag1, fromPc1, toPc1);

        if (toSq == toSq1 && fromPc == fromPc1) {
            // get the files
            int file = fromSq % 8, file1 = fromSq1 % 8;
            int rank = fromSq / 8, rank1 = fromSq1 / 8;

            if (file != file1) {
                FENFlag = 'a' + file;
            } else if (rank != rank1) {
                FENFlag = '0' + (8 - rank);
            }
        }
    }

    // 6.
    makeMove(bestMove);

    // 7.
    const bool DEBUG_MODE = false;
    if (DEBUG_MODE) {
        cout << "---------------------=+ Search Results " << moveNumber - 1 << ". +=---------------------\n";
        cout << "Move: " << moveToFEN(bestMove, '-') << " | ";
        cout << "Eval: " << eval << "\n";
        cout << "Depth: " << searchDepth << " | ";
        cout << "Time: " << searchTime << " | ";
        cout << "mNodes: " << (float) searchedNodes / 1000000 << " | ";
        cout << "mNodes per second: " << (float) searchedNodes / 1000000 / searchTime;
        cout << "\n";
        printBoardPrettily();
    }


    return true;
}
