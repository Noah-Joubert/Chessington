//
// Created by Noah Joubert on 2021-05-09.
//
#include "../misc.h"
#include "../types.h"
#include "search.h"
#include "SearchController.h"

mutex mx;

void partitionMoveList(MoveList moves, int n, vector<MoveList> &moveLists) {
    int length = moves.size();

    // create the move lists
    for (int i = 0; i < n; i++) {
        MoveList l;
        moveLists.emplace_back(l);
    }

    int i = 0;
    while (!moves.empty()) {
        Move move = moves.back();
        moves.pop_back();

        moveLists.at(i).emplace_back(move);
        i = (i + 1) % n;
    }
}

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
    genAllMoves();
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
    getMoveList(); // annoyingly we have to generate all moves before checking for checkmate/ stalemate
    MoveList moves = activeMoveList;
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
    if (searchParameters->ttParameters.useTTInQSearch) {
        bool nodeExists = false; // whether we've stored a search for this position
        mx.lock(); //TODO work this out!
        node = TT->probe(zobristState, nodeExists); // probe the table
        mx.unlock(); //TODO work this out!

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

            //TODO chek if this works

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
        mx.lock(); //TODO work this out!
        TT->set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
        mx.unlock(); //TODO work this out!
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
        mx.lock(); //TODO work this out!
        node = TT->probe(zobristState, nodeExists); // probe the table
        mx.unlock(); //TODO work this out!

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

        // a. Late move reduction.
        //TODO this doesn't actually work. so yeah maybe work that one out. why aren't any of these techniques easy! :)
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
        mx.lock(); //TODO work this out!
        TT->set(zobristState, bestMove, depth, evaluationType, moveNumber, nodeEvaluation);
        mx.unlock(); //TODO work this out!
    }

    // * 6.
    return nodeEvaluation; // return the evaluation for the best move
}
int firstPly(SearchController board, MoveList moves, int depth, SearchResults &results) {
    /* This function separates out the first ply of the search so we can use multithreading!
     * It takes in a portion of the move list and searches it */

    /* How does it work?
     * 1. Loop through all moves, execute negamax. If a move is better than our best search so far, save it as the best move. I use alpha beta pruning
         * a. Beta represents the maximum score that the minimising player is assured of. So if the evaluation is greater than beta, the minimising player won't take this path.
         * b. Alpha represents the minimum score that the maximising player is assured of. So if the evaluation is greater than alpha, this becomes new alpha!
     * 2. Return the score from the best move searched.
     * */

    int alpha = -INFIN, beta = INFIN;
    int nodeEvaluation = -INFIN;

    // * 1.
    int subEval; // dummy variable
    Move subBestMove = 0;
    int moveCount = 0;
    for (Move move: moves) {
        moveCount ++;
        // do a full depth search
        board.makeMove(move); // make the move
        subEval = -board.negaMax(-beta, -alpha, depth - 1, subBestMove);
        board.unMakeMove(); // unmake the move

        // a. Fail low
        if (subEval > nodeEvaluation) {
            nodeEvaluation = subEval;
            mx.lock();
            results.bestMove = move; // (this used to be in the following if statement and that caused a bug!)
            mx.unlock();
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

    // add the local stats onto the global stats store
    board.flushStats();

    // * 2.
    results.evaluation = nodeEvaluation; // return the evaluation for the best move
}
SearchResults threadFactory(SearchController &SuperBoard, SearchParameters searchParameters, SearchStats &searchStats, int searchDepth) {
    /* This function divides the move-list into threads which can be searched in parallel!
     * */

    vector<thread> threads; // the threads!
    vector<MoveList> partitionedMoveLists; // the partition of the MoveList
    SearchResults results[searchParameters.numThreads];

    partitionMoveList(SuperBoard.getMoveList(), searchParameters.numThreads, partitionedMoveLists);

    for (int i = 0; i < partitionedMoveLists.size(); i++) {
        SearchController newBoard(searchParameters);
        newBoard.joinTT(SuperBoard.getTT());
        newBoard.joinSearchStats(searchStats);
        newBoard.joinSearchParams(searchParameters);

        MoveList moveList = SuperBoard.getMoveHistory();
        for (Move m: moveList) {
            newBoard.makeMove(m);
        }

        threads.emplace_back(thread(firstPly, newBoard, partitionedMoveLists[i], searchDepth, ref(results[i])));
    }

    for (thread &T: threads) {
        T.join();
    }

    int maxEval = -INFIN;
    SearchResults bestSearch;
    for (SearchResults &result: results) {
        if (result.evaluation > maxEval) {
            bestSearch = result;
            maxEval = result.evaluation;
        }
    }

    return bestSearch;
}
bool search(SearchController &SuperBoard, SearchParameters searchParameters,  Move &bestMove, string &FENFlag, bool DEBUG_MODE) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 0. Check if the game has ended
     * 1. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
         * b. Run negamax
         * c. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
         * d. See if we must break out of iterative deepening
     * 2. See if the move could have been done by another piece for PGN notation
     * 3. Make the best move, and extract the PV
     * 4. Print out stats
     * */

    //TODO make this TT a lil' bit more global!
    TranspositionTable TT(searchParameters); // for now we will just init the TT at the beginning of every search
    SuperBoard.joinTT(&TT);

    // * 0. Check if the game has ended
    SuperBoard.getMoveList();
    if (SuperBoard.inCheckMate() || SuperBoard.inStalemate() || SuperBoard.checkThreefold()) {
        cout << "game over";
        return false;
    }

    int eval = 0; // evaluation for this position
    float searchTime = 0; // time taken for the search
    int searchDepth = searchParameters.startingDepth; // the depth at which we search
    SearchStats searchStats; searchStats.clear();
    TT.clearTotals(); // clear the totals from the transposition table

    // * 1.
    while (searchTime < searchParameters.minSearchTime) {
        Timer timer; // start the timer

        // * b. Run negamax
        SearchResults results = threadFactory(SuperBoard, searchParameters, searchStats, searchDepth);
        eval = results.evaluation;
        bestMove = results.bestMove;

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

    // * 2.
    short fromSq, toSq, promo, flag, fromPc, toPc;
    FENFlag = "-";
    decodeMove(bestMove, fromSq, toSq, promo, flag, fromPc, toPc);
    for (Move move: SuperBoard.getMoveList()) {
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
    MoveList pv;
    SuperBoard.extractPV(pv);
    SuperBoard.makeMove(bestMove);

    // * 4.
    if (DEBUG_MODE) {
        cout << "---------------------=+ Search Results " << SuperBoard.getMoveNumber() - 1 << ". +=---------------------\n";
        printMovesPrettily(pv);
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
        cout << "{Exact Probe Rate: " << (float)TT.totalProbeExact / TT.totalProbeFound * 100 << "% | ";
        cout << "Upper Probe Rate: " << (float)TT.totalProbeUpper / TT.totalProbeFound * 100 << "% | ";
        cout << "Lower Probe Rate: " << (float)TT.totalProbeLower / TT.totalProbeFound * 100 << "%}\n";
        cout << "\tNode set rate: " << (float)TT.totalNodesSet / TT.totalSetCalls * 100 << "% | ";
        cout << "{Overwrite proportion: " << (float)TT.totalOverwrittenNodesSet / TT.totalNodesSet * 100 << "% | ";
        cout << "Collision proportion: " << (float)TT.totalCollisionsSet / TT.totalNodesSet * 100 << "% | ";
        cout << "New node proportion: " << (float)TT.totalNewNodesSet / TT.totalNodesSet * 100 << "%}\n";
        cout << "\tReturned move validation rate: " << (float) TT.totalTTMovesInMoveList / TT.totalTTMovesFound * 100 << "%\n";
        SuperBoard.printBoardPrettily();
    }

    return true;
}

/* Depreciated */
bool SearchController::search(Move &bestMove, string &FENFlag, bool DEBUG_MODE) {
    /* This is the search function. It executes a search */
    /* How does it do it?
     * 1. Iterative deepening. The search is timed, and the searchDepth is increased by one until the search takes an appropriate amount of time.
         * a. Check if the game has ended
         * b. Run negamax
         * c. Either exit out of iterative deepening depending on if the search took long enough, or increase the depth and keep going.
         * d. See if we must break out of iterative deepening
     * 2. See if the move could have been done by another piece for PGN notation
     * 3. Make the best move, and extract the PV
     * 4. Print out stats
     * */

    //TODO separate this from the SearchController functions

    int eval = 0; // evaluation for this position
    float searchTime = 0; // time taken for the search
    int searchDepth = searchParameters->startingDepth; // the depth at which we search
    searchStats.clear();
    TT->clearTotals(); // clear the totals from the transposition table

    // * 1.
    while (searchTime < searchParameters->minSearchTime) {
        Timer timer; // start the timer

        // * a. Check if the game has ended
        getMoveList();
        if (inCheckMate() || inStalemate() || checkThreefold()) {
            cout << "game over";
            return false;
        }

        // * b. Run negamax
        eval = negaMax(-INFIN, INFIN, searchDepth, bestMove);

        if (getCurrentSide() == BLACK) {
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
    MoveList pv;
    extractPV(pv);
    makeMove(bestMove);

    // * 4.
    if (DEBUG_MODE) {
        cout << "---------------------=+ Search Results " << moveNumber - 1 << ". +=---------------------\n";
        printMovesPrettily(pv);
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
        cout << "\tFill rate: " << (float)TT->totalUniqueNodes / TT->getSize() * 100 << "% | ";
        cout << "Absolute size: " << (float)TT->totalUniqueNodes * sizeof(TTNode) / 1000000 << "mb\n";
        cout << "\tProbe hit rate: " << (float)TT->totalProbeFound / TT->totalProbeCalls * 100 << "% | ";
        cout << "{Exact Probe Rate: " << (float)TT->totalProbeExact / TT->totalProbeFound * 100 << "% | ";
        cout << "Upper Probe Rate: " << (float)TT->totalProbeUpper / TT->totalProbeFound * 100 << "% | ";
        cout << "Lower Probe Rate: " << (float)TT->totalProbeLower / TT->totalProbeFound * 100 << "%}\n";
        cout << "\tNode set rate: " << (float)TT->totalNodesSet / TT->totalSetCalls * 100 << "% | ";
        cout << "{Overwrite proportion: " << (float)TT->totalOverwrittenNodesSet / TT->totalNodesSet * 100 << "% | ";
        cout << "Collision proportion: " << (float)TT->totalCollisionsSet / TT->totalNodesSet * 100 << "% | ";
        cout << "New node proportion: " << (float)TT->totalNewNodesSet / TT->totalNodesSet * 100 << "%}\n";
        cout << "\tReturned move validation rate: " << (float) TT->totalTTMovesInMoveList / TT->totalTTMovesFound * 100 << "%\n";
        printBoardPrettily();
    }

    return true;
}
