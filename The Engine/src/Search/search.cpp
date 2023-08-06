//
// Created by Noah Joubert on 2021-05-09.
//
#include "../misc.h"
#include "../types.h"
#include "search.h"
#include "SearchController.h"

int SearchController::SEEMove(Move m) {
    // evaluate the SEE of a move. we assume the move has already been made, and will me immediately unmade

    // decode the move
    short fromSq, toSq, promo, flag, fromPc, toPc;
    decodeMove(m, fromSq, toSq, promo, flag, fromPc, toPc);

    int SEEEval = PieceScores[toPc] - SEE(toSq);

    return SEEEval;
}
int SearchController::SEE(short square) {
    /* Static exchange evaluation
     * It returns the value of all the captures on one square.
     * We always capture with the cheapest piece first.
     * */
    int SEEValue = 0;

    // get the smallest attacker on the square (this actually doesn't include en-passants (or promotions). honestly I can't be asked)
    short smallestAttackerSquare;
    short smallestAttacker = -1;
    getSmallestAttacker(square, smallestAttackerSquare, smallestAttacker);

    // get the enemy piece
    U64 to = toBB(square);
    short toPiece = getPieceAt(to);

    // if there are no more attackers, return 0
    if (smallestAttackerSquare != -1) {
        /* Manually do the move */
        setSquare(smallestAttacker, currentSide, square); // move the taking piece
        setSquare(smallestAttacker, currentSide, smallestAttackerSquare); // move the taking piece
        setSquare(toPiece, otherSide, square); // remove the taken piece
        switchSide();

        SEEValue = PieceWorths[toPiece] - SEE(square);

        /* Manually do the move */
        switchSide();
        setSquare(smallestAttacker, currentSide, square); // move the taking piece
        setSquare(smallestAttacker, currentSide, smallestAttackerSquare); // move the taking piece
        setSquare(toPiece, otherSide, square); // remove the taken piece
    }

    return SEEValue;
}

