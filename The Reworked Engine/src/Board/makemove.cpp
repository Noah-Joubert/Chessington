//
// Created by Noah Joubert on 28/07/2023.
//

//TODO: refactor this

#include "board.h"

#ifndef SEARCH_MAKEMOVECPP
#define SEARCH_MAKEMOVECPP

/*
 * These masks give where the kings should be for castling to be allowed.
 * */
constexpr U64 WCASTLEKING = 1152921504606846976, BCASTLEKING = 16;

/*
 * These masks give where the rooks should be for castling to be allowed
 * */
constexpr U64 wLeft = 72057594037927936,
        bLeft = 1,
        wRight = 9223372036854775808,
        bRight = 128;

/*
 * The first mask is used to check that the king and castle are still in the right place for castling
 * The second mask is to check that there aren't any pieces between the rook and king, so we can safely castle
 * */
constexpr U64 CastleMasks[2][2] = {{WCASTLEKING | wLeft, WCASTLEKING | wRight}, {BCASTLEKING | bLeft, BCASTLEKING | bRight}};

enum MoveType {
    Quiet = 0,
    Capture = 1,
    Promotion = 2,
    EnPassant = 3,
    Castle = 4
};
struct DecodedMove {
    short fromType;
    short toType;
    short from;
    short to;
    short promo;
    short flag;
    DecodedMove(Move move) {
        from = move & fromMask;
        to = (move & toMask) >> 6;
        promo = (move & promoMask) >> 12;
        flag = (move & flagMask) >> 14;
        fromType = (move & fromTypeMask) >> 16;
        toType = (move & toTypeMask) >> 19;
    }
};

inline void getCastleSquares(U64 to, short &newRook, short &newKing) {
    if (to & toBB(A1)) {
        newRook = D1;
        newKing = C1;
    }
    if (to & toBB(H1)) {
        newRook = F1;
        newKing = G1;
    }
    if (to & toBB(A8)) {
        newRook = D8;
        newKing = C8;
    }
    if (to & toBB(H8)) {
        newRook = F8;
        newKing = G8;
    }
}

template <MoveType T>
void executeMove(Bitboards &bitboards, Side &currentSide, DecodedMove &move);
template<> void executeMove<Quiet>(Bitboards &bitboards, Side &currentSide, DecodedMove &move) {
    bitboards.setSquare(move.fromType, currentSide, move.from);
    bitboards.setSquare(move.fromType, currentSide, move.to);
}
template<> void executeMove<Capture>(Bitboards &bitboards, Side &currentSide, DecodedMove &move) {
    Side otherSide = currentSide == WHITE ? BLACK : WHITE;

    // reset the from and too square
    bitboards.setSquare(move.fromType, currentSide, move.from);
    bitboards.setSquare(move.toType, otherSide, move.to);

    // set the too square
    bitboards.setSquare(move.fromType, currentSide, move.to);
}
template<> void executeMove<Promotion>(Bitboards &bitboards, Side &currentSide, DecodedMove &move) {
    if (move.toType == EMPTY) {
        /* quiet move */
        executeMove<Quiet>(bitboards, currentSide, move);
    } else {
        /* capture */
        executeMove<Capture>(bitboards, currentSide, move);
    }

    // set the destination square to the promotion piece
    // remove the pawn
    bitboards.setSquare(move.fromType, currentSide, move.to);

    // add in the promoted piece
    short promoPiece = getPromoPiece(move.promo);
    bitboards.setSquare(promoPiece, currentSide, move.to);
}
template<> void executeMove<EnPassant>(Bitboards &bitboards, Side &currentSide, DecodedMove &move) {
    short enPassSquare = move.to + currentSide == WHITE ? 8 : -8;
    Side otherSide = currentSide == WHITE ? BLACK : WHITE;

    // deal with the moving pawn
    bitboards.setSquare(move.fromType, currentSide, move.from);
    bitboards.setSquare(move.fromType, currentSide, move.to);

    // deal with the taken pawn
    bitboards.setSquare(move.toType, otherSide, enPassSquare);
}
template<> void executeMove<Castle>(Bitboards &bitboards, Side &currentSide, DecodedMove &move) {
    // first reset the castle and king squares
    bitboards.setSquare(move.fromType, currentSide, move.from);
    bitboards.setSquare(move.toType, currentSide, move.to);

    short newRook, newKing;
    U64 rook = toBB(move.to); // all 0's from the to square a 1
    getCastleSquares(rook, newRook, newKing);

    // now set the new castle and king squares
    bitboards.setSquare(move.fromType, currentSide, newKing);
    bitboards.setSquare(move.toType, currentSide, newRook);
}
void executeMoveWrapper(Bitboards &bitboards, Side &currentSide, DecodedMove &decodedMove) {
    if (decodedMove.flag == EnPassant) {
        /* en passant */
        executeMove<EnPassant>(bitboards, currentSide, decodedMove);
    } else if (decodedMove.flag == Castle) {
        /* castle */
        executeMove<Castle>(bitboards, currentSide, decodedMove);
    } else if (decodedMove.flag == Promotion) {
        executeMove<Promotion>(bitboards, currentSide, decodedMove);
    } else {
        executeMove<Quiet>(bitboards, currentSide, decodedMove);
    }
}

/* make move */
/* How does make move work?
 * You call the makeMove function, or the unmakeMove function with a legal move.
 */
void Board::makeMove(Move move) {
    /* --- update en-passant rights --- */
    enPassantHistory.emplace_back(bitboards.enPassantRights);
    bitboards.enPassantRights = 0;

    DecodedMove decodedMove(move);
    executeMoveWrapper(bitboards, currentSide, decodedMove);

    /* --- update castle rights --- */
    SpecialMoveRights crights = bitboards.castleRights;
    CastleRightsHistory.emplace_back(crights);
    U64 w = (bitboards.getPieceBB(KING) | bitboards.getPieceBB(ROOK)) & bitboards.getSideBB(WHITE);
    if ((crights & 1) && (w & CastleMasks[WHITE][0]) != CastleMasks[WHITE][0]) {
        crights ^= 1;
    }
    if ((crights & 2) && (w & CastleMasks[WHITE][1]) != CastleMasks[WHITE][1]) {
        crights ^= 2;
    }
    U64 b = (bitboards.getPieceBB(KING) | bitboards.getPieceBB(ROOK)) & bitboards.getSideBB(BLACK);
    if ((crights & 4) && (b & CastleMasks[BLACK][0]) != CastleMasks[BLACK][0]) {
        crights ^= 4;
    }
    if ((crights & 8) && (b & CastleMasks[BLACK][1]) != CastleMasks[BLACK][1]) {
        crights ^= 8;
    }

    bitboards.castleRights = crights;

    /* --- switch the side,, store the move --- */
    switchSide(); // switch the side
    moveHistory.emplace_back(move);
    moveNumber ++;
}
void Board::unmakeMove() {
    /* How does this work
     * 1. Pop the last move from history
     * 2. Reload the previous enPassantRights & castling rights
     * 3. Switch the side back
     * 4. Run the move in reverse
     * */

    Move move = moveHistory.back(); // get the last move played
    moveHistory.pop_back();
    moveNumber--; // decrease the move number

    bitboards.castleRights = CastleRightsHistory.back();
    CastleRightsHistory.pop_back();

    bitboards.enPassantRights = enPassantHistory.back();
    enPassantHistory.pop_back();

    switchSide(); // switch the side

    // decode the move
    DecodedMove decodedMove(move);

    executeMoveWrapper(bitboards, currentSide, decodedMove);
}



#endif SEARCH_MAKEMOVECPP