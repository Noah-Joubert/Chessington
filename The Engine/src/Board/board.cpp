//
// Created by Noah Joubert on 23/07/2023.
//
#include "bitboards.cpp"
#include "../misc.h"
#include "board.h"
#include "makemove.cpp"
#include "movegen.cpp"

#ifndef FILE_board_SEEN
#define FILE_board_SEEN



/* getters - a lot of these can only be used once moves have been generated */
bool Board::canCastle(short SIDE) {
    if (SIDE == WHITE) {
        return CastleRights & 1 || CastleRights & 2;
    } else if (SIDE == BLACK) {
        return CastleRights & 4 || CastleRights && 8;
    }
}
int Board::getSmallestAttacker(int square) {
    U64 attackers = getSquareAttackers(toBB(square), otherSide);
    attackers &= pieceBB[friendly]; // make sure only friendly piece

    if (attackers == 0) return -1;

    // loop through all pieces in order of worth
    for (int piece = PAWN; piece <= KING; piece++) {
        // get the pieces that are also attackers
        U64 pieceAttackers = pieceBB[piece] & attackers;

        if (pieceAttackers == 0) continue;

        return bitScanForward(pieceAttackers);
    }
}
U64 Board::getEmptySquares() {
    return emptySquares;
}


/* More getters */
U64 Board::getPieces(short PIECE, short SIDE) {
    short sideKey;
    if (SIDE == WHITE) {
        sideKey = nWhite;
    } else {
        sideKey = nBlack;
    }

    return pieceBB[PIECE] & pieceBB[sideKey];
}
short Board::innerGetCurrentSide() {
    return currentSide;
}
short Board::innerGetOtherSide() {
    return otherSide;
}


/* other stuff */
short Board::FENToPieceCode(char c) {
    c = tolower(c);
    switch (c) {
        case 'p':
            return PAWN;
        case 'n':
            return KNIGHT;
        case 'b':
            return BISHOP;
        case 'r':
            return ROOK;
        case 'q':
            return QUEEN;
        case 'k':
            return KING;
    }
    return EMPTY;
}
void Board::readFENInner(string FEN) {
    // This is the inner function called to set the board to a given FEN code.

    /* reset the board */
    for (int i = 0; i < 9; i++) pieceBB[i] = 0;
    occupiedSquares = 0;
    emptySquares = ~0;

    /* split the FEN into it's components */
    string sections[6];
    splitString(FEN, sections, " ", 6);

    /* split up the position into ranks */
    string ranks[8];
    splitString(sections[0], ranks, "/", 8);
    for (int r = 0; r < 8; r ++) {
        // reset the 8 different piece bitboards ~ 6 pawns + 2 colours
        string rank = ranks[r]; // get the rank
        // loop through the characters in the string
        int index = 0;
        for (char c: rank) {
            if (isdigit(c)) {
                index += int(c) - 48;
            } else {
                // add the piece to the board
                short piece = FENToPieceCode(c);
                short square = r * 8 + index;
                Side side = isupper(c) ? WHITE : BLACK;
                setSquare(piece, side, square);
                index ++;
            }
        }
    }

    /* see who's turn it is */
    short side = sections[1] == "w" ? WHITE : BLACK;
    if (side != currentSide) innerSwitchSide();

    /* now deal with castling rights */
    // start of by setting castling rights to 0
    CastleRights = 0;
    for (char c: sections[2]) {
        switch (c) {
            case 'K':
                // white can castle king side
                CastleRights |= 2;
            case 'Q':
                CastleRights |= 1;
            case 'k':
                CastleRights |= 8;
            case 'q':
                CastleRights |= 4;
        }
    }

    /* now deal with en-passant rights */
    string enpassSquare = sections[3];
    U64 enPassantRights = 0;
    if (enpassSquare != "-") {
        enPassantRights = int(enpassSquare[0]) - int('a');
        enPassantRights = toBB(enPassantRights);
    }
    enPassantRights = enPassantRights;



    moveHistory.clear();
    enPassantHistory.clear();
    CastleRightsHistory.clear();
}

/* Setters */
void Board::innerSwitchSide() {
    if (currentSide == WHITE) {
        currentSide = BLACK;
        otherSide = WHITE;
        friendly = nBlack;
        enemy = nWhite;
    } else {
        currentSide = WHITE;
        otherSide = BLACK;
        friendly = nWhite;
        enemy = nBlack;
    }
}

#endif !FILE_board_SEEN