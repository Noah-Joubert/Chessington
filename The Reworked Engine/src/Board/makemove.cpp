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

/* make move */
/* How does make move work?
 * You call the innerMakeMove function, or the innerUnMakeMove function with a legal move.
 */
void Board::innerMakeMove(Move move) {
    // inner function used to make a move

    /* How does this work?
     * 1. Add the move to the moveHistory. Increment moveNumber.
     * 2. Clear the enPassantRights
     * 3. Execute the move, considering what type of move it is (eg. pawn push, capture, promotion)
     * 4. Update castling rights
     * 5. Switch side
     */

    moveHistory.emplace_back(move);
    moveNumber ++;
    clearEnPassRights();

    short from, to, promo, flag, fromType, toType;
    decodeMove(move, from, to, promo, flag, fromType, toType);

    if (flag == ENPASSANT) {
        /* en passant */
        doEnPass(fromType, toType, from, to);
    } else if (flag == CASTLING) {
        /* castle */
        doCastle(fromType, toType, from, to);
    } else {
        // normal move
        if (toType == EMPTY) {
            /* quiet move */
            doQuiet(fromType, from, to);

            // check for double pawn push and update en-passant rights
            if ((fromType == PAWN) && ((from - to) % 16 == 0)) {
                short file = to % 8;
                bitboards.enPassantRights ^= toBB(file);
            }
        } else {
            /* capture */
            doCapture(fromType, toType, from, to);
        }

        // if it was a promotion, set the destination square to the promotion piece
        if (flag == PROMOTION) {
            // remove the pawn
            setSquare(fromType, currentSide, to);

            // add in the promoted piece
            short promoPiece = getPromoPiece(promo);
            setSquare(promoPiece, currentSide, to);
        }
    }

    updateCastleRights();
    innerSwitchSide(); // switch the side
}
void Board::innerUnMakeMove() {
    // inner function used to un-make a move

    /* How does this work
     * 1. Pop the last move from history
     * 2. Reload the previous enPassantRights & castling rights
     * 3. Switch the side back
     * 4. Run the move in reverse
     * */

    Move move = moveHistory.back(); // get the last move played
    moveHistory.pop_back();
    moveNumber--; // decrease the move number

    undoEnPassRights();
    undoCastleRights();

    innerSwitchSide(); // switch the side

    // decode the move
    short from, to, promo, flag, fromType, toType;
    decodeMove(move, from, to, promo, flag, fromType, toType);

    if (flag == ENPASSANT) {
        /* en passant */
        doEnPass(fromType, toType, from, to);

    } else if (flag == CASTLING) {
        /* castle */
        doCastle(fromType, toType, from, to);

    } else {
        // normal move

        // if it's a promotion, turn the to square back to a pawn
        if (flag == PROMOTION) {
            // take out the promoted piece

            short promoPiece = getPromoPiece(promo);

            setSquare(fromType, currentSide, to);
            setSquare(promoPiece, currentSide, to);

        }
        if (toType == EMPTY) {
            /* quiet move */
            doQuiet(fromType, from, to);
        } else {
            /* capture */
            doCapture(fromType, toType, from, to);
        }
    }
}
inline void Board::decodeMove(Move move, short &from, short &to, short &promo, short &flag, short &fromType, short &toType) {
    from = move & fromMask;
    to = (move & toMask) >> 6;
    promo = (move & promoMask) >> 12;
    flag = (move & flagMask) >> 14;
    fromType = (move & fromTypeMask) >> 16;
    toType = (move & toTypeMask) >> 19;
}
inline void Board::clearEnPassRights() {
    // update zobrist key by removing any previous en-passants.
    enPassantHistory.emplace_back(bitboards.enPassantRights);
    bitboards.enPassantRights = 0;
}
inline void Board::doEnPass(short &fromType, short &toType, short &from, short &to) {
    short enPassSquare;

    if (currentSide == WHITE) {
        enPassSquare = to + 8;
    } else {
        enPassSquare = to - 8;
    }

    // deal with the moving pawn
    setSquare(fromType, currentSide, from);
    setSquare(fromType, currentSide, to);

    // deal with the taken pawn
    setSquare(toType, otherSide, enPassSquare);
}
void Board::updateCastleRights() {
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
}
inline void Board::undoCastleRights() {
    bitboards.castleRights = CastleRightsHistory.back();
    CastleRightsHistory.pop_back();
}
inline void Board::undoEnPassRights() {
    bitboards.enPassantRights = enPassantHistory.back();
    enPassantHistory.pop_back();
}
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
inline void Board::doCastle(short &fromType, short &toType, short &from, short &to) {
    // first reset the castle and king squares
    setSquare(fromType, currentSide, from);
    setSquare(toType, currentSide, to);

    short newRook, newKing;
    U64 rook = toBB(to); // all 0's from the to square a 1
    getCastleSquares(rook, newRook, newKing);

    // now set the new castle and king squares
    setSquare(fromType, currentSide, newKing);
    setSquare(toType, currentSide, newRook);
}
inline void Board::doQuiet(short &fromType, short &from, short &to) {
    setSquare(fromType, currentSide, from);
    setSquare(fromType, currentSide, to);
}
inline void Board::doCapture(short &fromType, short &toType, short &from, short &to) {
    // reset the from and too square
    setSquare(fromType, currentSide, from);
    setSquare(toType, otherSide, to);

    // set the too square
    setSquare(fromType, currentSide, to);
}
inline void Board::setSquare(short &type, Side &side, short &sq) {
    bitboards.setSquare(type, side, sq);
}

#endif SEARCH_MAKEMOVECPP