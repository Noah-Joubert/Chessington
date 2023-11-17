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
void SearchController::extractPV(MoveList &moves) {
    /* Extract the principle variation from the TT */

    // generate legal moves
    genMoves();
    MoveList legalMoves = combinedMoveList;
    if (inCheckMate() | inStalemate() | checkThreefold()) {
        // check if the game is over
        return;
    }

    // probe the TT
    bool found = false;
    TTNode *ttEntry = TT->probe(zobristState, found);
    auto pos = std::find(legalMoves.begin(), legalMoves.end(), ttEntry->move);

    // check the move is legal, and the TT entry is found
    if (found && (pos != legalMoves.end())) {
        moves.insert(moves.begin(), ttEntry->move);
        makeMove(ttEntry->move);
        extractPV(moves);
        unMakeMove();
    } else {
        return;
    }
};

int SearchController::quiescence(int alpha, int beta, int depth) {
    /* What is the Quiescence Search?
     * This is a special type of search which we enter at depth 0. It only considers captures.
     * This is to mitigate the horizon effect whereby the search stops just before a series of moves which drastically effect the game.
     * It tends to take up at least 50% of the search, so definitely worth some thought.
     * */

    /* 1. Take the static evaluation as a standPat score which represents the minimum evaluation for this node.
     * 2. Check for checkmate/ stalemate
     * 3. Try probing TT
     * 4. Loop through all moves and continue the quiescence search
        * a. SEE pruning. Static Exchange Evaluation gives the value of exchange of material on particular square.
     * 5. In the case that we don't search any moves, we are at the bottom of the tree, so return the evaluation
     * 6. Save the search to the TT
     *
     * */
    searchStats.totalNodesSearched++; // count the number of nodes searched
    searchStats.totalQuiescenceSearched ++; // count the quiescence nodes searched

    int originalAlpha = alpha, originalBeta = beta; // store the original alpha/ beta so we can identify this node type

    Move bestMove; // best move from this node

    // * 1.
    // A quiescence search should always improve the board evaluation for the current player.
    // This is because the point of the quiescence search is to minimise the horizon effect, whereby the boards static evaluation is much less than the actual evaluation of the position.
    // So we take the standPat as a lower bound on our evaluation (alpha).
    int standPat = relativeLazy(); // we take the current static evaluation as a lower bound on the score
    if (standPat >= beta) {
        // fail hard
        return beta;
    }

    // else finally take standard pat as alpha
    if (alpha < standPat) {
        alpha = standPat;
    }

    // * 2.
    //TODO. side effects! implicit data dependence
    getMoveList(); // annoyingly we have to generate all moves before checking for checkmate/ stalemate
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return searchParameters->stalemateEvaluation;
    }

    // * 3. Probe the TT
    TTNode *node;
    MoveList moves = activeMoveList;
    //TODO get move from hash table if exists
    if (searchParameters->ttParameters.useTTInQSearch) {
        bool nodeExists = false; // whether we've stored a search for this position
        node = TT->probe(zobristState, nodeExists); // probe the table

        if (nodeExists) {
            // see if the node exists and put it to the front of the move list
            Move TTMove = node->move;
            auto pos = std::remove(moves.begin(), moves.end(), TTMove);
            TT->totalTTMovesFound ++;

            // see if the move is actually valid
            if (pos != moves.end()) {
                TT->totalTTMovesInMoveList ++;
                moves.insert(moves.begin(), TTMove);
            }
        }
    }

    // * 4.
    int nodeEvaluation = -INFIN;
    int movesSearched = 0; // keep track of the number of moves properly searched, as if none we will need to do a proper evaluation
    for (Move move: moves) {
        if (!(move & toTypeMask)) {
            // see if this is a non-capture quiescence move
            searchStats.totalNonCaptureQSearched ++;

            // if we are deep enough stop making these moves, unless promotion
            if ((depth <= searchParameters->maxDepthForChecks) && !(move & promoMask)) {
                continue;
            }
        }

        makeMove(move); // make the move before doing SEE

        /* a. SSE pruning/ Delta pruning
         * As a quiescence search should improve the nodes' evaluation, we can confidently prune nodes with negative SEE.
         * We use this together with delta pruning, where we prune a move if we don't believe it can raise alpha
         * i.e. lazyEval() + SEE + Margin < alpha.
         * */
        int SEEEvaluation = SEEMove(moveHistory.back());
        if (searchParameters->useSEE) {

            if (
                    SEEEvaluation < 0 ||
                    (searchParameters->useDelta &&
                     (standPat + PieceScores[getTooPiece(move)] + searchParameters->deltaMargin < alpha))
                    ) {
                unMakeMove();
                continue;
            }
        } else {
            if (    false ||
                    (searchParameters->useDelta &&
                     (standPat + PieceScores[getTooPiece(move)] + searchParameters->deltaMargin < alpha))
                    ) {
                unMakeMove();
                continue;
            }
        }

        movesSearched ++;

        // do a full depth search
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
            alpha = beta;
            break;
        }
    }

    // * 5. In the case that we don't search any moves, we are at the bottom of the tree, so return the evaluation
    if (!movesSearched) {
        return evaluate();
    }

    // * 6. write to the TT
    int evaluationType = getEvaluationType(nodeEvaluation, originalAlpha, beta);
    if (searchParameters->ttParameters.useTTInQSearch) {
        TT->set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
    }

    return nodeEvaluation;
}
int SearchController::negaMax(int alpha, int beta, int depth, Move &bestMove) {
    /* Negamax */
    /* How does it work?
     * 1. The depth counts down to 0. At which point we enter the quiescence search
     * 2. We then generate moves, so we can check for checkmates/stalemates/three-folds. It returns a massive negative number in the case of check-mate (as it would be bad for the current player).
     * 3. Probe the TT
     * 4. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move. I use alpha beta pruning
         * a. Try a late move reduction. This is where we reduced the depth of the search. We only do it under certain circumstances.
         * b. Beta represent-> the maximum score that the minimising player is assured of. So if the evaluation is greater than beta, the minimising player won't take this path.
         * c. Alpha represents the minimum score that the maximising player is assured of. So if the evaluation is greater than alpha, this becomes new alpha!
     * 5. Work out the alpha/beta evaluation type. Either we have hard failed high, in which case the evaluation is a lower bound - beta. Or we have failed low, so the evaluation is an upper bound - alpha. Then write to TT
     * 6. Return the score from the best move searched.
     * */
    searchStats.totalNodesSearched++; // count the number of nodes searched
    int originalAlpha = alpha, originalBeta = beta; // store the original alpha/ beta so we can identify this node type
    int nodeEvaluation = -INFIN;

    // * 1.
    if (depth <= 0) {
        /* Go into quiescence search! */
        if (searchParameters->useQuiescence) {return quiescence(alpha, beta, depth);}
        else {return evaluate();}
    }

    // * 2.
    MoveList moves = getMoveList(); // generate moves before checking for checkmate/ stalemate
    if (inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return (-MATE - depth);
    } else if (inStalemate() | checkThreefold()) {
        // if there is a three-fold or a inStalemate, return the negative of the evaluation
        return searchParameters->stalemateEvaluation;
    }

    // * 3. Probe the TT
    TTNode *node;
    if (searchParameters->ttParameters.useTT) {
        bool nodeExists = false; // whether we've stored a search for this position
        node = TT->probe(zobristState, nodeExists); // probe the table

        if (nodeExists) {
            // see if the node exists and put it to the front of the move list
            Move TTMove = node->move;
            auto pos = std::remove(moves.begin(), moves.end(), TTMove);
            TT->totalTTMovesFound ++;

            // see if the move is actually valid
            if (pos != moves.end()) {
                TT->totalTTMovesInMoveList ++;
                moves.insert(moves.begin(), TTMove);

                // try using the results to improve alpha/ beta
                if ((node->depth >= depth) && searchParameters->ttParameters.useTTPruning) {
                    if (node->flag == EXACT_EVAL) {
                        bestMove = TTMove;
                        return node->eval;
                    } else if (node->flag == LOWER_EVAL) {
                        alpha = alpha > node->eval ? alpha: node->eval;
                    } else if (node->flag == UPPER_EVAL) {
                        beta = beta < node->eval ? beta: node->eval;
                    }

                    if (alpha >= beta) {
                        return alpha;
                    }
                }
            }
        }
    }

    // * 4.
    int posInMoveList = 0; // how far we are into the move-list
    int fullMovesSearched = 0; // the number of full searches we have carried out
    Move subBestMove = 0;
    for (Move move: moves) {
        posInMoveList ++;

        // a. Late move reduction (Doesn't work at the moment)!
        int subEval;
        if (
                (searchParameters->useLMR) && // LMR is available
                (fullMovesSearched >= searchParameters->minMovesBeforeLMR) &&  // we've searched some moves to full depth
                (depth <= searchParameters->useLMRDepth) && // we are deep enough
                (posInMoveList > activeMoveList.size()) && // move is not tactical
                (!inCheck) // not in check
                ) {
            // do a search at a reduced depth to see if we fail low, if we do, then we prune this node
            subEval = -negaMax(-alpha - 1, -alpha, depth - 2, subBestMove);
            if (subEval <= alpha) {
                continue;
            }
        } 

        fullMovesSearched ++;

        // do a full depth search
        makeMove(move); // make the move
        subEval = -negaMax(-beta, -alpha, depth - 1, subBestMove);
        unMakeMove(); // unmake the move

        // b. Fail low
        if (subEval > nodeEvaluation) {
            nodeEvaluation = subEval;
            bestMove = move; // (this used to be in the following if statement and that caused a bug!)
            if (nodeEvaluation > alpha) {
                alpha = nodeEvaluation;
            }
        }

        // c. Fail hard beta cut off.
        if (alpha >= beta) {
            alpha = beta;
            break;
        }
    }

    /* Discussion: How should we treat each evaluation type?
     * First consider vanilla alpha/beta pruning. Until we fail high, every single move needs to be considered.
     * So fail high e.g. LOWER_EVAL nodes should be searched first. This leads into move ordering principles i.e. search captures first as they are more likely to fail high.
     * So for more ordering, we prioritise fail high, exact, then fail low. (LOWER_EVAL, EXACT_EVAL, UPPER_EVAL)
     * But for transposition tables entries fail high nodes are great as they raise alpha, but fail low nodes are also good as they lower beta. And both can lead to a cutoff
     * All in all, exact valuations are best
     * */
    // * 5. write to the TT
    int evaluationType = getEvaluationType(nodeEvaluation, originalAlpha,  beta); // we pass the original alpha, and the new beta
    if (searchParameters->ttParameters.useTT) {
        TT->set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
    }

    // * 6.
    return nodeEvaluation; // return the evaluation for the best move
}
SearchResults search(SearchController &SuperBoard) {
    /* This is the search function. It executes a search, and returns the results */
    /* How does it do it?
     * 0. Firstly prepare various variables for the search.
     * 1 Check if the game has ended
     * 2. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
         * b. Run negamax
         * c. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
         * d. See if we must break out of iterative deepening
     * 3. Build the results object, and return it
     * */

    // * 0. Prepare variables
    SearchResults searchResults; // the return object
    SearchParameters *searchParameters = SuperBoard.getSearchParameters(); // fetch the search parameters
    SuperBoard.clearStats(); // clear the stats counter
    SuperBoard.getTT()->clearTotals(); // clear the totals from the transposition table //todo embed this into searchstates

    int eval = 0; // evaluation for this position
    float searchTime = 0; // time taken for the search
    int searchDepth = searchParameters->startingDepth; // the depth at which we search
    Move bestMove;

    // * 1. Check if the game has ended
    SuperBoard.getMoveList();
    if (SuperBoard.inCheckMate() || SuperBoard.inStalemate() || SuperBoard.checkThreefold()) {
        searchResults.searchCompleted = false;
        return searchResults;
    }

    // * 2. Iterative deepening
    while (searchTime < searchParameters->minSearchTime) {
        Timer timer; // start the timer

        // * b. Run negamax
        eval = SuperBoard.negaMax(-INFIN, INFIN, searchDepth, bestMove);

        if (SuperBoard.getCurrentSide() == BLACK) {
            eval *= -1;
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

    // * 3. build the results object
    searchResults.evaluation = eval;
    searchResults.bestMove = bestMove;
    searchResults.searchCompleted = true;
    SuperBoard.extractPV(searchResults.principleVariation); //todo remove first element and emplace the bestMove
    searchResults.stats = SuperBoard.getStats();
    searchResults.searchTime = searchTime;
    searchResults.depth = searchDepth;

    return searchResults;
}
