#include "../types.h"
#include "bitboards.cpp"

#ifndef SEARCH_CPP_NEW_BOARD_H
#define SEARCH_CPP_NEW_BOARD_H

struct Bitboards {
    U64 pieceBB[6];
    U64 sideBB[2];
    U64 EmptySquares = ~0, OccupiedSquares = 0;
    SpecialMoveRights enPassantRights = 0, castleRights = 0;

    void setSquare(Pieces piece, Side side, short sq) {
        U64 square = toBB(sq);

        this->pieceBB[piece] ^= square;
        this->sideBB[side] ^= square;
        this->EmptySquares ^= square;
        this->OccupiedSquares ^= square;
    }
    U64 getPieceBB(Pieces piece) const {
        return this->pieceBB[piece];
    }
    U64 getSideBB(Side side) const {
        return this->sideBB[side];
    }
};

class Board {
private:
    Bitboards bitboards;
    Side currentSide, otherSide;
    MoveList moveList;
public:
    void readFEN(string FEN);
    MoveList genMoves();
    void debugPrint();

    Bitboards getBitboards() {return this->bitboards;}
    Side getCurrentSide() {return currentSide;}
};

#endif //SEARCH_CPP_NEW_BOARD_H
