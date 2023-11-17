//
// Created by Noah Joubert on 03/11/2023.
//

#ifndef SEARCH_CPP_BITBOSTFD_H
#define SEARCH_CPP_BITBOSTFD_H

#include "types.h"

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

#endif //SEARCH_CPP_BITBOSTFD_H
