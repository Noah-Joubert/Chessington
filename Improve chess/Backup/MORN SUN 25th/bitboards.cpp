//
// Created by Noah Joubert on 2021-04-22.
//
#ifndef FILE_bitboards_SEEN
#define FILE_bitboards_SEEN

#include "types.h"
#include <string>

U64 knightMasks[64]; // masks that give the knight moves for any square
U64 kingMasks[64]; // masks that give the king moves for any square
U64 pawnCaptureMask[2][64]; // masks that give the pawn attacks, first for white attacks, second for black attacks
string SquareStrings[64] = {"A8", "B8", "C8", "D8", "E8", "F8", "G8", "H8",
                            "A7", "B7", "C7", "D7", "E7", "F7", "G7", "H7",
                            "A6", "B6", "C6", "D6", "E6", "F6", "G6", "H6",
                            "A5", "B5", "C5", "D5", "E5", "F5", "G5", "H5",
                            "A4", "B4", "C4", "D4", "E4", "F4", "G4", "H4",
                            "A3", "B3", "C3", "D3", "E3", "F3", "G3", "H3",
                            "A2", "B2", "C2", "D2", "E2", "F2", "G2", "H2",
                            "A1", "B1", "C1", "D1", "E1", "F1", "G1", "H1"};
U64 castleLeftMask, castleRightMask;


// masks for interpreting a move
constexpr Move fromMask = 63, toMask = 4032, promoMask = 12288, flagMask = 49152, fromTypeMask = 458752, toTypeMask = 3670016;

// masks for files
constexpr U64 AFile = 72340172838076673, BFile = AFile << 1, CFile = AFile << 2, DFile = AFile << 3,
        EFile = AFile << 4, FFile = AFile << 5, GFile = AFile << 6, HFile = AFile << 7;
constexpr U64 notAFile = ~AFile, notBFile = ~BFile, notCFile = ~CFile, notDFile = ~DFile, noEFile = ~EFile,
        notFFile = ~FFile, notGFile = ~GFile, notHFile = ~HFile;
constexpr U64 ABFile = AFile | BFile, notABFile = ~ABFile, GHFile = GFile | HFile, notGHFile = ~GHFile;

// masks for ranks
constexpr U64 Rank8 = 255, Rank7 = Rank8 << 8, Rank6 = Rank7 << 8, Rank5 = Rank6 << 8, Rank4 = Rank5 << 8,
        Rank3 = Rank4 << 8, Rank2 = Rank3 << 8, Rank1 = Rank2 << 8;
constexpr U64 notRank8 = ~Rank8, notRank7 = ~Rank7, notRank6 = ~Rank6, notRank5 = ~Rank5, notRank4 = ~Rank4,
        notRank3 = ~Rank3, notRank2 = ~Rank2, notRank1 = ~Rank1;

U64 initKnightPattern(U64 knight) {
    U64 moves = C64(0);

    // get one left moves
    moves |= ((knight >> 17) | (knight << 15)) & notHFile;

    // get two left moves
    moves |= ((knight >> 10) | (knight << 6)) & (notGHFile);

    // get one right move
    moves |= ((knight >> 15) | (knight << 17)) & notAFile;

    // get two right moves
    moves |= ((knight >> 6) | (knight << 10)) & (notABFile);

    return moves;
}
U64 initKingPattern(U64 king) {
    U64 moves = C64(0);

    moves |= ((king >> 1) | (king >> 9) | (king << 7)) & notHFile;
    moves |= ((king << 1) | (king << 9) | (king >> 7)) & notAFile;
    moves |= (king >> 8) | (king << 8);

    return moves;
}
U64 initPawnAttackPattern(U64 pawn, unsigned short side) {
    U64 move = C64(0);

    if (side == WHITE) {
        // generate the takes
        move |= ((pawn >> 9) & notHFile) | ((pawn >> 7) & notAFile);
    } else {
        // generate the takes
        move |= (((pawn << 9) & notAFile) | ((pawn << 7) & notHFile));
    }

    return move;
}
void initStaticMasks() {
    for (int a = A8; a <= H1; a++) {
        knightMasks[a] = initKnightPattern(C64(1) << a);
        kingMasks[a] = initKingPattern(C64(1) << a);
        pawnCaptureMask[WHITE][a] = initPawnAttackPattern(C64(1) << a, WHITE);
        pawnCaptureMask[BLACK][a] = initPawnAttackPattern(C64(1) << a, BLACK);
    }

    castleLeftMask = C64(1) << A1 | C64(1) << A8;
    castleRightMask = C64(1) << H1 | C64(1) << H8;
}

inline unsigned short bitScanForward(U64 &BB) {
    // returns the number of leading zeros - ie index of lsb.
    return __builtin_ctzll(BB);
}
inline unsigned short popIntLSB(U64 &BB) {
    const unsigned short index = bitScanForward(BB);
    BB &= BB - 1; // clear the lsb
    return index;
}
inline U64 popLSB(U64 &BB) {
    U64 lsb = BB & -BB; // get the lsb
    BB ^= lsb; // reset the lsb
    return lsb;  // return the lsb
}
inline unsigned short count(U64 x) {
    return (unsigned short) __builtin_popcountll(x);
}

#endif /* !FILE_TYPES_SEEN */