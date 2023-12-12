#include "../types.h"
#include "bitboards.cpp"

#ifndef SEARCH_CPP_NEW_BOARD_H
#define SEARCH_CPP_NEW_BOARD_H

struct Bitboards {
    U64 pieceBB[6];
    U64 sideBB[2];
    U64 EmptySquares = ~0, OccupiedSquares = 0;
    SpecialMoveRights enPassantRights = 0, castleRights = 0;

    void setSquare(short piece, Side side, short sq) {
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
    Pieces getPieceAt(U64 sq) {
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
    void readFEN(string FEN);
    MoveList genMoves();
    void debugPrint();

    /* move gen */
    Bitboards getBitboards() {return this->bitboards;}
    Side getCurrentSide() {return this->currentSide;}
    void setMoveList(MoveList ml) {this->moveList = ml;}
    MoveList getMoveList() { return moveList; }

    /* make move */
    // TODO: refactor this
    void innerMakeMove(Move move);
    void innerUnMakeMove();
    inline void decodeMove(Move move, short &from, short &to, short &promo, short &flag, short &fromType, short &toType);
    inline void clearEnPassRights();
    inline void doEnPass(short &fromType, short &toType, short &from, short &to);
    void updateCastleRights();
    inline void undoCastleRights();
    inline void undoEnPassRights();
    inline void doCastle(short &fromType, short &toType, short &from, short &to);
    inline void doQuiet(short &fromType, short &from, short &to);
    inline void doCapture(short &fromType, short &toType, short &from, short &to);
    inline void setSquare(short &type, Side &side, short &sq);
    inline void innerSwitchSide() {
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
