//
// Created by Noah Joubert on 2021-05-09.
//
#include "../misc.h"
#include "search.h"


int SearchController::negaMax(int alpha, int beta, int depth, Move &bestMove) {
    /* Negamax */
    /* How does it work?
     * 1. The depth counts down to 0. When at zero, we return the static evaluation, which is relative to the current player (positive = good, negative = bad).
     * 2. Probe the TT, if our search is deep enough
     * 3. We then generate moves, so we can check for checkmates/stalemates/three-folds. It returns a massive negative number in the case of check-mate (as it would be bad for the current player).
     * 4. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move. I use alpha beta pruning
         * a. Beta represents the maximum score that the minimising player is assured of. So if the evaluation is greater than beta, the minimising player won't take this path.
         * b. Alpha represents the minimum score that the maximising player is assured of. So if the evaluation is greater than alpha, this becomes new alpha!
     * 5. Store the search in the TT, if our search is deep enough.
     * 6. Return the score from the best move searched.
     * */

    searchedNodes++; // count the number of nodes searched
    int originalAlpha = alpha, originalBeta = beta; // store the original alpha/ beta so we can identify this node type

    // * 1.
    if (depth == 0) {
        // return static evaluation
        return evaluate();
    }

    // * 2.
    if (depth >= searchParameters.ttParameters.minTTProbeDepth) {
        bool nodeExists = false; // whether we've stored a search for this position
        TTNode *node = TT.probe(zobristState, nodeExists); // probe the table, if our search is deep enough
        if (nodeExists && node->depth >= depth) {
            // see if the node exists, and it is to a higher depth than the searchDepth

            // see what type of evaluation this is
            if (node->flag == EXACT_EVAL) {
                // if the evaluation is exact, check the move is valid, then return it

                // do a quick legality check on the move by seeing if it is in the move list, because of collision risk
                MoveList moves = getMoveList();
                if (find(moves.begin(), moves.end(), node->move) != moves.end()) {
                    bestMove = node->move;
                    return node->eval;
                }

            } else if (node->flag == LOWER_EVAL) {
                // this is a lower evaluation, meaning we've had a fail hard beta cut-off
                // so this evaluation is a lower bound of the node
                alpha = node->eval > alpha ? node->eval : alpha;

            } else if (node->flag == UPPER_EVAL) {
                // this is an upper evaluation, meaning we've not had any move exceed alpha
                // so this evaluation is an upper bound of the node
                beta = node->eval < beta ? node->eval : beta;
            }

            if (alpha >= beta) {
                // now check for an early fail high cutoff!
                bestMove = node->move;
                return node->eval; //TODO why do we return node->eval here?
            }
        }
    }

    // * 3.
    MoveList moves = getMoveList();
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return searchParameters.stalemateEvaluation;
    }

    // * 4.
    for (Move move: moves) {
        Move subBestMove = 0;

        // do a full depth search
        makeMove(move); // make the move
        int subEval = -negaMax(-beta, -alpha, depth - 1, subBestMove);
        unMakeMove(); // unmake the move

        // a. Fail hard beta cut off.
        if (subEval >= beta) {
            alpha = beta;
            break;
        }
        // b. New Best Move
        if (subEval > alpha) {
            alpha = subEval;
            bestMove = move;
        }
    }

    // * 5.
    if (depth >= searchParameters.ttParameters.minTTInsertDepth) {
        int evaluationType;
        if (alpha <= originalAlpha) {
            evaluationType = UPPER_EVAL;
        } else if (alpha >= originalBeta) {
            evaluationType = LOWER_EVAL;
        } else {
            evaluationType = EXACT_EVAL;
        }
        TT.set(zobristState, bestMove, depth, evaluationType, moveNumber, alpha);
    }

    // * 6.
    return alpha; // return the evaluation for the best move
}

bool SearchController::search(Move &bestMove, string &FENFlag, bool DEBUG_MODE) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 1. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
         * a. Run negamax
         * b. Check if the game has ended
         * c. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
         * d. Break out of the iterative deepening loop if we are in a depth spiral e.g. because there is only one legal move possible
     * 2. See if the move could have been done by another piece for PGN notation
     * 3. Make the best move
     * 4. Print out stats
     * */

    int eval = 0; // evaluation for this position
    float searchTime = 0; // time taken for the search
    int searchDepth = searchParameters.startingDepth; // the depth at which we search
    searchedNodes = 0; // the number of nodes searched
    TT.clearTotals(); // clear the totals from the transposition table

    // * 1.
    while (searchTime < searchParameters.minSearchTime) {
        Timer timer; // start the timer

        // * a. Run negamax
        int alpha = -INFINITY, beta = INFINITY;
        eval = negaMax(alpha, beta, searchDepth, bestMove); // run the search
        if (getCurrentSide() == BLACK) {
            eval *= -1;
        }

        // * b. Check if the game has ended
        getMoveList();
        if (inCheckMate() || inStalemate() || checkThreefold()) {
            cout << "game over";
            return false;
        }

        // * c. See how long the search was
        searchTime = timer.end(); // end the timer
        searchDepth = searchDepth + 1;

        // * d. This breaks the iterative deepening if whatever reason the search is super-duper quick i.e. there is only one more to play
        if (searchDepth > 50) {
            break;
        }
    }

    // * 2.
    short fromSq, toSq, promo, flag, fromPc, toPc;
    FENFlag = "-";
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

    // * 3.
    makeMove(bestMove);

    // * 4.
    if (DEBUG_MODE) {
        cout << "---------------------=+ Search Results " << moveNumber - 1 << ". +=---------------------\n";
        cout << "Results: \n";
        cout << "\tMove: " << moveToFEN(bestMove, "-") << " | ";
        cout << "Eval: " << eval << "\n";
        cout << "\tDepth: " << searchDepth << " | ";
        cout << "Time: " << searchTime << " | ";
        cout << "mNodes: " << (float) searchedNodes / 1000000 << " | ";
        cout << "mNodes per second: " << (float) searchedNodes / 1000000 / searchTime;
        cout << "\n";
        cout << "Transposition Table: \n";
        cout << "\tFill rate: " << (float)TT.totalUniqueNodes / TT.getSize() * 100 << "% | ";
        cout << "Absolute size: " << (float)TT.totalUniqueNodes * sizeof(TTNode) / 1000000 << "mb\n";
        cout << "\tProbe hit rate: " << (float)TT.totalProbeFound / TT.totalProbeCalls * 100 << "% | ";
        cout << "{Exact Probe Rate: " << (float)TT.totalProbeExact / TT.totalProbeFound * 100 << "% | ";
        cout << "Upper Probe Rate: " << (float)TT.totalProbeUpper / TT.totalProbeFound * 100 << "% | ";
        cout << "Lower Probe Rate: " << (float)TT.totalProbeLower / TT.totalProbeFound * 100 << "%}\n";
        cout << "\tNode set rate: " << (float)TT.totalNodesSet / TT.totalSetCalls * 100 << "% | ";
        cout << "{Overwrite proportion: " << (float)TT.totalOverwrittenNodesSet / TT.totalNodesSet * 100 << "% | ";
        cout << "Collision proportion: " << (float)TT.totalCollisionsSet / TT.totalNodesSet * 100 << "% | ";
        cout << "New node proportion: " << (float)TT.totalNewNodesSet / TT.totalNodesSet * 100 << "%}\n";
        printBoardPrettily();
    }

    return true;
}
