//
// Created by Noah Joubert on 2021-06-17.
//

#ifndef IMPROVE_CHESS_SEARCH_H
#define IMPROVE_CHESS_SEARCH_H

#define PV_NODE 1
#define REG_NODE 0
#define ASPIRATION_WIDTH 40
#define FAILED_ALPHA 0
#define FAILED_BETA 1
#define DELTA_MARGIN 150
#define LMR_COUNT 4
#define LMR_DEPTH 3
#define STALEMATE 1000

float minSearchTime = 0.1; // minimum time for a search ~ used in iterative deepening
int R = 3; // null move depth reduction
const int QUIESENCE_MARGIN = 100;
const int MAX_DEPTH = 100;
int startingDepth = 5; // starting depth of a search
long int searchDepth, negaNodes, qNodes;

Move killerMoves[MAX_DEPTH][2];
int counterMoveHistory[2][6][64];

long int failHighs = 0;
long int failHighMoves[40];

vector<long int> moveCounts;

bool useTT = true; // whether the transposition table is being used
bool useAspirationWindows = true; // whether to use aspiration windows
bool useNMP = true; // whether to use null move pruning
bool useMateDistPruning = true; // whether to use inCheckMate distance pruning
bool useFutility = false; // whether to use futility pruning
bool useReverseFutility = false; // whether to use reverse futility pruning
bool useDeltaPruning = true; // whether to use delta pruning
bool useLMR = false; // whether to use late move reductions

TranspositionTable TT;
void TranspositionTable::print(TTNode *node) {
    cout << "Key: " << node->key << " | " << "Depth: " << (int) node->depth << " | "
         << "Age: " << (int) node->age << " | "  << "Eval:" << node->eval << " | " << "Flag: " << (int)node->flag << "\t\t";
    printMoveBitboard(node->move);
}

int quiesenceSearch(SearchState &Board, int depth, int alpha, int beta, Move &bestMove, long int &nodeCount);
int oldQuiescence(SearchState &Board, int depth, float alpha, float beta, Move &bestMove);
int negaMax(SearchState &Board, int depth, int alpha, int beta, Move &bestMove, bool nullAllowed, int nodeType, long int &nodeCount);
int firstPly(SearchState &Board, int depth, int alpha, int beta, Move &bestMove, bool deepened);
bool search(SearchState &SuperBoard, Move &bMove, char &twice);

void extractPV(SearchState &Board, MoveList &PV) {
    // extracts the principle variation ie. best moves from the transposition table
    bool found = false;
    TTNode *currentNode = TT.probe(Board.getZobristState(), found);
    if (found) {
        Move bestMove = currentNode->move;

        // check for repetition in the PV to prevent a stack overflow error
        for (Move move: PV) {
            if (move == bestMove) {
                return;
            }
        }

        PV.emplace_back(bestMove);

        Board.makeMove(bestMove);
        extractPV(Board, PV);
        Board.unMakeMove();
    }
}
bool hasSearchFailed(int eval, int alpha, int beta, int &failType) {
    if (eval <= alpha) {
        failType = FAILED_ALPHA;
        return true;
    } else if (eval >= beta) {
        failType = FAILED_BETA;
        return true;
    }

    return false;
}