int SearchController::quiescence(int alpha, int beta, int depth) {
    /* What is the Quiescence Search?
     * This is a special type of search which we enter at depth 0. It only considers captures.
     * This is to mitigate the horizon effect whereby the search stops just before a series of moves which drastically effect the game.
     * It tends to take up at least 50% of the search, so definitely worth some thought.
     * */

    /* 1. Take the static evaluation as a standPat score which represents the minimum evaluation for this node.
     * 2. Probe the TT
     * 3. Check for checkmate/ stalemate
     * 4. Loop through all moves and continue the quiescence search
        * a. SEE pruning. Static Exchange Evaluation gives the value of exchange of material on particular square.
     * 5. Write to the TT
     * */

    //TODO add in pawn checks
    //TODO consider limiting quiet checks

    int originalAlpha = alpha, originalBeta = beta; // store the original alpha/ beta so we can identify this node type

    searchStats.totalNodesSearched++; // count the number of nodes searched
    searchStats.totalQuiescenceSearched ++; // count the quiescence nodes searched

    Move bestMove; // best move from this node

    // * 1.
    // A quiescence search should always improve the board evaluation for the current player.
    // This is because the point of the quiescence search is to minimise the horizon effect, whereby the boards static evaluation is much less than the actual evaluation of the position.
    // So we take the standPat as a lower bound on our evaluation (alpha).
    int standPat = relativeLazy(); // we take the current static evaluation as a lower bound on the score
    if (standPat >= beta) {
        // fail hard
        return standPat;
    }

    // else finally take standard pat as alpha
    if (alpha < standPat) {
        alpha = standPat;
    }

    // * 2. Probe the TT
    if (searchParameters.ttParameters.useTTInQSearch) {
        bool useNode = false; // whether we've stored a search for this position
        TTNode *node = TT.probe(zobristState, useNode, alpha, beta, depth); // probe the table

        // see if the node exists, and it is to a higher depth than the searchDepth
        if (useNode) {
            // TODO if the move is a BETA_EVAL or EXACT_EVAL, add it to the front of the moveList,
            bestMove = node->move;
            return node->flag == EXACT_EVAL ? node->eval: node->flag == ALPHA_EVAL ? alpha : beta;
        }
    }

    // * 3.
    getMoveList(); // annoyingly we have to generate all moves before checking for checkmate/ stalemate
    MoveList moves = activeMoveList;
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return searchParameters.stalemateEvaluation;
    }

    // * 4.
    int nodeEvaluation = standPat; // need to set as standPat not -infinity, in-case there are no moves (i.e. leaf node)
    for (Move move: moves) {
        if (!(move & toTypeMask)) {
            // see if this is a non-capture quiescence move
            searchStats.totalNonCaptureQSearched ++;

            // if we are deep enough/ stop making these moves, unless promotion //TODO standardise
//            if ((depth <= -1) && !(move & promoMask)) {
//                continue;
//            }
        }

        // do a full depth search
        makeMove(move); // make the move before doing SEE

        /* a. SSE pruning/ Delta pruning
         * As a quiescence search should improve the nodes' evaluation, we can confidently prune nodes with negative SEE.
         * We use this together with delta pruning, where we prune a move if we don't believe it can raise alpha
         * i.e. lazyEval() + SEE + Margin < alpha.
         * */
        int SEEEvaluation = SEEMove(moveHistory.back());
        if (
                (searchParameters.useSEE && (SEEEvaluation < 0)) ||
                (searchParameters.useDelta && (standPat + PieceWorths[getTooPiece(move)] + searchParameters.deltaMargin < alpha))
                ) {
            unMakeMove();
            continue;
        }

        int subEval = -quiescence(-beta, -alpha, depth - 1);
        unMakeMove(); // unmake the move

        if (subEval > nodeEvaluation) {
            nodeEvaluation = subEval;
            bestMove = move;
            if (nodeEvaluation > alpha) {
                alpha = nodeEvaluation;
            }
        }

        // b. Fail hard beta cut off.
        if (alpha >= beta) {
            break;
        }
    }

    // * 5. write to the TT
    int evaluationType = getEvaluationType(nodeEvaluation, originalAlpha, originalBeta);
    if (searchParameters.ttParameters.useTTInQSearch && (depth >= searchParameters.ttParameters.minTTInsertDepth)) {
        TT.set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
    }

    return nodeEvaluation; // return nodeEvaluation as we use a fail soft alpha beta framework
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
    int nodeEvaluation = -INFIN; // stores the calculated evaluation of this node, as we use Fail Soft alpha/beta framework

    // * 1.
    if (depth == 0) {
        /* Go into quiescence search! */
        if (searchParameters.useQuiescence) {return quiescence(alpha, beta, depth);}
        else {return evaluate();}
    }

    // * 2.
    bool ttEvaluationFound = false;
    Move ttMove;
    int ttEval;
    if (searchParameters.ttParameters.useTT) {
        bool shouldUse = false; // whether we will use the TT probe results
        TTNode *node = TT.probe(zobristState, shouldUse, alpha, beta, depth); // probe the table

        // see if the node exists, and it is to a higher depth than the searchDepth
        if (shouldUse) {
            // TODO if the move is a BETA_EVAL or EXACT_EVAL, add it to the front of the moveList,
            bestMove = node->move;
            return node->flag == EXACT_EVAL ? node->eval: node->flag == ALPHA_EVAL ? alpha : beta;
        }
    }

    // * 3.
    MoveList moves = getMoveList(); // generate moves before checking for checkmate/ stalemate
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
            break;
        }
    }

    // * 5.
    /* Discussion: How should we treat each evaluation type?
     * First consider vanilla alpha/beta pruning. Until we fail high, every single move needs to be considered.
     * So fail high e.g. BETA_EVAL nodes should be searched first. This leads into move ordering principles i.e. search captures first as they are more likely to fail high.
     * So for more ordering, we prioritise fail high, exact, then fail low. (BETA_EVAL, EXACT_EVAL, ALPHA_EVAL)
     * But for transposition tables entries fail high nodes are great as they raise alpha, but fail low nodes are also good as they lower beta. And both can lead to a cutoff
     * All in all, exact valuations are best
     * */
    int evaluationType = getEvaluationType(nodeEvaluation, originalAlpha, originalBeta);

    // * 6.
    if (searchParameters.ttParameters.useTT &&
            (depth >= searchParameters.ttParameters.minTTInsertDepth)
    ) {
        TT.set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
    }

    if (ttEvaluationFound) {
        bool evaluationAsExpected = ((nodeEvaluation == ttEval) && (evaluationType == EXACT_EVAL)) + ((nodeEvaluation <= ttEval) && (evaluationType == BETA_EVAL)) + ((nodeEvaluation >= ttEval) && (evaluationType == ALPHA_EVAL));
        bool moveAsExpected = !(evaluationType == EXACT_EVAL) || bestMove == ttMove;

        if (!(moveAsExpected && evaluationAsExpected)) {

            // If we had an exact TT match, print out the two searches
            cout << "\n";
            cout << "Search results: Eval " << nodeEvaluation << " | Evaluation Type: "
                 << evaltypeToString(evaluationType) << "\n";
            cout << "TT results: Eval " << ttEval << "\n";
            cout << "Same move: " << (bestMove == ttMove) << "\n";
            cout << "As expected? " << ((nodeEvaluation == ttEval) && (evaluationType == EXACT_EVAL)) +
                                       ((nodeEvaluation <= ttEval) && (evaluationType == BETA_EVAL)) +
                                       ((nodeEvaluation >= ttEval) && (evaluationType == ALPHA_EVAL));
            cout << "\n";
        }
    }

    // * 7.
    return nodeEvaluation; // return nodeEvaluation as we use a fail soft alpha beta framework
}

