//
// Created by Noah Joubert on 2021-05-09.
//

#include "../SearchController.h"
#include "evaluation.h"


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
int SearchController::evaluate() {
    /* Right now it just does piece worth's */

    /* negamax requires that the evaluation is relative to the current side */
    //TODO THIS
    return materialEvaluation * (currentSide == WHITE ? 1 : -1);
}

int SearchController::relativeLazy() {
    /* Right now it just does piece worth's */

    /* negamax requires that the evaluation is relative to the current side */
    //TODO THIS
    return materialEvaluation * (currentSide == WHITE ? 1 : -1);
}
int SearchController::biasedMaterial() {
    /* this evaluation function is relative to white */

    int eval = 0;

    /* Start with knights */
    U64 wKnights = getPieces(KNIGHT, WHITE), bKnights = getPieces(KNIGHT, BLACK);
    eval += PieceScores[KNIGHT] * (count(wKnights) - count(bKnights));

    /* Now bishops */
    U64 wBishops = getPieces(BISHOP, WHITE), bBishops = getPieces(BISHOP, BLACK);
    eval += PieceScores[BISHOP] * (count(wBishops) - count(bBishops));

    /* Now rooks */
    U64 wRooks = getPieces(ROOK, WHITE), bRooks = getPieces(ROOK, BLACK);
    eval += PieceScores[ROOK] * (count(wRooks) - count(bRooks));

    /* Now queen */
    U64 wQueen = getPieces(QUEEN, WHITE), bQueen = getPieces(QUEEN, BLACK);
    eval += PieceScores[QUEEN] * (count(wQueen) - count(bQueen));

    /* Now pawns */
    U64 wPawns = getPieces(PAWN, WHITE), bPawns = getPieces(PAWN, BLACK);
    eval += PieceScores[PAWN] * (count(wPawns) - count(bPawns));

    /* do PST */
    for (Pieces piece: {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
        for (Side side: {WHITE, BLACK}) {
            eval += dotProduct(getPieces(piece, side), PST[piece][side]) * (side == WHITE ? 1 : -1);
        }
    }

    //TODO add in endgame king stuff

    /* negamax requires that the evaluation is relative to the current side */
    return eval;
}
