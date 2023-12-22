#include <utility>

#include "types.h"
#include "bitboards.cpp"

#ifndef SEARCH_CPP_NEW_BOARD_H
#define SEARCH_CPP_NEW_BOARD_H

struct Bitboards {
    U64 pieceBB[6];
    U64 sideBB[2];
    U64 EmptySquares = ~0, OccupiedSquares = 0;
    SpecialMoveRights enPassantRights = 0, castleRights = 0;

    inline void setSquare(short piece, Side side, short sq) {
        U64 square = toBB(sq);

        this->pieceBB[piece] ^= square;
        this->sideBB[side] ^= square;
        this->EmptySquares ^= square;
        this->OccupiedSquares ^= square;
    }
    inline U64 getPieceBB(Pieces piece) const {
        return this->pieceBB[piece];
    }
    inline U64 getSideBB(Side side) const {
        return this->sideBB[side];
    }
    inline Pieces getPieceAt(U64 sq) {
        if (pieceBB[PAWN] & sq) return PAWN;
        if (pieceBB[KNIGHT] & sq) return KNIGHT;
        if (pieceBB[BISHOP] & sq) return BISHOP;
        if (pieceBB[ROOK] & sq) return ROOK;
        if (pieceBB[QUEEN] & sq) return QUEEN;
        if (pieceBB[KING] & sq) return KING;
        assert("No piece here ~_~");
    }
};

class Board {
private:
    /* move gen */
    Bitboards bitboards;
    Side currentSide, otherSide;
    MoveList moveList;

    /* make move */
    MoveList moveHistory;
    vector<SpecialMoveRights> enPassantHistory; // stores past en-passant rights
    vector<SpecialMoveRights> CastleRightsHistory; // stores previous castle rights
    int moveNumber;
public:
    Board ();
    void debugPrint();

    /* getters */
    Bitboards getBitboards() {return this->bitboards;}

    /* setters */
    void readFEN(string FEN);

    /* move gen */
    MoveList genMoves();

    /* make move */
    void makeMove(Move move);
    void unmakeMove();
    inline void switchSide() {
        if (currentSide == WHITE) {
            currentSide = BLACK;
            otherSide = WHITE;
        } else {
            currentSide = WHITE;
            otherSide = BLACK;
        }
    }
};

#endif //SEARCH_CPP_NEW_BOARD_H
