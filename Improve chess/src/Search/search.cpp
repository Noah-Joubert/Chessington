//
// Created by Noah Joubert on 2021-05-09.
//
#include "SearchState.cpp"
#include "../misc.cpp"
#include "search.h"

int quiesenceSearch(SearchState&Board, int depth, int alpha, int beta, Move &bestMove, long int &nodeCount) {
    nodeCount ++;
    qNodes ++;

    // get stand pat
    int standPat = Board.evaluate(alpha, beta);
    if (standPat >= beta) return beta;
    if (standPat > alpha) alpha = standPat;

    // get the q-moves
    MoveList qMoves = Board.getQMoveList();

    int scores[qMoves.size()];
    for (int i = 0; i < qMoves.size(); i++) scores[i] = 0; // set all the scores to 0
    if (!qMoves.empty()) orderQMoves(Board, qMoves, depth, scores);

    // check for checkmate/ (inStalemate or checkThreefold)
    if (Board.inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating allMoves
        // return -MATE as a checkmate is very bad for the current player
        return -MATE - depth;

    } else if (Board.checkThreefold()) {
        // if there is a three fold, return the negative of the evaluation
        // we don't check for inStalemate as only qMoves are generated
        return STALEMATE;
    }

    int moveCount = 0;
    for (Move move: qMoves) {
        // try delta pruning ~ skipping a capture if it is unlikely to raise alpha
        if (useDeltaPruning &&  // if delta pruning is enabled
            Board.getPhase() <= 18 && // and not late in endgame
            moveWorth(move) + DELTA_MARGIN + standPat < alpha // and this capture won't beat alpha
            ) continue;

        // skip moves if their SEE is less than 0
//        if (scores[moveCount] < 0) break;

        Board.makeMove(move);
        int eval = - quiesenceSearch(Board, depth + 1, -beta, -alpha, bestMove, nodeCount);
        Board.unMakeMove();

        moveCount ++;

        // check for a fail high
        if (eval >= beta) {
            // record the fail high
            failHighMoves[moveCount - 1] ++;
            failHighs ++;

            return beta;
        }
        if (eval > alpha) {
            alpha = eval;
        }
    }

    return alpha;
}
int negaMax(SearchState &Board, int depth, int alpha, int beta, Move &bestMove, bool nullAllowed, int nodeType, long int &nodeCount) {
    /* this is the search function */
    // if at the end of the tree, return static evaluation
    nodeCount++;
    negaNodes ++;

    if (depth <= 0) {
        // return static evaluation
        // this used to be -beta, -alpha which caused me many, many hours of headaches!
        return quiesenceSearch(Board, 0, alpha, beta, bestMove, nodeCount);
//        return SearchState.evaluate(alpha, beta);
    }

    int alphaOriginal = alpha; // store alpha
    int bestEval = -INFINITY; // stores the evaluation of the best move
    Move subBestMove = 0; // stores the best move from this position

    // probe the transposition table
    if (useTT) {
        // look up the move in the transposition table
        bool nodeExists;
        TTNode *node = TT.probe(Board.getZobristState(), nodeExists);

        // check if the node exists and is to the right depth
        if (nodeExists && node->depth >= depth) {
            // see what type of evaluation this is
            if (node->flag == EXACT_EVAL) {
                // do a quick legality check on the move by seeing if it is in the move list
                subBestMove = node->move;
                bestEval = node->eval;
            } else if (node->flag == LOWER_EVAL) {
                alpha = node->eval > alpha ? node->eval : alpha;
            } else if (node->flag == UPPER_EVAL) {
                beta = node->eval < beta ? node->eval : beta;
            }

            if (alpha >= beta) {
                bestMove = node->move;
                bestEval = node->eval;
                return bestEval;
            }
        }
    }

    // generate all the moves ~ this needs to be done before checking for inCheckMate/ inStalemate
    MoveList moves = Board.getMoveList();
    orderMoves(Board, moves, depth); // order the moves

    // now validate the potential TT move
    if (subBestMove != 0) {
        if (find(moves.begin(), moves.end(), subBestMove) != moves.end()) {
            bestMove = subBestMove;
            return bestEval;
        }
    }

    // get the lazy static evaluation of the position
    int staticEval = Board.relativeLazy();

    // do mate distance pruning ~ if a inCheckMate has been found at a lower depth, prune the branch
    // first check upper-bound
    if (useMateDistPruning) {
        int matingValue = MATE + depth;
        if (matingValue < beta) {
            beta = matingValue;
            if (alpha >= matingValue) return matingValue;
        }
        // now check lower bound
        matingValue = -MATE - depth;
        if (matingValue > alpha) {
            alpha = matingValue;
            if (beta <= matingValue) return matingValue;
        }
    }

    // check for checkmate/ (inStalemate or checkThreefold)
    if (Board.inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return -MATE - depth;

    } else if (Board.inStalemate() | Board.checkThreefold()) {
        // if there is a three fold or a inStalemate, return the negative of the evaluation
        return STALEMATE;

    }

    // consider trying a null move ~ doing a null move if the player is winning, to see if the opponent can
    // improve it's position, by doing a reduced depth search
    if (depth >= 7) R = 4;
    else if (depth >= 3) R = 3;
    else R = 2;
    if (useNMP &&
        !Board.getInCheck() && // not in check
            depth >= R && // and not above a certain depth R
            nullAllowed && // and the last move was not a null move
            staticEval >= beta && // and the static eval exceeds beta
            nodeType != PV_NODE && // and this is not a pv node
            Board.getPhase() <= 18 // and not in in the endgame
            ) {
        // switch side to carry out the null move
        Board.switchSide();

        // do the search at a reduced depth
        int eval = -negaMax(Board, depth - R, -beta, -beta + 1, bestMove, false, nodeType, nodeCount);

        Board.switchSide();

        if (eval >= beta) return eval;
    }

    // see whether we should do futility pruning
    bool futilityPruning = useFutility &&
                           !Board.getInCheck(); // the board isn't in check

    // consider reverse futility pruning
    if (useReverseFutility &&
        !Board.getInCheck() &&
        nodeType != PV_NODE && // not a pv-node
        staticEval - reverseFutilityMargin(depth) >= beta && // the move has no potential to raise alpha
        !closeToMate(alpha, beta) // not close to inCheckMate
        ) {
        if (depth == 1 || depth == 2) {
            return staticEval;
        } if (depth == 3) {
            depth --;
        }
    }

    // now loop through all the moves
    nodeType = PV_NODE;
    int moveCount = 0; // keep track of how many moves have been searched
    for (Move move: moves) {
        int value; // evaluation of this node

        // try futility pruning ~ skipping a move if it has no potential to raise alpha
        if (futilityPruning &&
            nodeType != PV_NODE && // not a pv-node
            staticEval + futilityMargin(depth) + moveWorth(move) <= alpha && // the move has no potential to raise alpha
            !givesCheck(move, Board) && // the move is quiet
            !closeToMate(alpha, beta) // not close to inCheckMate
            ) {
            continue;
        }


        // consider trying late move reductions
        if (useLMR && // and LMR is enabled
            moveCount >= LMR_COUNT && // and enough moves have been searched
            depth >= LMR_DEPTH && // depth at least 3
            depth <= searchDepth - 2 && // at least two plies from root
            nodeType != PV_NODE && // not a pvNode
            !Board.getInCheck() && // if not in check
            !isCapture(move) && // not a capture
            !isPromo(move) && // not a promotion
            !givesCheck(move, Board) && // not a check
            !isKillerMove(move, depth) // not a killer
            ) {

            // make the search
            Board.makeMove(move); // make the move
            value = - negaMax(Board, depth - 1 - 1, -(alpha + 1), -alpha, bestMove, true, nodeType, nodeCount);
            Board.unMakeMove(); // unmake the move

        } else {
            value = alpha + 1;
        }

        // do a full depth search if no reductions or if the score is above alpha
        if (value > alpha) {
            Board.makeMove(move); // make the move
            value = -negaMax(Board, depth - 1, -beta, -alpha, bestMove, true, nodeType, nodeCount);
            Board.unMakeMove(); // unmake the move
        }


        moveCount += 1;

        // see if this is the new best move
        if (value > bestEval) {
            // store information about this move
            bestEval = value;
            subBestMove = move;
        }

        // update alpha
        if (bestEval > alpha) {
            alpha = bestEval;
        }

        // check for a fail high
        if (alpha >= beta) {
            failHighMoves[moveCount - 1] ++;
            failHighs ++;

            // see if this is a 'killer move' ie. quiet
            if (isQuietMove(move, Board)) {
                // store the move as a killer move
                if (killerMoves[0][depth] != move) {
                    killerMoves[1][depth] = killerMoves[0][depth];
                    killerMoves[0][depth] = move;
                }

                // store the move for the history heuristic
                int fromType = (move & fromTypeMask) >> 16;
                int toSq = (move & toMask) >> 6;
                counterMoveHistory[Board.getCurrentSide()][fromType][toSq] += depth * depth;
            }

            break;
        }

        nodeType = REG_NODE;
    }

    // add the search to the transposition table
    int flag; // evaluation type
    if (bestEval <= alphaOriginal) {
        flag = UPPER_EVAL;
    } else if (bestEval >= beta) {
        flag = LOWER_EVAL;
    } else {
        flag = EXACT_EVAL;
    }
    TT.set(Board.getZobristState(), subBestMove, depth, flag, Board.getMoveNumber(), bestEval);

    bestMove = subBestMove; // send the best move up a recurrence
    return bestEval; // return the evaluation for the best move
}
int firstPly(SearchState &Board, int depth, int alpha, int beta, Move &bestMove, bool deepened) {
    /* this is the search function */
    // if at the end of the tree, return static evaluation

    int alphaOriginal = alpha; // store alpha
    int bestEval = -INFINITY; // stores the evaluation of the best move
    Move subBestMove = 0; // stores the best move from this position
    int nullAllowed = true;
    int nodeType;

    // generate all the moves ~ this needs to be done before checking for inCheckMate/ inStalemate
    MoveList moves = Board.getMoveList();
    orderMoves(Board, moves, depth);

    // get the lazy static evaluation of the position
    int staticEval = Board.relativeLazy();

    // check for checkmate/ (inStalemate or checkThreefold)
    if (Board.inCheckMate()) {
        // return static evaluation ~ do this after checking if depth == 0, to avoid generating moves
        // return -MATE as a checkmate is very bad for the current player
        return -MATE - depth;

    } else if (Board.inStalemate() | Board.checkThreefold()) {
        // if there is a three fold or a inStalemate, return the negative of the evaluation
        return STALEMATE;

    }

    // consider trying a null move ~ doing a null move if the player is winning, to see if the opponent can
    // improve it's position, by doing a reduced depth search
    if (depth >= 7) R = 4;
    else if (depth >= 6) R = 3;
    else R = 2;
    if (useNMP &&
        !Board.getInCheck() && // not in check
            depth >= R && // and not above a certain depth R
            nullAllowed && // and the last move was not a null move
            staticEval >= beta && // and the static eval exceeds beta
            nodeType != PV_NODE && // and this is not a pv node
            Board.getPhase() <= 18 // and not in in the endgame
            ) {
        // switch side to carry out the null move
        Board.switchSide();

        // do the search at a reduced depth
        int eval = -negaMax(Board, depth - R, -beta, -beta + 1, bestMove, false, nodeType, negaNodes);

        Board.switchSide();

        if (eval >= beta) return eval;
    }

    int firstPlyReduction = moves.size() / 2;
    int secondPlyReduction = moves.size() * 3/4;

    // now loop through all the moves
    int i = 0;
    nodeType = PV_NODE;
    int moveCount = 0;
    for (Move move: moves) {
        moveCount ++;
        long int nodeCount = 0;

        Board.makeMove(move);
        int value = -negaMax(Board, depth - 1, -beta, -alpha, bestMove, true, nodeType, nodeCount);
        Board.unMakeMove();

        negaNodes += nodeCount;
        i++;

        // see if this is the new best move
        if (value > bestEval) {
            // store information about this move
            bestEval = value;
            subBestMove = move;
        }

        // update alpha
        if (bestEval > alpha) {
            alpha = bestEval;
        }

        // check for an alpha-beta break
        if (alpha >= beta) {
            break;
        }

        nodeType = REG_NODE;
    }

    // add the search to the transposition table
    int flag; // evaluation type
    if (bestEval <= alphaOriginal) {
        flag = UPPER_EVAL;
    } else if (bestEval >= beta) {
        flag = LOWER_EVAL;
    } else {
        flag = EXACT_EVAL;
    }

    TT.set(Board.getZobristState(), subBestMove, depth, flag, Board.getMoveNumber(), bestEval);

    bestMove = subBestMove; // send the best move up a recurrence
    return bestEval; // return the evaluation for the best move
}
bool search(SearchState &SuperBoard, Move &bMove, char &twice) {
    float searchTime = 0; // time taken for the search
    int eval = 0; // time taken for the evaluation
    Move bestMove = 0; // holds the best move

    searchDepth = startingDepth;

    // iterative deepening
    cout << "Current depth: ";
    bool failedSearch = false; // whether the previous search failed
    bool prevSearchFailed = false;
    int failType; // whether the search failed low or high
    bool firstSearch = true; // whether this is the first search, so we can't use aspiration windows
    int previousEval; // holds the evaluation of the previous search
    int aspirationWindow = ASPIRATION_WIDTH;

    // initialize the array which keeps track of what move a beta cutt-off occurs on
//    for (long int &i: failHighMoves) i = 0;
//    failHighs = 0;

    while (true) {
        cout << searchDepth << ",";

        // determine the aspiration window size
        aspirationWindow = ASPIRATION_WIDTH;

        Timer timer; // start the timer

        negaNodes = 0; // reset the move count

        // get alpha and beta
        int alpha, beta;
        if (firstSearch || (!useAspirationWindows)) {
            alpha = -INFINITY, beta = INFINITY;
        } else {
            alpha = previousEval - aspirationWindow, beta = previousEval + aspirationWindow;

            // if the search failed, adjust the bounds accordingly
            if (failedSearch) {
                if (failType == FAILED_ALPHA) {
                    alpha = -INFINITY;
                } else if (failType == FAILED_BETA) {
                    beta = INFINITY;
                }
            }

            // if two searches have failed, loose all bounds
            if (failedSearch && prevSearchFailed) {
                alpha = -INFINITY;
                beta = INFINITY;
            }
        }

        // do the search
        eval = firstPly(SuperBoard, searchDepth, alpha, beta, bestMove, !firstSearch); // run the search

        // check if the game has ended
        SuperBoard.getMoveList();
        if (SuperBoard.inCheckMate() || SuperBoard.inStalemate() || SuperBoard.checkThreefold()) {
            cout << "Game over.\n";
            return false;
        }

        searchTime = timer.end(); // end the timer

        // see if the search has failed
        prevSearchFailed = failedSearch;
        failedSearch = hasSearchFailed(eval, alpha, beta, failType);

        if (!failedSearch) {
            // if the search didn't fail, either increase the search depth, or quit
            firstSearch = false;
            failedSearch = false;

            // store the last evaluation
            previousEval = eval;

            if (searchTime < minSearchTime && abs(eval) <= MATE) {
                // if the search was too quick, increase the depth
                searchDepth ++;
                continue;
            } else {
                // else finish the search, if it took to much time, or a inCheckMate has been found
                break;
            }
        } else {
            // if the search did fail, do a research at the same depth
            failedSearch = true;
            cout << "(failed " << (float)aspirationWindow / 100 << " " << previousEval << " " << ")";
        }
    }
    cout << "\n";

    if (SuperBoard.getCurrentSide() == BLACK) {
        eval *= -1;
    }

    // print out the results
    cout << setw(15) << "Score" << "|" << setw(15) << "Negamax knodes" << "|" << setw(15) << "Q knodes" << "|" << setw(15) << "Total knodes" << "|" << setw(15) << "Time taken" << "|"
        << setw(15) << "kNPS" << "|" << setw(15) << "Depth" << "|" << "\n";
    cout << setw(15) << (float)eval/100 << "|" << setw(15) << negaNodes / 1000 << "|" << setw(15) << qNodes / 1000 << "|" << setw(15) << (qNodes + negaNodes) / 1000 << "|" << setw(15) << searchTime << "|"
        << setw(15) << (negaNodes+qNodes) / searchTime / 1000 << "|" << setw(15) << searchDepth << "|" << "\n";

    // extract the pv
    MoveList PV;
    extractPV(SuperBoard, PV);
    bMove = PV.front(); // get the best move
    printMovesPrettily(PV); // print the pv

    // see if the move could have been done by another piece for PGN notation
    bMove = PV.front();
    short fromSq, toSq, promo, flag, fromPc, toPc;
    decodeMove(bMove, fromSq, toSq, promo, flag, fromPc, toPc);
    for (Move move: SuperBoard.getMoveList()) {
        if (move == bMove) continue;

        short fromSq1, toSq1, promo1, flag1, fromPc1, toPc1;
        decodeMove(move, fromSq1, toSq1, promo1, flag1, fromPc1, toPc1);

        if (toSq == toSq1 && fromPc == fromPc1) {
            // get the files
            int file = fromSq % 8, file1 = fromSq1 % 8;
            int rank = fromSq / 8, rank1 = fromSq1 / 8;

            if (file != file1) {
                twice = 'a' + file;
            } else if (rank != rank1) {
                twice = '0' + (8-rank);
            }
         }
    }

    // make the move and print the board
    SuperBoard.makeMove(bMove);
    SuperBoard.printBoardPrettily();

    // print out the TT stats
    cout << "Total nodes: " << TT.total << " | " << "Collisions: " << TT.collisions << " | " << "Overwrites: " << TT.overwrites << " | " << "New nodes: " << TT.newNodes << "\n";
    TT.total = 0;
    TT.collisions = 0;
    TT.overwrites = 0;
    TT.newNodes = 0;

    // print out the fail high stats - to see how effective move ordering is
    int num = 5;
    for (int i = 0; i < 5; i++) cout << setw(15) << "Move: "  + to_string(i + 1) + " | ";
    cout << "\n";
    for (int i = 0; i < 5; i++) cout << setw(15) << to_string(100 * failHighMoves[i] / failHighs) +  " | ";
    cout << "\n";

    return true;
}
