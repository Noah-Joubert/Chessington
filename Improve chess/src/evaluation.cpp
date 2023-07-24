//
// Created by Noah Joubert on 2021-05-09.
//

#include "evaluation.h"
#include "Search/SearchState.cpp"

#define MOBILITY_WEIGHT 2
#define KING_QUEEN_MOVES 3
#define KNIGHT_PER_PAWN_BONUS 3
#define BISHOP_PAIR 30
#define ROOK_OPEN_FILE 4
#define ROOK_PER_PAWN_BONUS 3
#define DOUBLED_PAWN 40
#define EARLY_QUEEN 150
#define CASTLE_BONUS 75
#define CAN_CASTLE_BONUS 20
#define PASSED_PAWN_PUSH_ENDGAME 90
#define NORMAL_PAWN_PUSH_ENDGAME 15

const int LAZY_THRESHOLD = 150;

int dotProduct(U64 BB, const Byte weights[]) {
    float total = 0;

    while (BB) {
        short index = popIntLSB(BB);
        total += weights[index];
    }

    return total;
}

int SearchState::relativeLazy() {
    return lazyEval() * (currentSide ? 1: -1);
}
int SearchState::lazyEval() {
    int score = material;

    score -= pst;

    return score;
}
int SearchState::openingEval(U64 wAttacks, U64 bAttacks) {
    int eval = 0;

    /* Step 1: Piece Square Tables */
    // use incrementally updated piece square tables and then opening specific king tables
    eval += pst;
    U64 WKing = pieceBB[KING] & pieceBB[nWhite], BKing = pieceBB[KING] & pieceBB[nBlack];
    eval += PST_Opening_King[WHITE][bitScanForward(WKing)];
    eval -= PST_Opening_King[BLACK][bitScanForward(BKing)];

    /* Step 2: King safety */
    // add on castling bonus
    if (hasCastled[WHITE]) eval += CASTLE_BONUS;
    else if (canCastle(WHITE)) eval += CAN_CASTLE_BONUS;
    if (hasCastled[BLACK]) eval -= CASTLE_BONUS;
    else if (canCastle(BLACK)) eval -= CAN_CASTLE_BONUS;
    // treat the kings as a queen, calculate it's moves, and aim to minimise it's moves
    U64 wMoves = genAttack<QUEEN>(WKing, emptySquares), bMoves = genAttack<QUEEN>(BKing, emptySquares);
    eval += (count(bMoves) - count(wMoves)) * KING_QUEEN_MOVES;

    /* Step 3: Penalty for early queen development */
    // see if the queen's have left the 3x3 set of squares from where they started
    U64 wQueen = getPieces(QUEEN, WHITE), bQueen = getPieces(QUEEN, BLACK);
    U64 wQAllowed = genAttack<KING>(toBB(D1), emptySquares), bQAllowed = genAttack<KING>(toBB(D8), emptySquares);
    if ((wQAllowed & wQueen) == 0) {eval -= EARLY_QUEEN;}
    if ((bQAllowed & bQueen) == 0) {eval += EARLY_QUEEN;}

    return eval;
}
int SearchState::endgameEval() {
    int eval = 0;

    /* Step 1: King activity */
    U64 WKing = pieceBB[KING] & pieceBB[nWhite], BKing = pieceBB[KING] & pieceBB[nBlack];
    eval += PST_Endgame_King[WHITE][bitScanForward(WKing)];
    eval -= PST_Endgame_King[BLACK][bitScanForward(BKing)];

    /* Step 2: Material worth's */
    // look for passed pawns
    U64 wPawns = getPieces(PAWN, WHITE), bPawns = getPieces(PAWN, BLACK);
    for (U64 file: {AFile, BFile, CFile, DFile, EFile, FFile, GFile, HFile}) {
        // look for pawns on this file
        U64 w = wPawns & file, b = bPawns & file;

        // look for pawns on this and adjacent files
        U64 adjacentFiles = file | ((file >> 1) & notHFile) | ((file << 1) & notAFile);
        U64 adjW = wPawns & adjacentFiles, adjB = bPawns & adjacentFiles;

        // first see if the white pawn is passed
        U64 wSingle = popLSB(w);
        int wPassed = true;
        while (adjB != 0) {
            U64 bBlocker = popLSB(adjB);

            // see if the white pawn is closer to the end then the black pawn
            if (wSingle > bBlocker) {
                wPassed = false;
                break;
            }
        }
        if (wSingle == 0) wPassed = false;

        int bPassed = true;
        U64 bSingle = popLSB(b);
        while (adjW != 0) {
            U64 wBlocker = popLSB(adjW);

            // see if the white pawn is closer to the end then the black pawn
            if (bSingle < wBlocker) {
                bPassed = false;
                break;
            }
        }
        if (bSingle == 0) bPassed = false;


        // see how far the pawn is from the end
        int distance = 7 - bitScanForward(wSingle) / 8;
        if (wSingle != 0) {
            if (wPassed) {
                eval += PASSED_PAWN_PUSH_ENDGAME * distance;
            } else {
                eval += NORMAL_PAWN_PUSH_ENDGAME * distance;
            }
        }
        if (bSingle != 0) {
            distance = bitScanForward(bSingle) / 8;
            if (bPassed) {
                eval -= PASSED_PAWN_PUSH_ENDGAME * distance;
            } else {
                eval -= NORMAL_PAWN_PUSH_ENDGAME * distance;
            }
        }
//        cout << "File: " << popIntLSB(file) + 1 << " " << wPassed << bPassed << " " << eval <<  "\n";
    }

    return eval;
}
int SearchState::materialEval() {
    int eval = 0;

    /* Start with knights */
    // their value can range from 3 - 2.5, depending on how many pawns there are
    U64 wKnights = getPieces(KNIGHT, WHITE), bKnights = getPieces(KNIGHT, BLACK);
    short pawnCount = count(pieceBB[PAWN]);
    eval += (250 + pawnCount * KNIGHT_PER_PAWN_BONUS) * (count(wKnights) - count(bKnights));

    /* Now bishops */
    // add on bonus for bishop pair, then just regular bishop value
    U64 wBishops = getPieces(BISHOP, WHITE), bBishops = getPieces(BISHOP, BLACK);
    if (count(wBishops) >= 2) eval += BISHOP_PAIR;
    if (count(bBishops) >= 2) eval -= BISHOP_PAIR;
    eval += (300) * (count(wBishops) - count(bBishops));

    /* Now rooks */
    // their value can range from 5 - 5.5 depending on how many pawns there are
    U64 wRooks = getPieces(ROOK, WHITE), bRooks = getPieces(ROOK, BLACK);
    eval += (500 + pawnCount * ROOK_PER_PAWN_BONUS) * (count(wRooks) - count(bRooks));
    // add bonus for open file
    U64 wFile = genNorthMoves(wRooks, emptySquares) | genSouthMoves(wRooks, emptySquares),
        bFile = genNorthMoves(bRooks, emptySquares) | genSouthMoves(bRooks, emptySquares);
    eval += ROOK_OPEN_FILE * (count(wFile) - count(bFile));

    /* Now queen */
    U64 wQueen = getPieces(QUEEN, WHITE), bQueen = getPieces(QUEEN, BLACK);
    eval += (900) * (count(wQueen) - count(bQueen));

    /* Now pawns */
    U64 wPawns = getPieces(PAWN, WHITE), bPawns = getPieces(PAWN, BLACK);
    // add on basic pawn worth's
    eval += (100) * (count(wPawns) - count(bPawns));
    // look for doubled pawns
    for (U64 file: {AFile, BFile, CFile, DFile, EFile, FFile, GFile, HFile}) {
        U64 w = wPawns & file, b = bPawns & file;
        if (count(w) >= 2) eval -= DOUBLED_PAWN;
        if (count(b) >= 2) eval += DOUBLED_PAWN;
    }

    return eval;
}
int SearchState::evaluate(int alpha, int beta) {
    /* this evaluation function is relative to white */

    /* lazy evaluation? */
    int lazy = lazyEval();
    if (lazy >= beta + LAZY_THRESHOLD || lazy <= alpha + LAZY_THRESHOLD) return lazy * (currentSide ? 1: -1);

    /* Special evaluation */
    genAllMoves(ALL_MOVES);
    if (inCheckMate()) return -MATE;
    if (inStalemate() || checkThreefold()) return 0;

    /* now onto the main evaluation */
    // the evaluation function is split up into an opening and endgame evaluation function
    // they are both weighted depending on the pieces that are on the board
    int eval = 0;

    /* Step 1: Calculate attack maps for both sides */
    // make sure white is moving
    U64 wAttacks = attackMap;
    switchSide();
    U64 bAttacks = attackMap;
    switchSide();
    if (currentSide == BLACK) {
        // swap the bitboards if it's black's move;
        U64 temp = wAttacks;
        wAttacks = bAttacks;
        bAttacks = temp;
    }

    /* Step 2: Add on the weighted opening and endgame evaluations */
    eval += (openingEval(wAttacks, bAttacks) * (ENDGAME - PHASE) + endgameEval() * (PHASE)) / ENDGAME;
//    cout << "Phase: " << PHASE << "\n";
//    cout << "Eval: " << eval << "\n";

    /* Step 3: Calculate mobility */
    eval += (count(wAttacks) - count(bAttacks)) * MOBILITY_WEIGHT;

    /* Step 4: Calculate the material balance */
    eval += materialEval();

    /* negamax requires that the evaluation is relative to the current side */
    return eval * (currentSide == WHITE ? -1 : 1);
}
