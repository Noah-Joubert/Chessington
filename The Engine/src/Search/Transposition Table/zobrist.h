//
// Created by Noah Joubert on 28/07/2023.
//

#include "../../types.h"

#ifndef SEARCH_CPP_ZOBRIST_H
#define SEARCH_CPP_ZOBRIST_H


/* What is a Zobrist hash?
 * It is a key which represents a chess position. They aren't unique, so there is a risk of collisions.
 * There are created by xor-ing together a bunch of random numbers.
 * Each random number represents a property of the chess board e.g. white castle rights.
 * The random numbers are created at initialisation.
 * */

/* Zobrist hashes can either be calculated from scratch, each time.
 * Or updated incrementally whenever a move is made/ un-made.
 * */

/* Zobrist hashes need to be updated when:
 * 1. Making a move
 * 2. Un-making a move (or just store the previous hashes)
 * 3. Changing castle/en-passant rights
 * 4. Setting the FEN
 * 5. Switching sides (or just do this after a move is made)
 * */

typedef U64 Zobrist; // datatype used for zobrist hash

/* These are the random numbers assigned to each property of a chess position. They are created at initialisation. */
Zobrist pieceKeys[12][64]; // generated keys for all the pieces and squares - 2 sides, 6 pieces, 64 squares
Zobrist enPassKeys[8]; // generated keys for all files for en-passant rights
Zobrist castleKeys[4]; // generated keys for all castle rights
Zobrist sideKey[2]; // generated keys for whose side it is

struct PRNG {
    // taken from stockfish - a Pseudo Random Number Generator
    U64 seed = 1070372;

    U64 rand() {
        seed ^= seed >> 12, seed ^= seed << 25, seed ^= seed >> 27;
        return seed * 2685821657736338717LL;
    }
};
void initZobristKeys() {
    // used to init the zobrist keys
    PRNG rng;

    // init the piece keys
    for (short side: {WHITE, BLACK}) {
        for (int pc = PAWN; pc <= KING; pc++) {
            for (int sq = A8; sq <= H1; sq++) {
                pieceKeys[side * 6 + pc][sq] = rng.rand();
            }
        }
    }

    // do the side keys
    sideKey[0] = rng.rand();
    sideKey[1] = rng.rand();

    // init the en-passant keys
    for (short file = 0; file < 8; file ++) {
        enPassKeys[file] = rng.rand();
    }

    // init the castling keys
    for (int i = 0; i < 4; i ++) {
        castleKeys[i] = rng.rand();
    }
}


#endif //SEARCH_CPP_ZOBRIST_H