inline bool isQuietMove(Move &move, SearchState &Board) {
    // see if the move is a capture
    short toType = (move & toTypeMask) >> 19;
    bool isCapture = (toType != EMPTY);
    if (isCapture) return false;

    // see if the move is a promotion
    int flag = (move & flagMask) >> 14;
    bool isPromo = (flag == PROMOTION);
    if (isPromo) return false;

    if (Board.givesCheck(move)) return false;

    return true;
}
inline bool isCapture(Move &move) {
    // see if the move is a capture
    short toType = (move & toTypeMask) >> 19;
    bool isCapture = (toType != EMPTY);
    return isCapture;
}
inline bool isPromo(Move &move) {
    int flag = (move & flagMask) >> 14;
    bool isPromo = (flag == PROMOTION);
    return isPromo;
}
bool isKillerMove(Move &move, int depth) {
    return move == killerMoves[0][depth] || move == killerMoves[1][depth];
}
int moveWorth(Move &move) {
    short toType = (move & toTypeMask) >> 19;

    if (toType == EMPTY) return 0;

    return PieceWorths[toType];
}
bool closeToMate(int &alpha, int &beta) {
    int alphaDepth = alpha - MATE;
    int betaDepth = -beta - MATE;

    return ((MAX_DEPTH >= alphaDepth) && (alphaDepth >= 0)) || ((MAX_DEPTH >= betaDepth) && (betaDepth >= 0));
}
int futilityMargin(int &depth) {
    if (depth == 1) {
        return 100;
    }
    else if (depth == 2) {
        return 300;
    }
    else if (depth == 3) {
        return 500;
    }

    return INFINITY;
}
int reverseFutilityMargin(int &depth) {
    if (depth == 1) return 300 ;
    else if (depth == 2) return 500;
    else if (depth == 3) return 900;

    return INFINITY;
}
inline void LMR(int &count, int &depth) {
    depth -= 1;
}

void sortMoveList(MoveList &moves, int *scores) {
    // create a new move list which is sorted based on the array of move scores
    MoveList newList;
    while (true) {
        int highest = -INFINITY;
        int index = 0;

        for (int i = 0; i < moves.size(); i++) {
            if (scores[i] > highest) {
                index = i;
                highest = scores[i];
            }
        }

        if (highest != -INFINITY) {
            newList.emplace_back(moves.at(index));
            scores[index] = -INFINITY;
        } else {
            break;
        }
    }

    moves = newList;
}
void MVVLVA(SearchState &Board, MoveList &moves, int *scores) {
    // assign each move a score
    for (int i = 0; i < moves.size(); i++) {
        Move move = moves.at(i);
        short fromType = (move & fromTypeMask) >> 16;
        short toType = (move & toTypeMask) >> 19;

        // if it's a quite move, give it a very low move score
        if (toType == EMPTY) {
            scores[i] += -PieceWorths[QUEEN] / 2;
        } else {
            scores[i] += (PieceWorths[toType] - PieceWorths[fromType]) / 2;
        }
    }
}
void SEESort(SearchState &Board, MoveList &moves, int *scores) {
    int i = 0;
    for (Move m: moves) {
        int square = (m & toMask) >> 6;
        int toType = (m & toTypeMask) >> 19;

        Board.makeMove(m);
        scores[i] += PieceWorths[toType] - Board.SEE(square) ;
        Board.unMakeMove();
        i ++;
    }
}
void killer(MoveList &moves, int *scores, int &depth) {
        int i = 0;
        for (Move move: moves) {
            if (isKillerMove(move, depth)) {
                // add to this moves score
                scores[i] += INFINITY;
                return;
            }
            i ++;
        }
    }
