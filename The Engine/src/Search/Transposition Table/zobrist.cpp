//
// Created by Noah Joubert on 28/07/2023.
//

#include "../SearchController.h"

#ifndef SEARCH_CPP_ZOBRIST_CPP
#define SEARCH_CPP_ZOBRIST_CPP

bool SearchController::validateZobrist() {
    // this is used to see whether the incrementally calculated zobrist hash is equal to the one calculated from scratch

    Zobrist created = calculateZobristHash();
    return zobristState == created;
}
Zobrist SearchController::calculateZobristHash() {
    // recalculates the zobrist hash from scratch

    Zobrist key = 0;

    // first do pieces
    for (short pc = PAWN; pc <= KING; pc ++) {
        vector<short> w_pc = toArray(pieceBB[pc] & pieceBB[nWhite]);
        vector<short> b_pc = toArray(pieceBB[pc] & pieceBB[nBlack]);

        for (short sq: w_pc) {key ^= pieceKeys[pc][sq];}
        for (short sq: b_pc) {key ^= pieceKeys[pc + 6][sq];}
    }

    // now do current player
    key ^= sideKey[currentSide];

    // next do en-pass rights
    for (int i = 0; i < 8; i++) {
        if (enPassantRights & toBB(i)) {
            key ^= enPassKeys[i];
            break;
        }
    }

    // next do castling rights
    for (int i = 0; i < 4; i++) {
        if (CastleRights & toBB(i)) {key ^= castleKeys[i];}
    }

    return key;
}
void SearchController::calculateAndSetZobristHash() {
    // recalculates the zobrist hash from scratch, and sets it as the current hash
    zobristState = calculateZobristHash();
    prevZobristStates.emplace_back(zobristState);
}
void SearchController::updateMoveZobrist(Move move) {
    /* Updates the zobrist hash after a move has been made.
     * Switching the side is done separately, as we may want to switch sides without making a move
     * */
    short from, to, promo, flag, fromType, toType;
    decodeMove(move, from, to, promo, flag, fromType, toType);

    if (flag == ENPASSANT) {
        // xor in and out our pawn
        zobristXOR(PAWN, from, currentSide);
        zobristXOR(PAWN, to, currentSide);

        // xor out the taken pawn
        short enPassSquare;
        if (currentSide == WHITE) {
            enPassSquare = to + 8;
        } else {
            enPassSquare = to - 8;
        }
        zobristXOR(PAWN, enPassSquare, otherSide); // xor out the taken pawn
    } else if (flag == CASTLING) {
        // xor out the castle and king from their old positions
        zobristXOR(KING, from, currentSide); // xor out the king
        zobristXOR(ROOK, to, currentSide); // xor out the rook

        // xor them into their new positions
        short newRook, newKing;
        U64 rook = toBB(to); // all 0's from the to square a 1
        getCastleSquares(rook, newRook, newKing, currentSide);
        zobristXOR(KING, newKing, currentSide); // xor out the king
        zobristXOR(ROOK, newRook, currentSide); // xor out the rook
    } else {
        zobristXOR(fromType, from, currentSide); // xor out the start square, start player

        // xor out the end square, if occupied
        if (toType != EMPTY) {
            zobristXOR(toType, to, otherSide); // xor out the end square, end player
        }

        if (flag == PROMOTION) {
            fromType = getPromoPiece(promo);
        }

        // xor out the end square, start player.
        zobristXOR(fromType, to, currentSide);
    }
}
void SearchController::updateEnPassZobrist() {
    // xor out en-pass rights
    for (int i = 0; i < 8; i++) {
        if (enPassantRights & toBB(i)) {
            zobristState ^= enPassKeys[i];
            break;
        }
    }
}
void SearchController::updateCastlingZobrist() {
    // xor out the castle rights into the zobrist hash
    for (int i = 0; i < 4; i++) {
        if (CastleRights & toBB(i)) {zobristState ^= castleKeys[i];}
    }
}
inline void SearchController::zobristXOR(short piece, short square, Side side) {
    // XOR's a piece at a certain square
    zobristState ^= pieceKeys[piece + 6 * side][square];
}
void SearchController::updateSideZobrist() {
    /* Updates the zobrist hash after switching sides */
    zobristState ^= sideKey[currentSide];
    zobristState ^= sideKey[otherSide];
}

#endif SEARCH_CPP_ZOBRIST_CPP
