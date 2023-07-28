//
// Created by Noah Joubert on 2021-04-02.
//

#ifndef FILE_TYPES_SEEN
#define FILE_TYPES_SEEN

#include <cstdint>
#include <vector>
#include <mmintrin.h>
#define C64(constantU64) constantU64##ULL
using namespace std;

// datatype used for bitboards. A 64 bit integer
typedef uint64_t U64;

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

template <typename T>
struct MyDataStructure {
    T *data;
    int last = -1; // stores number of items currently stored -1
    int max = -1; // stores max number of items

    void reserve(int sizeIn) {
        data = new T[sizeIn];
        max = sizeIn;
    }
    void emplace_back(T dataIn) {
//        static_assert(size == -1, "Data structure didn't have size set");
        last ++;
        data[last] = dataIn;
    }
    T back() {
        return data[last];
    }
    void pop_back() {
        last -= 1;
    }
    void clear() {
        last = -1;
    }
    T at(int i) {
        return data[i];
    }
    bool empty() {
        return last == -1;
    }
    void insert(MyDataStructure newData) {
        for (int i = 0; i <= newData.last; i++) {
            emplace_back(newData.at(i));
        }
    }
    short int size() {
        return last + 1;
    }
};

struct CastleRights {
    U64 king, left, right;

    U64 leftMask, rightMask;

    int leftBreak = -2, rightBreak = -2, kingBreak = -2;
};

typedef uint8_t EnPassantRights;
typedef vector<short int> Stack;
typedef vector<Move> MoveList;
typedef vector<EnPassantRights> EnPassantRightStack;

enum Side {
    WHITE = 0,
    BLACK = 1
};
enum Pieces {
    EMPTY = 0,
    PAWN = 1,
    KNIGHT = 2,
    BISHOP = 3,
    ROOK = 4,
    QUEEN = 5,
    KING = 6,
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
enum CastleMask {
    /* king masks */
    wKing = 1152921504606846976,
    bKing = 16,

    /* rook masks */
    wLeft = 72057594037927936,
    bLeft = 1,
    wRight = 9223372036854775808,
    bRight = 128
};

#endif /* !FILE_TYPES_SEEN */