bool SearchController::search(Move &bestMove, string &FENFlag, bool DEBUG_MODE) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 1. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
         * a. Run negamax
         * b. Check if the game has ended
         * c. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
         * d. See if we must break out of iterative deepening
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

        // * d. This breaks the iterative deepening
        // Firstly if the search is to a crazy depth, something is going wrong. Secondly, if a mate is found
        if ((searchDepth > 50) || (abs(eval) >= MATE)) {
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
        cout << "Time: " << searchTime << "s | ";
        cout << "Nodes: " << (float) searchStats.totalNodesSearched / 1000000 << " million | ";
        cout << "Nodes per second: " << (float) searchStats.totalNodesSearched / 1000000 / searchTime << " million | ";
        cout << "Quiescence proportion: " << (float) searchStats.totalQuiescenceSearched / searchStats.totalNodesSearched * 100 << "% | ";
        cout << "Non-capture Q proportion: " << (float) searchStats.totalNonCaptureQSearched / searchStats.totalQuiescenceSearched * 100 << "% | ";
        cout << "\n";
        cout << "Transposition Table: \n";
        cout << "\tFill rate: " << (float)TT.totalUniqueNodes / TT.getSize() * 100 << "% | ";
        cout << "Absolute size: " << (float)TT.totalUniqueNodes * sizeof(TTNode) / 1000000 << "mb\n";
        cout << "\tProbe hit rate: " << (float)TT.totalProbeFound / TT.totalProbeCalls * 100 << "% | ";
        cout << "Probe right depth rate: " << (float)TT.totalProbeRightDepth / TT.totalProbeFound * 100 << "% | ";
        cout << "{Exact Probe Rate: " << (float)TT.totalProbeExact / TT.totalProbeRightDepth * 100 << "% | ";
        cout << "Upper Probe Rate: " << (float)TT.totalProbeUpper / TT.totalProbeRightDepth * 100 << "% | ";
        cout << "Lower Probe Rate: " << (float)TT.totalProbeLower / TT.totalProbeRightDepth * 100 << "%}\n";
        cout << "\tNode set rate: " << (float)TT.totalNodesSet / TT.totalSetCalls * 100 << "% | ";
        cout << "{Overwrite proportion: " << (float)TT.totalOverwrittenNodesSet / TT.totalNodesSet * 100 << "% | ";
        cout << "Collision proportion: " << (float)TT.totalCollisionsSet / TT.totalNodesSet * 100 << "% | ";
        cout << "New node proportion: " << (float)TT.totalNewNodesSet / TT.totalNodesSet * 100 << "%}\n";
        printBoardPrettily();
    }

    return true;
}
