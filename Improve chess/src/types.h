//
// Created by Noah Joubert on 2021-04-02.
//

#ifndef FILE_TYPES_SEEN
#define FILE_TYPES_SEEN

#include <cstdint>
#include <vector>
#include <iostream>

#define C64(constantU64) constantU64##ULL
using namespace std;

// datatype used for bitboards. A 64 bit integer
typedef uint64_t U64;
typedef uint32_t U32;
typedef uint16_t U16;
typedef uint8_t U8;
typedef int8_t Byte;

/* datatype used for moves. A 32 bit integer
 * bits 0-5 origin square (0-63)
 * bits 6-11 destination (0-63)
 * bits 12-13 promotion piece type (0 = Bishop, 1 = Knight, 2 = Rook, 3 = Queen)
 * bits 14-15 special move flag (1 = promotion, 2 = en passant, 3 = castling)
 * bits 16-18 are the source piece
 * bits 19-21 are the end piece */
typedef uint32_t Move;

/* stores the castling rights */
/* w left: bit 1
 * w right: bit 2
 * b left: bit 3
 * b right: bit 4
*/
typedef uint8_t CRights;
typedef uint8_t EnPassantRights;
typedef vector<short int> Stack;
typedef vector<Move> MoveList;

#define ALL_MOVES 0
#define QUIESENCE_MOVES 1

enum Side {
    WHITE = 0,
    BLACK = 1
};
enum Pieces {
    PAWN = 0,
    KNIGHT = 1,
    BISHOP = 2,
    ROOK = 3,
    QUEEN = 4,
    KING = 5,
    EMPTY = 6,
    nWhite = 7,
    nBlack = 8,
    nPiece = 9
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
enum Direction {
    west = -1,
    east = 1,
    south = 8,
    north = -8,

    noWe = -9,
    noEa = -7,
    soWe = 7,
    soEa = 9
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

// datatype used for zobrist hash
typedef U64 Zobrist;
// taken from stockfish - a Pseudo Random Number Generator
struct PRNG {
    U64 seed = 1070372;

    U64 rand() {
        seed ^= seed >> 12, seed ^= seed << 25, seed ^= seed >> 27;
        return seed * 2685821657736338717LL;
    }
};
Zobrist pieceKeys[12][64]; // generated keys for all the pieces and squares - 2 sides, 6 pieces, 64 squares
Zobrist enPassKeys[8]; // generated keys for all files for en-passant rights
Zobrist castleKeys[4]; // generated keys for all castle rights
Zobrist sideKey[2]; // generated keys for who's side it is
void initZobrist() {
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

// functions needed throughout the program
const string SquareStrings[64] = {"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
                                  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
                                  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
                                  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
                                  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
                                  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
                                  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
                                  "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1"};

// used to time code
class Timer {
    chrono::time_point<chrono::high_resolution_clock> startTime, endTime;
public:
    Timer() {
        startTime = chrono::high_resolution_clock::now();
    }
    float end() {
        endTime = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = endTime - startTime;
        return elapsed.count();
    }
};

#define EXACT_EVAL 1
#define LOWER_EVAL 2
#define UPPER_EVAL 3
typedef uint16_t Zob16; // used to just hold the last 16 bits of a zobrist key to save memory
// stores information about a chess position
struct TTNode {
    Zob16 key = 0; // top half of the zobrist key, used to check for collisions
    Move move = 0; // best move
    U8 depth = 0; // depth search
    U8 flag = 0; // holds whether the evaluation is exact, or an alpha-beta cut off
    U8 age = 0; // holds the half move clock for when this node was added - used for replacing old nodes
    int16_t eval = 0; // evaluation of this node
};
// transposition table
class TranspositionTable {
    // going of off stockfish's source code, this TT will be a an array TTNodes
    // the array will be indexed using the first part of the zobrist key
    // the second part of the zobrist key will be stored to check for collisions

    static const int power2 = 20; // the table will be a power of 2 size
    static const int TTsize = 1 << power2; // size
    static const int keyMask = TTsize - 1; // mask which takes the first first power2 bits of the number

    static const int replaceDepth = 1, replaceAge = 8;

    TTNode table[TTsize];  // array which holds the transposition table

public:
    int total = 0, collisions = 0, overwrites = 0, newNodes = 0;

    inline TTNode* find(Zobrist &key) {
        int index = key & keyMask;

        return &table[index];
    }
    inline TTNode* probe(Zobrist key, bool &found) {
        // get the node
        TTNode* node = find(key);

        // compare the zobrist keys
        found = node->key == toZob16(key);

        return node;
    }
    void set(Zobrist key, Move &move, int &depth, int &flag, short age, int &eval) {
        TTNode* node = find(key);
        Zob16 shiftedKey = toZob16(key);

        total ++;

        if (    (   node->key == 0  ) ||
                (   (node->key != shiftedKey)  &&   (  (depth - node->depth >= replaceDepth)   ||  (age - node->age >= replaceAge)  )) ||
                (   (node->key == shiftedKey)  &&   (depth > node->depth)   ))
        {
            // the node will be overwritten if
                // a. It is empty
                // b. the node has the same zobrist key and the search is to a higher depth
                // c. the node has a different zobrist key and
                    // i. The search is to a significantly higher depth
                    // ii. The search is significantly more recent

            if (node->key == 0) {
                newNodes ++;
            } else if (node->key == shiftedKey) {
                overwrites ++;
            } else {
                collisions ++;
            }

            node->key = shiftedKey;
            node->move = (Move) move;
            node->depth = (U8) depth;
            node->flag = (U8) flag;
            node->age = (U8) age;
            node->eval = (int16_t) eval;
        }
    }
    void resetNode(Zobrist &key) {
        TTNode* node = find(key);
        node = new TTNode();
    }

    inline Zob16 toZob16(Zobrist &key) {
        return (Zob16) (key >> 48);
    }
    void print(TTNode *node);
    void printAll() {
        for (TTNode &node: table) {
            if (node.key != 0) print(&node);
        }
    }
    void clear() {
        for (int i = 0; i < TTsize; i++) {
            TTNode node;
            table[i] = node;
        }
    }
};

#endif /* !FILE_TYPES_SEEN */