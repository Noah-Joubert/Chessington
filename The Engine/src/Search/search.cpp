//
// Created by Noah Joubert on 2021-05-09.
//
#include "../misc.h"
#include "../types.h"
#include "search.h"

int SearchController::quiescence(int alpha, int beta, int depth) {
    /* What is the Quiescence Search?
     * This is a special type of search which we enter at depth 0. It only considers captures. //TODO add in checks and promotions
     * This is to mitigate the horizon effect whereby the search stops just before a series of moves which drastically effect the game.
     * */

    /* 1. Take the static evaluation as a standPat score which represents the minimum evaluation for this node.
     * 2. Check for checkmate/ stalemate
     * 3. Loop through all moves and continue the quiescence search
     * */

    searchStats.totalNodesSearched++; // count the number of nodes searched
    searchStats.totalQuiescenceSearched ++; // count the quiescence nodes searched

    // * 1.
    // A quiescence search should always improve the board evaluation for the current player.
    // This is because the point of the quiescence search is to minimise the horizon effect, whereby the boards static evaluation is much less than the actual evaluation of the position.
    // So we take the standPat as a lower bound on our evaluation (alpha).
    int standPat = evaluate(); // we take the current static evaluation as a lower bound on the score
    if (standPat >= beta) {
        // fail hard
        return beta;
    } else if (alpha < standPat) {
        alpha = standPat;
    }
    
    // * 2.
    getMoveList(); // generate moves before checking for checkmate/ stalemate
    MoveList moves = activeMoveList;
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return searchParameters.stalemateEvaluation;
    }

    // * 3.
    int nodeEvaluation = -INFIN;
    for (Move move: moves) {
        // do a full depth search
        makeMove(move); // make the move
        int subEval = -quiescence(-beta, -alpha, depth - 1);
        unMakeMove(); // unmake the move

        // a. Fail low
        if (subEval > nodeEvaluation) {
            nodeEvaluation = subEval;
            if (nodeEvaluation > alpha) {
                alpha = nodeEvaluation;
            }
        }

        // b. Fail hard beta cut off.
        if (alpha >= beta) {
            alpha = beta; // TODO are we losing information by taking beta here?
            break;
        }
    }

    return alpha;
}
int SearchController::negaMax(int alpha, int beta, int depth, Move &bestMove) {
    /* Negamax */
    /* How does it work?
     * 1. The depth counts down to 0. At which point we enter the quiescence search
     * 2. Probe the TT, if our search is deep enough
     * 3. We then generate moves, so we can check for checkmates/stalemates/three-folds. It returns a massive negative number in the case of check-mate (as it would be bad for the current player).
     * 4. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move. I use alpha beta pruning
         * a. Beta represents the maximum score that the minimising player is assured of. So if the evaluation is greater than beta, the minimising player won't take this path.
         * b. Alpha represents the minimum score that the maximising player is assured of. So if the evaluation is greater than alpha, this becomes new alpha!
     * 5. Work out the alpha/beta evaluation type. Either we have hard failed high, in which case the evaluation is a lower bound - beta. Or we have failed low, so the evaluation is an upper bound - alpha.
     * 6. Store the search in the TT, if our search is deep enough.
     * 7. Return the score from the best move searched.
     * */

    searchStats.totalNodesSearched++; // count the number of nodes searched
    int originalAlpha = alpha, originalBeta = beta; // store the original alpha/ beta so we can identify this node type
    int nodeEvaluation = -INFIN;

    // * 1.
    if (depth == 0) {
        /* Go into quiescence search! */
        return quiescence(alpha, beta, depth);
    }

    // * 2.
    if (searchParameters.ttParameters.useTT) {
        bool nodeExists = false; // whether we've stored a search for this position
        TTNode *node = TT.probe(zobristState, nodeExists); // probe the table

        // see if the node exists, and it is to a higher depth than the searchDepth
        if (nodeExists && (node->depth >= depth)) {
            // TODO if the move is a LOWER_EVAL or EXACT_EVAL, add it to the front of the moveList,

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

        // a. Fail low
        if (subEval > nodeEvaluation) {
            nodeEvaluation = subEval;
            if (nodeEvaluation > alpha) {
                alpha = nodeEvaluation;
                bestMove = move;
            }
        }

        // b. Fail hard beta cut off.
        if (alpha >= beta) {
            alpha = beta; // TODO are we losing information by taking beta here?
            break;
        }
    }

    // * 5.
    /* Discussion: How should we treat each evaluation type?
     * First consider vanilla alpha/beta pruning. Until we fail high, every single move needs to be considered.
     * So fail high e.g. LOWER_EVAL nodes should be searched first. This leads into move ordering principles i.e. search captures first as they are more likely to fail high.
     * So for more ordering, we prioritise fail high, exact, then fail low. (LOWER_EVAL, EXACT_EVAL, UPPER_EVAL)
     * But for transposition tables entries fail high nodes are great as they raise alpha, but fail low nodes are also good as they lower beta. And both can lead to a cutoff
     * All in all, exact valuations are best
     * */
    int evaluationType;
    if (nodeEvaluation <= originalAlpha) {
        evaluationType = UPPER_EVAL;
    } else if (nodeEvaluation >= originalBeta) {
        evaluationType = LOWER_EVAL;
    } else {
        evaluationType = EXACT_EVAL;
    }

    // * 6.
    if (searchParameters.ttParameters.useTT && (depth >= searchParameters.ttParameters.minTTInsertDepth)) {
        TT.set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
    }

    // * 7.
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
    searchStats.clear();
    TT.clearTotals(); // clear the totals from the transposition table

    // * 1.
    while (searchTime < searchParameters.minSearchTime) {
        Timer timer; // start the timer

        // * a. Run negamax
        int alpha = -INFIN, beta = INFIN;
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
        cout << "mNodes: " << (float) searchStats.totalNodesSearched / 1000000 << " | ";
        cout << "Quiescence proportion: " << (float) searchStats.totalQuiescenceSearched / searchStats.totalNodesSearched * 100 << "% | ";
        cout << "mNodes per second: " << (float) searchStats.totalNodesSearched / 1000000 / searchTime;
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
