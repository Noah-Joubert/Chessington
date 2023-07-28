//
// Created by Noah Joubert on 28/07/2023.
//

#include "../types.h"

#ifndef SEARCH_CPP_ZOBRIST_H
#define SEARCH_CPP_ZOBRIST_H

/* What is a Zobrist hash?
 * It is a key which represents a chess position. They aren't unique, so there is a risk of collisions.
 * There are created by xor-ing together a bunch of random numbers.
 * Each random number represents a property of the chess board e.g. white castle rights.
 * The random numbers are created at initialisation.
 * */

typedef U64 Zobrist; // datatype used for zobrist hash

/* These are the random numbers assigned to each property of a chess position. They are created at initialisation. */
Zobrist pieceKeys[12][64]; // generated keys for all the pieces and squares - 2 sides, 6 pieces, 64 squares
Zobrist enPassKeys[8]; // generated keys for all files for en-passant rights
Zobrist castleKeys[4]; // generated keys for all castle rights
Zobrist sideKey[2]; // generated keys for whose side it is

// taken from stockfish - a Pseudo Random Number Generator
struct PRNG {
    U64 seed = 1070372;

    U64 rand() {
        seed ^= seed >> 12, seed ^= seed << 25, seed ^= seed >> 27;
        return seed * 2685821657736338717LL;
    }
};

#endif //SEARCH_CPP_ZOBRIST_H
