#ifndef SEARCH_CPP_TYPES_H
#define SEARCH_CPP_TYPES_H

#include <iostream>
#include <cstdint>
#include <vector>

#define C64(constantU64) constantU64##ULL

using namespace std;

/* datatype used for moves. A 32 bit integer
 * bits 0-5 origin square (0-63)
 * bits 6-11 destination (0-63)
 * bits 12-13 promotion piece type (0 = Bishop, 1 = Knight, 2 = Rook, 3 = Queen)
 * bits 14-15 special move flag (1 = promotion, 2 = en passant, 3 = castling)
 * bits 16-18 are the source piece
 * bits 19-21 are the end piece
 * I'll detail how special moves are encoded
 * Castling: A move from king square to the rook square, with type king and rook
 * En-Passant: A move from the taking pawn square to it's empty destination square, with type pawn to pawn
 * Promotion: The promotion flag will be set to the new piece
 * */
/*
 * These masks are for decoding a Move bitboard.
 */
typedef uint32_t Move;
typedef uint64_t U64;
typedef vector<Move> MoveList;
typedef uint8_t SpecialMoveRights;

/* Masks for decoding a move bitboard */
constexpr Move fromMask = 63, toMask = 4032, promoMask = 12288, flagMask = 49152, fromTypeMask = 458752, toTypeMask = 3670016;

enum Side {
    WHITE = 0,
    BLACK = 1
};
enum MoveCode {
    BISHOPPROMO = 0,
    KNIGHTPROMO = 1,
    ROOKPROMO = 2,
    QUEENPROMO = 3,

    PROMOTION = 1,
    ENPASSANT = 2,
    CASTLING = 3
};
enum Square {
    // the board is flipped
    A8, B8, C8, D8, E8, F8, G8, H8,
    A7, B7, C7, D7, E7, F7, G7, H7,
    A6, B6, C6, D6, E6, F6, G6, H6,
    A5, B5, C5, D5, E5, F5, G5, H5,
    A4, B4, C4, D4, E4, F4, G4, H4,
    A3, B3, C3, D3, E3, F3, G3, H3,
    A2, B2, C2, D2, E2, F2, G2, H2,
    A1, B1, C1, D1, E1, F1, G1, H1
};
enum Pieces {
    ENPASSANTPAWNS = -3,
    PROMOPAWNS = -2,
    NONPROMOPAWNS = -1,
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    EMPTY = 6
};
inline Pieces getPromoPiece(short promoCode) {
    if (promoCode == KNIGHTPROMO) return KNIGHT;
    if (promoCode == BISHOPPROMO) return BISHOP;
    if (promoCode == ROOKPROMO) return ROOK;
    if (promoCode == QUEENPROMO) return QUEEN;
}

#endif //SEARCH_CPP_TYPES_H
