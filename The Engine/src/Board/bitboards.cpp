//
// Created by Noah Joubert on 2021-04-22.
//
#ifndef FILE_bitboards_SEEN
#define FILE_bitboards_SEEN

#include "../types.h"
#include <string>
#include <vector>

/*
 * These masks give the possible knight/ king/ pawn masks for each square
 */
U64 knightMasks[64];
U64 kingMasks[64];
U64 pawnCaptureMask[2][64]; // first for white attacks, second for black attacks
U64 castleLeftMask, castleRightMask;

/*
 * These masks are for decoding a Move bitboard.
 */
constexpr Move fromMask = 63, toMask = 4032, promoMask = 12288, flagMask = 49152, fromTypeMask = 458752, toTypeMask = 3670016;

/*
 * These masks give each file
 * */
constexpr U64 AFile = 72340172838076673, BFile = AFile << 1, CFile = AFile << 2, DFile = AFile << 3,
        EFile = AFile << 4, FFile = AFile << 5, GFile = AFile << 6, HFile = AFile << 7;
constexpr U64 notAFile = ~AFile, notBFile = ~BFile, notCFile = ~CFile, notDFile = ~DFile, noEFile = ~EFile,
        notFFile = ~FFile, notGFile = ~GFile, notHFile = ~HFile;
constexpr U64 ABFile = AFile | BFile, notABFile = ~ABFile, GHFile = GFile | HFile, notGHFile = ~GHFile;

/*
 * These masks give each ranks
 * */
constexpr U64 Rank8 = 255, Rank7 = Rank8 << 8, Rank6 = Rank7 << 8, Rank5 = Rank6 << 8, Rank4 = Rank5 << 8,
        Rank3 = Rank4 << 8, Rank2 = Rank3 << 8, Rank1 = Rank2 << 8;
constexpr U64 notRank8 = ~Rank8, notRank7 = ~Rank7, notRank6 = ~Rank6, notRank5 = ~Rank5, notRank4 = ~Rank4,
        notRank3 = ~Rank3, notRank2 = ~Rank2, notRank1 = ~Rank1;

/*
 * These masks give where the kings should be for castling to be allowed.
 * */
constexpr U64 WCASTLEKING = 1152921504606846976, BCASTLEKING = 16;

/*
 * These masks give where the rooks should be for castling to be allowed
 * */
constexpr U64 wLeft = 72057594037927936,
        bLeft = 1,
        wRight = 9223372036854775808,
        bRight = 128;

/*
 * The first mask is used to check that the king and castle are still in the right place for castling
 * The second mask is to check that there aren't any pieces between the rook and king, so we can safely castle
 * */
constexpr U64 CastleMasks[2][2] = {{WCASTLEKING | wLeft, WCASTLEKING | wRight}, {BCASTLEKING | bLeft, BCASTLEKING | bRight}};
constexpr U64 ClearCastleLaneMasks[2][2] = {{1008806316530991104, 6917529027641081856}, {14, 96}};

/*
 * These functions are ran once to initialise the static masks
 * */
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
U64 initPawnAttackPattern(U64 pawn, short side) {
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

/*
 * These functions are used often for bitboard manipulation
 */
inline U64 toBB(short sq) {
    // converts a square number to a bitboard. ie 3 -> 00000...00100
    return C64(1) << sq;
}
inline short bitScanForward(U64 &BB) {
    // returns the number of leading zeros - ie index of lsb.
    return __builtin_ctzll(BB);
}
inline short popIntLSB(U64 &BB) {
    const short index = bitScanForward(BB);
    BB &= BB - 1; // clear the lsb
    return index;
}
inline short pop8BitIntLSB(uint8_t &BB) {
    if (BB & toBB(0)) return 0;
    if (BB & toBB(1)) return 1;
    if (BB & toBB(2)) return 2;
    if (BB & toBB(3)) return 3;
    if (BB & toBB(4)) return 4;
    if (BB & toBB(5)) return 5;
    if (BB & toBB(6)) return 6;
    if (BB & toBB(7)) return 7;
}
inline U64 popLSB(U64 &BB) {
    U64 lsb = BB & -BB; // get the lsb
    BB ^= lsb; // reset the lsb
    return lsb;  // return the lsb
}
inline short count(U64 x) {
    return (short) __builtin_popcountll(x);
}
inline vector<short> toArray(U64 BB) {
    vector<short> vec;
    while (BB) vec.emplace_back(popIntLSB(BB));
    return vec;
}

/*
 * These functions shift a bitboard, either by a given direction, or forward/ backwards for pawns.
 * */
inline U64 push(U64 BB, short side) {
    return side == WHITE ? (BB >> 8) : (BB << 8);
}
inline U64 shift(U64 BB, short D) {
    return D == noEa ? (BB >> 7) & notAFile:
           D == noWe ? (BB >> 9) & notHFile:
           D == soEa ? (BB << 9) & notAFile:
           D == soWe ? (BB << 7) & notHFile:
           D == north ? (BB >> 8):
           D == south ? (BB << 8):
           0;
}

/*
 * getPromoPiece converts between promoCodes and Pieces.
 * getCastleSquares is used to return the squares where the king and rook should move after a castle
 * */
inline short getPromoPiece(short promoCode) {
    if (promoCode == KNIGHTPROMO) return KNIGHT;
    if (promoCode == BISHOPPROMO) return BISHOP;
    if (promoCode == ROOKPROMO) return ROOK;
    if (promoCode == QUEENPROMO) return QUEEN;
}
inline void getCastleSquares(U64 &to, short &newRook, short &newKing, Side &currentSide) {
    if (currentSide == WHITE) {
        if (to & castleLeftMask) {
            newRook = D1;
            newKing = C1;
        } else {
            newRook = F1;
            newKing = G1;
        }
    } else {
        if (to & castleLeftMask) {
            newRook = D8;
            newKing = C8;
        } else {
            newRook = F8;
            newKing = G8;
        }
    }
}

#endif !FILE_bitboards_SEEN