void counterMove(MoveList &moves, int *scores, int side) {
    int maxCount = 1;

    int i = 0;
    for (Move move: moves) {
        int fromType = (move & fromTypeMask) >> 16;
        int toSq = (move & toMask) >> 6;

        int count = counterMoveHistory[side][fromType][toSq];

        scores[i] += count;

        if (count > maxCount) {
            maxCount = count;
        }

        i ++;
    }

    i = 0;
    for (Move m: moves) {
        scores[i] *= PieceWorths[BISHOP] / maxCount;
        i ++;
    }

}
void TTOrdering(SearchState &Board, MoveList &moves, int *scores) {
    int index = 0;
    for (Move m: moves) {
        Board.makeMove(m); // make the move

        // probe the TT
        bool nodeExists;
        TTNode *node = TT.probe(Board.getZobristState(), nodeExists);

        Board.unMakeMove(); // unmake the move

        if (nodeExists) scores[index] += node->eval; // store the move's value
        index ++;
    }
}
void orderMoves(SearchState &Board, MoveList &moves, int &depth) {
    // this move ordering is based on building up a score for each move, then sorting the list by these scores
    int moveScores[moves.size()];
    for (int i = 0; i < moves.size(); i++) moveScores[i] = 0; // set all the scores to 0

//    // get the captures and quiet moves separate
//    MoveList captures = SearchState.getCaptures();
//    MoveList quiets = SearchState.getQuiets();
//
//    /* --------------------------------- */
//    // get the capture and quiet scores
//    int captureScores[captures.size()];
//    int quietScores[quiets.size()];
//    for (int &score: captureScores) score = 0;
//    for (int &score: quietScores) score = 0;
//
//    /* Captures */
//    // sort by Most valuable victim, least valuable attacker
//    if (depth >= 3) {
//        MVVLVA(SearchState, captures, captureScores);
//    }
//
//    /* Quiets */
//    // killer move heuristic
//    if (depth >= 3) {
//        killer(quiets, quietScores, depth);
//    }
//    // counter move heuristic
//    if (depth >= 3) {
//        counterMove(quiets, quietScores, SearchState.currentSide);
//    }
//
//    if (depth >= 5) {
//        TTOrdering(SearchState, quiets, quietScores);
//        TTOrdering(SearchState, captures, captureScores);
//    }
//
//    sortMoveList(quiets, quietScores);
//    sortMoveList(captures, captureScores);
//
//    /* Combine */
//    MoveList newMoveList;
//    newMoveList.insert(newMoveList.begin(), quiets.begin(), quiets.end());
//    newMoveList.insert(newMoveList.begin(), captures.begin(), captures.end());
//    moves = newMoveList;
//
//    // look in the TT to see if there is a best move, and give it a very high score
//    if (depth >= 2) {
//        // look up the position in the TT to see if there is a best move
//        bool nodeExists;
//        TTNode *node = TT.probe(SearchState.zobristState, nodeExists);
//        if (nodeExists) {
//            Move bestMove = node->move;
//
//            // get the index of the best move in the move-list
//            for (int i = 0; i < moves.size(); i++) {
//                // if found, give the move a very high score
//                if (bestMove == moves.at(i)) {
//                    moveScores[i] += INFINITY;
//                    break;
//                }
//            }
//        }
//    }
//
//    sortMoveList(moves, moveScores);

    /* ---------------------------------- */

    /* Do different move ordering for different depths */
    // use TT values evaluations to sort moves
    if (depth >= 5) {
        TTOrdering(Board, moves, moveScores);
    }

    // evaluate how good moves are in terms of material gain
    if (depth >= 2) {
        // most valuable victim, least valuable attacker
        MVVLVA(Board, moves, moveScores);
    }

    // killer move heuristic
    if (depth >= 2) {
        killer(moves, moveScores, depth);
    }

    // look in the TT to see if there is a best move, and give it a very high score
    if (depth >= 2) {
        // look up the position in the TT to see if there is a best move
        bool nodeExists;
        TTNode *node = TT.probe(Board.getZobristState(), nodeExists);
        if (nodeExists) {
            Move bestMove = node->move;

            // get the index of the best move in the move-list
            for (int i = 0; i < moves.size(); i++) {
                // if found, give the move a very high score
                if (bestMove == moves.at(i)) {
                    moveScores[i] += INFINITY;
                    break;
                }
            }
        }
    }

    // see if moves give check, and add on a checking bonus
    if (depth >= 2) {
        int i = 0;
        for (Move m: moves) {
            if (Board.givesCheck(m)) moveScores[i] += 50;
            i++;
        }
    }

    sortMoveList(moves, moveScores);
}
void firstPlyOrderMoves(SearchState &Board, MoveList &moves) {
    int scores[moves.size()];
    for (int &score: scores) score = - 100;

    TTOrdering(Board, moves, scores);
    sortMoveList(moves, scores);

}
void orderQMoves(SearchState &Board, MoveList &moves, int &depth, int *moveScores) {
    if (depth > 4) {
        return;
    }

    SEESort(Board, moves, moveScores);
//    MVVLVA(SearchState, moves, moveScores);

    sortMoveList(moves, moveScores);
}


#endif //IMPROVE_CHESS_SEARCH_H
