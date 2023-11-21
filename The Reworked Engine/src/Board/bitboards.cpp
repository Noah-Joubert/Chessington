//
// Created by Noah Joubert on 03/11/2023.
//

#ifndef SEARCH_CPP_BITBOSTFD_H
#define SEARCH_CPP_BITBOSTFD_H

#include "../types.h"

inline short bitScanForward(U64 &BB) {
    // returns the number of leading zeros - ie index of lsb.
    return __builtin_ctzll(BB);
}
inline U64 popLSB(U64 &BB) {
    U64 lsb = BB & -BB; // get the lsb
    BB ^= lsb; // reset the lsb
    return lsb;  // return the lsb
}
inline short popIntLSB(U64 &BB) {
    const short index = bitScanForward(BB);
    BB &= BB - 1; // clear the lsb
    return index;
}
inline U64 toBB(short sq) {
    // converts a square number to a bitboard. ie 3 -> 00000...00100
    return C64(1) << sq;
}
inline short count(U64 x) {
    return (short) __builtin_popcountll(x);
}
void printBitboard(U64 BB) {
    cout << "-----------------\n";
    for (int sq = A8; sq <= H1; sq++) {
        U64 singleBit = C64(1) << sq; // shift a 1 'square' bits right
        if (BB & singleBit) {
            cout << "  *   " << " | ";
        } else {
            cout << "      " << " | ";
        }

        if ((sq + 1) % 8 == 0) {
            cout << "\n";
        }
    }
}

#endif //SEARCH_CPP_BITBOSTFD_H
