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
/* I'll detail how special moves are encoded
 * Castling: A move from king square to the rook square, with type king and rook
 * En-Passant: A move from the taking pawn square to it's empty destination square, with type pawn to pawn
 * Promotion: The promotion flag will be set to the new piece
 * */
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

const string initialFEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

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

#endif /* !FILE_TYPES_SEEN */