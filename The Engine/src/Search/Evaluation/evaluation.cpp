//
// Created by Noah Joubert on 2021-05-09.
//

#include "../SearchController.h"
#include "evaluation.h"

int SearchController::evaluate() {
    /* Right now it just does piece worth's */

    /* negamax requires that the evaluation is relative to the current side */
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

    /* negamax requires that the evaluation is relative to the current side */
    return eval ;
}
