//
// Created by Noah Joubert on 2021-04-22.
//
#include "types.h"
#include "bitboards.cpp"
#include "position.cpp"
#include "misc.cpp"

inline U64 genNorthMoves(U64 generatingPieces, U64 &emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 8) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    return flood >> 8;
}
inline U64 genSouthMoves(U64 generatingPieces, U64 &emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 8) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    return flood << 8;
}
inline U64 genWestMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notHFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 1) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood >> 1;

    // reset any flood squares that have been circular shifted
    flood &= notHFile;

    return flood;
}
inline U64 genEastMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notAFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 1) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood << 1;

    // reset any flood squares that have been circular shifted
    flood &= notAFile;

    // shift once more to include the blocker, and exclude the start square
    return flood;
}
inline U64 genNWMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notHFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 9) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood >> 9;

    // reset any flood squares that have been circular shifted
    flood &= notHFile;

    return flood;
}
inline U64 genSWMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notHFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 7) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood << 7;

    // reset any flood squares that have been circular shifted
    flood &= notHFile;

    return flood;
}
inline U64 genNEMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notAFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces >> 7) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood >> 7;

    // reset any flood squares that have been circular shifted
    flood &= notAFile;

    return flood;
}
inline U64 genSEMoves(U64 generatingPieces, U64 emptySquares) {
    // get the flooded bit board of north moves
    U64 flood = 0; // set of possible locations

    // apply the blocker to the H file to stop wrap around's
    emptySquares &= notAFile;

    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;
    generatingPieces = (generatingPieces << 9) & emptySquares;
    flood |= generatingPieces;

    // shift once more to include the blocker, and exclude the start square
    flood = flood << 9;

    // reset any flood squares that have been circular shifted
    flood &= notAFile;

    return flood;
}

// This generates a bitboard of squares attacked by any piece (bar pawns) ~ this includes non-legal attacks however.
// The returned bitboard includes friendly/ enemy/ and empty squares
// It can be used to generate moves for all pieces bar pawns.
template<unsigned short T>
inline U64 genAttack(U64 generatingPiece, U64 &emptySquares);
template<> inline U64 genAttack<ROOK>(U64 generatingRook, U64 &emptySquares) {
    return (genNorthMoves(generatingRook, emptySquares) | genSouthMoves(generatingRook, emptySquares) |
            genWestMoves(generatingRook, emptySquares) | genEastMoves(generatingRook, emptySquares));
};
template<> inline U64 genAttack<BISHOP>(U64 generatingBishop, U64 &emptySquares) {
    return (genNWMoves(generatingBishop, emptySquares) | genNEMoves(generatingBishop, emptySquares) |
            genSEMoves(generatingBishop, emptySquares) | genSWMoves(generatingBishop, emptySquares));
}
template<> inline U64 genAttack<QUEEN>(U64 generatingQueen, U64 &emptySquares) {
    return genAttack<ROOK>(generatingQueen, emptySquares) | genAttack<BISHOP>(generatingQueen, emptySquares);
}
template<> inline U64 genAttack<KNIGHT>(U64 generatingKnight, U64 &emptySquares) {
    return knightMasks[bitScanForward(generatingKnight)];
}
template<> inline U64 genAttack<KING>(U64 generatingKing, U64 &emptySquares) {
    return kingMasks[bitScanForward(generatingKing)];
}

// pawn move generators
U64 genEnPassantAttacks(U64 generatingPawn, U64 enPassantRights, unsigned short SIDE) {
    enPassantRights |= (enPassantRights << (8 * 2)) | (enPassantRights << (8 * 5));
    U64 allowedRank;
    if (SIDE == WHITE) {
        allowedRank = Rank6;
    } else {
        allowedRank = Rank3;
    }

    return pawnCaptureMask[SIDE][bitScanForward(generatingPawn)] & enPassantRights & allowedRank;
}
U64 genLeftPawnAttack(U64 generatingPawn, unsigned short side) {
    U64 move = 0;

    if (side == WHITE) {
        // generate the takes
        move |= (generatingPawn >> 9) & notHFile;
    } else {
        // generate the takes
        move |= (generatingPawn << 7) & notHFile;
    }

    return move;
}
U64 genRightPawnAttack(U64 generatingPawn, unsigned short side) {
    U64 move = 0;

    if (side == WHITE) {
        // generate the takes
        move |= (generatingPawn >> 7) & notAFile;
    } else {
        // generate the takes
        move |= (generatingPawn << 9) & notAFile;
    }

    return move;
}
U64 genPawnAttacks(U64 generatingPawn, unsigned short SIDE) {
    return pawnCaptureMask[SIDE][bitScanForward(generatingPawn)];
}
U64 genPawnPushes(U64 generatingPawn, U64 &emptySquares, unsigned short SIDE) {
    U64 pushes = 0;

    if (SIDE == WHITE) {
        // generate the first step
        generatingPawn = (generatingPawn >> 8) & emptySquares; // see if the next square is empty
        pushes |= generatingPawn;

        // generate the second step
        generatingPawn = (generatingPawn >> 8) & emptySquares &
                         Rank4; // see if the next square is empty, check for double push
        pushes |= generatingPawn;
    } else {
        // generate the first step
        generatingPawn = (generatingPawn << 8) & emptySquares; // see if the next square is empty
        pushes |= generatingPawn;

        // generate the second step
        generatingPawn = (generatingPawn << 8) & emptySquares &
                         Rank5; // see if the next square is empty, check for double push
        pushes |= generatingPawn;
    }

    return pushes;
}

// special 'flood' attack generator for a set of pawns/knights
U64 genBulkKnightAttacks(U64 knight) {
    U64 moves = 0;

    // get one left moves
    moves |= ((knight >> 17) | (knight << 15)) & notHFile;

    // get two left moves
    moves |= ((knight >> 10) | (knight << 6)) & notGHFile;

    // get one right move
    moves |= ((knight >> 15) | (knight << 17)) & notAFile;

    // get two right moves
    moves |= ((knight >> 6) | (knight << 10)) & notABFile;

    return moves;
}
U64 genBulkPawnAttacks(U64 pawn, unsigned short side) {
    U64 move = 0;

    if (side == WHITE) {
        // generate the takes
        move |= ((pawn >> 9) & notHFile) | ((pawn >> 7) & notAFile);
    } else {
        // generate the takes
        move |= (((pawn << 9) & notAFile) | ((pawn << 7) & notHFile));
    }

    return move;
}

// move encoding
inline Move encodeMove(unsigned short startSquare, unsigned short endSquare, unsigned short promoCode, unsigned short moveFlag, unsigned short startType, unsigned short endType) {
    return (startSquare) | (endSquare << 6) | (promoCode << 12) | (moveFlag << 14) | (startType << 16) | (endType << 19);
}
void convertActiveBitboard(unsigned short &startSquare,unsigned short  startType, U64 moveBB, MoveList &activeMoveList, U64 *pieceBB) {
    if (moveBB == 0) return;

    // loop through all of the enemy piece types
    for (unsigned short endPiece = PAWN; endPiece <= KING; endPiece ++) {
        U64 subMoveBB = moveBB & pieceBB[endPiece];
        // take all the bits out of the move bitboard
        while (subMoveBB) {
            unsigned short endSquare = popIntLSB(subMoveBB); // get the end square

            // encode the move and add it to the chosen move list
            // a move flag is needed to encode en-passant moves
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, 0, 0, startType, endPiece));
        }
    }
}
void convertEnpassantBitboard(unsigned short &startSquare, U64 moveBB, MoveList &activeMoveList) {
    // take all the bits out of the move bitboard
    while (moveBB) {
        unsigned short endSquare = popIntLSB(moveBB); // get the end square
        // encode the move and add it to the chosen move list
        activeMoveList.emplace_back(encodeMove(startSquare, endSquare, 0, ENPASSANT, PAWN, PAWN));
    }
}
void convertQuietBitboard(unsigned short &startSquare,unsigned short  startType, U64 moveBB, MoveList &quietMoveList) {
    // take all the bits out of the move bitboard
    while (moveBB) {
        const unsigned short endSquare = popIntLSB(moveBB); // get the end square

        // encode the move and add it to the chosen move list
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, 0, 0, startType, EMPTY));
    }
}
void convertQuietPromoBitboard (unsigned short &startSquare, U64 moveBB, MoveList &quietMoveList) {
    // take all the bits out of the move bitboard
    while (moveBB) {
        unsigned short endSquare = popIntLSB(moveBB); // get the end square

        // encode the move and add it to the chosen move list
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, EMPTY));
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, EMPTY));
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, EMPTY));
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, EMPTY));
    }
}
void convertActivePromoBitboard(unsigned short &startSquare, U64 moveBB, MoveList &activeMoveList, U64 *pieceBB) {
    if (moveBB == 0) return;


    // loop through all of the enemy piece types
    for (unsigned short endPiece = PAWN; endPiece <= KING; endPiece ++) {
        U64 subMoveBB = moveBB & pieceBB[endPiece];
        // take all the bits out of the move bitboard
        while (subMoveBB) {
            unsigned short endSquare = popIntLSB(subMoveBB); // get the end square

            // encode the move and add it to the chosen move list
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, endPiece));
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, endPiece));
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, endPiece));
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, endPiece));
        }
    }
}

/* board status stuff */
U64 getSquareAttackers(U64 sq, unsigned short SIDE) {
    unsigned short mover, enemy;

    if (SIDE == WHITE) {
        mover = nWhite;
        enemy = nBlack;
    } else {
        mover = nBlack;
        enemy = nWhite;
    }

    sq = pieceBB[KING] & pieceBB[mover];

    // treat the king as a 'super piece' and get all the possible moves from it
    U64 attacks = 0;
    U64 knight, diag, horiz, king, pawn;
    knight = genAttack<KNIGHT>(sq, emptySquares) & pieceBB[KNIGHT];
    diag = genAttack<BISHOP>(sq, emptySquares) & (pieceBB[BISHOP] | pieceBB[QUEEN]);
    horiz = genAttack<ROOK>(sq, emptySquares) & (pieceBB[ROOK] | pieceBB[QUEEN]);
    king = genAttack<KING>(sq, emptySquares) & pieceBB[KING];
    pawn = genPawnAttacks(sq, SIDE) & pieceBB[PAWN];
    attacks = (knight | diag | horiz | king | pawn);
    attacks &= pieceBB[enemy];

    return attacks;
}
bool checkKingCheck(unsigned short SIDE) {
    U64 king = pieceBB[KING];
    if (SIDE == WHITE) {
        king &= pieceBB[nWhite];
    } else {
        king &= pieceBB[nBlack];
    }

    return getSquareAttackers(king, SIDE);
}
short getPieceAt(U64 &sq) {
    if (pieceBB[PAWN] & sq) return PAWN;
    if (pieceBB[KNIGHT] & sq) return KNIGHT;
    if (pieceBB[BISHOP] & sq) return BISHOP;
    if (pieceBB[ROOK] & sq) return ROOK;
    if (pieceBB[QUEEN] & sq) return QUEEN;
    if (pieceBB[KING] & sq) return KING;
}
U64 getRay(U64 &from, U64 &to) {
    U64 ray;
    ray = genNorthMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genSouthMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genEastMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genWestMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genNEMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genNWMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genSEMoves(from, emptySquares);
    if (ray & to) return ray | from;

    ray = genSWMoves(from, emptySquares);
    if (ray & to) return ray | from;
}

/* move generation stuff */
void genBlockers() {
    // used when generating legal moves
    // Returns the set of pieces which prevent a check. Includes both players' pieces for en-passant gen.
    U64 king = pieceBB[KING];

    king &= pieceBB[friendly];

    U64 rookNqueen = pieceBB[enemy] & (pieceBB[ROOK] | pieceBB[QUEEN]);
    U64 bishopNqueen = pieceBB[enemy] & (pieceBB[BISHOP] | pieceBB[QUEEN]);

    U64 kingMoves, enemyMoves;

    // first consider north and south moves
    kingMoves = genNorthMoves(king, emptySquares) | genSouthMoves(king, emptySquares);
    enemyMoves = genNorthMoves(rookNqueen, emptySquares) | genSouthMoves(rookNqueen, emptySquares);
    blockersNS = kingMoves & enemyMoves & occupiedSquares;

    // next consider east and west moves - note uses the same pieces as up
    kingMoves = genWestMoves(king, emptySquares) | genEastMoves(king, emptySquares);
    enemyMoves = genWestMoves(rookNqueen, emptySquares) | genEastMoves(rookNqueen, emptySquares);
    blockersEW = kingMoves & enemyMoves & occupiedSquares;

    // next consider NE and SW moves
    kingMoves = genNEMoves(king, emptySquares) | genSWMoves(king, emptySquares);
    enemyMoves = genNEMoves(bishopNqueen, emptySquares) | genSWMoves(bishopNqueen, emptySquares);
    blockersNE = kingMoves & enemyMoves & occupiedSquares;

    // next consider NW and SE moves
    kingMoves = genNWMoves(king, emptySquares) | genSEMoves(king, emptySquares);
    enemyMoves = genNWMoves(bishopNqueen, emptySquares) | genSEMoves(bishopNqueen, emptySquares);
    blockersNW = kingMoves & enemyMoves & occupiedSquares;
}
void genAttackMap() {
    // used to find safe squares for king to move to
    // exclude the king from empty squares to stop moves backwards along rays from being generated
    U64 enemyPieces;
    U64 friendlyKing = pieceBB[KING] & pieceBB[friendly];

    emptySquares |= friendlyKing; // set the king square to empty

    enemyPieces = pieceBB[enemy];
    attackMap = 0;

    // generate rook direction attacks
    U64 rookNqueen = enemyPieces & (pieceBB[ROOK] | pieceBB[QUEEN]);
    U64 horiAttacks = genAttack<ROOK>(rookNqueen, emptySquares);
    attackMap |= horiAttacks;

    // generate bishop direction attacks
    U64 bishopNqueen = enemyPieces & (pieceBB[BISHOP] | pieceBB[QUEEN]);
    U64 diagAttacks = genAttack<BISHOP>(bishopNqueen, emptySquares);
    attackMap |= diagAttacks;

    // generate pawn attacks
    U64 pawns = enemyPieces & pieceBB[PAWN];
    U64 pawnAttacks = genBulkPawnAttacks(pawns, otherSide);
    attackMap |= pawnAttacks;

    // generate knight attacks
    U64 knights = enemyPieces & pieceBB[KNIGHT];
    U64 knightAttacks = genBulkKnightAttacks(knights);
    attackMap |= knightAttacks;

    // generate king attacks
    U64 king = enemyPieces & pieceBB[KING];
    if (king) {
        U64 kingAttacks = genAttack<KING>(king, emptySquares);
        attackMap |= kingAttacks;
    }

    emptySquares ^= friendlyKing; // set the king square to occupied
}
void genKingMoves() {
    U64 moves = 0, actives = 0, quiets = 0;
    U64 generatingPiece = pieceBB[friendly] & pieceBB[KING]; // get the king
    if (generatingPiece) {
        unsigned short generatingPieceIndex = bitScanForward(generatingPiece);

        // combine the moves - note the for king moves you must only include safe squares
        moves = genAttack<KING>(generatingPiece, emptySquares) & (~attackMap);
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves

        convertQuietBitboard(generatingPieceIndex, KING, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, KING, actives, activeMoveList, pieceBB);
    }
}
void genRookMoves() {
    // you need to consider blockers when generating rook moves.
    U64 diagonalBlockers = blockersNE | blockersNW;
    U64 generatingPieces = pieceBB[friendly] & pieceBB[ROOK] & (~diagonalBlockers); // get the set of pieces

    while (generatingPieces) {
        U64 moves = 0, actives = 0, quiets = 0;
        U64 generatingPiece = popLSB(generatingPieces); // get the position of a piece
        unsigned short generatingPieceIndex = bitScanForward(generatingPiece);

        // blockers hold the set of squares that can only move along a certain direction
        // We AND the rook with the compliment of the blockers for each direction
        U64 rookNS = generatingPiece & (~blockersEW);
        U64 rookEW = generatingPiece & (~blockersNS);
        moves = genNorthMoves(rookNS, emptySquares) | genSouthMoves(rookNS, emptySquares)
                | genEastMoves(rookEW, emptySquares) | genWestMoves(rookEW, emptySquares);
        moves &= checkingRay;
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves

        convertQuietBitboard(generatingPieceIndex, ROOK, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, ROOK, actives, activeMoveList, pieceBB);
    }
}
void genBishopMoves() {
    // you need to consider blockers for bishop moves
    U64 horizontalBlockers = ~(blockersNS | blockersEW);
    U64 generatingPieces = pieceBB[friendly] & pieceBB[BISHOP] & horizontalBlockers; // get the set of pieces

    while (generatingPieces) {
        U64 moves = 0, actives = 0, quiets = 0;
        U64 generatingPiece = popLSB(generatingPieces); // get the position of a piece
        unsigned short generatingPieceIndex = bitScanForward(generatingPiece);

        U64 bishopNW = generatingPiece & (~blockersNE);
        U64 bishopNE = generatingPiece & (~blockersNW);

        // combine the moves
        moves = genNEMoves(bishopNE, emptySquares) | genSWMoves(bishopNE, emptySquares)
                | genNWMoves(bishopNW, emptySquares) | genSEMoves(bishopNW, emptySquares);
        moves &= checkingRay;
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves

        convertQuietBitboard(generatingPieceIndex, BISHOP, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, BISHOP, actives, activeMoveList, pieceBB);
    }
}
void genQueenMoves() {
    // you very much need to consider blockers for queen moves!

    U64 diagonalBlockers = ~(blockersNE | blockersNW);
    U64 horizontalBlockers = ~(blockersNS | blockersEW);

    U64 generatingPiece = pieceBB[friendly] & pieceBB[QUEEN]; // get the set of queens

    U64 moves = 0, actives = 0, quiets = 0;
    if (generatingPiece) {
        unsigned short generatingPieceIndex = bitScanForward(generatingPiece);

        U64 bishopNW = generatingPiece & (~blockersNE) & horizontalBlockers;
        U64 bishopNE = generatingPiece & (~blockersNW) & horizontalBlockers;
        U64 rookNS = generatingPiece & (~blockersEW) & diagonalBlockers;
        U64 rookEW = generatingPiece & (~blockersNS) & diagonalBlockers;

        moves = genNorthMoves(rookNS, emptySquares) | genSouthMoves(rookNS, emptySquares)
                | genEastMoves(rookEW, emptySquares) | genWestMoves(rookEW, emptySquares);
        moves |= genNEMoves(bishopNE, emptySquares) | genSWMoves(bishopNE, emptySquares)
                 | genNWMoves(bishopNW, emptySquares) | genSEMoves(bishopNW, emptySquares);
        moves &= checkingRay;
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves

        convertQuietBitboard(generatingPieceIndex, QUEEN, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, QUEEN, actives, activeMoveList, pieceBB);
    }
}
void genKnightMoves() {
    // knights can not move at all if they are pinned
    U64 generatingPieces = pieceBB[friendly] & pieceBB[KNIGHT]; // get the set of pieces
    generatingPieces &= ~(blockersNS | blockersEW | blockersNE | blockersNW); // consider blockers

    while (generatingPieces) {
        U64 moves = 0, actives = 0, quiets = 0;
        U64 generatingPiece = popLSB(generatingPieces); // get the position of a piece
        unsigned short generatingPieceIndex = bitScanForward(generatingPiece);

        // combine the moves
        moves = genAttack<KNIGHT>(generatingPiece, emptySquares);
        moves &= checkingRay;
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves


        convertQuietBitboard(generatingPieceIndex, KNIGHT, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, KNIGHT, actives, activeMoveList, pieceBB);
    }
}
void genPawnMoves() {
    U64 generatingPawns = pieceBB[PAWN] & pieceBB[friendly];

    // fills the en-passant rights to the two ranks which a take could be on
    U64 enPassantRank = enPassantRights;
    enPassantRank = (enPassantRank << (2 * 8)) | (enPassantRank << (5 * 8));

    while (generatingPawns) {
        U64 pushes = 0, captures = 0, enPassants = 0, pushPromo = 0, capturePromo = 0, promoRank = 0, excPromoRank = 0;
        U64 generatingPawn = popLSB(generatingPawns); // get the position of a rook
        unsigned short generatingPawnIndex = bitScanForward(generatingPawn);

        // pushes aren't allowed if you are horizontally or diagonally pinned
        U64 pushingPawn = generatingPawn & ~(blockersEW | blockersNE | blockersNW);
        pushes = genPawnPushes(pushingPawn, emptySquares, currentSide);

        // captures can't be done if you are horizontally or vertically pinned.
        // captures to the left can't be pinned to the right
        // captures to the right can't be pinned to the left
        U64 capturingPawn = generatingPawn & ~(blockersEW | blockersNS);
        U64 capturingPawnLeft, capturingPawnRight;
        if (currentSide == WHITE) {
            capturingPawnLeft = capturingPawn & ~(blockersNE);
            capturingPawnRight = capturingPawn & ~(blockersNW);
        } else {
            capturingPawnLeft = capturingPawn & ~(blockersNW);
            capturingPawnRight = capturingPawn & ~(blockersNE);
        }

        captures = genLeftPawnAttack(capturingPawnLeft, currentSide) |
                   genRightPawnAttack(capturingPawnRight, currentSide);
        captures &= pieceBB[enemy];

        // en-passants are more difficult as there could be a discovered check due to the pawn taken being a blocker
        // there is also the possibility for a rare double discovered check
        // before generating en-passants we will first check whether there are any en-passant rights
        if (enPassantRights) {
            U64 subCheckingRay = checkingRay; // used for check evasion
            U64 allowedRank, diagBlockers = ~(blockersNE | blockersNW);
            if (currentSide == WHITE) {
                subCheckingRay >>= 8;
                diagBlockers >>= 8;
                allowedRank = Rank6;
            } else {
                subCheckingRay <<= 8;
                diagBlockers <<= 8;
                allowedRank = Rank3;
            }

            U64 enPassLeft = 0, enPassRight = 0;

            // first get the left and right captures capture, and it with the allowed rank and enPassantRank
            // also and with the subCheckingRay for check evasions
            enPassLeft |=
                    genLeftPawnAttack(capturingPawnLeft, currentSide) & enPassantRank & allowedRank & diagBlockers &
                    subCheckingRay;
            enPassRight |= genRightPawnAttack(capturingPawnRight, currentSide) & enPassantRank & allowedRank &
                           diagBlockers & subCheckingRay;

            enPassants |= enPassLeft | enPassRight;

            // we now need to check for the rare double discovered horizontal check
            // we'll only do this if enPassants have been generated
            if (enPassants) {
                U64 king = pieceBB[KING] & pieceBB[friendly];

                // generate empty squares without the taken pawn and taking pawn
                U64 subEmptySquares = emptySquares;
                if (enPassLeft) {
                    subEmptySquares |= (generatingPawn >> 1) | generatingPawn;
                } else {
                    subEmptySquares |= (generatingPawn << 1) | generatingPawn;
                }
                U64 kingRank = genEastMoves(king, subEmptySquares) | genWestMoves(king, subEmptySquares);
                kingRank &= (pieceBB[ROOK] | pieceBB[QUEEN]) & pieceBB[enemy];

                // if there is no such attacked, generate the en-passant moves
                if (!kingRank) {
                    convertEnpassantBitboard(generatingPawnIndex, enPassants, activeMoveList);
                }
            }
        }

        if (currentSide == WHITE) {
            promoRank = Rank8;
            excPromoRank = notRank8;
        } else {
            promoRank = Rank1;
            excPromoRank = notRank1;
        }

        pushes &= checkingRay;
        captures &= checkingRay;

        // filter out the promotions
        pushPromo = pushes & promoRank;
        capturePromo = captures & promoRank;

        // remove the promotions from the move bitboards
        captures &= excPromoRank;
        pushes &= excPromoRank;

        convertActiveBitboard(generatingPawnIndex, PAWN, captures, activeMoveList, pieceBB);
        convertQuietBitboard(generatingPawnIndex, PAWN, pushes, quietMoveList);
        convertQuietPromoBitboard(generatingPawnIndex, pushPromo, activeMoveList);
        convertActivePromoBitboard(generatingPawnIndex, capturePromo, activeMoveList, pieceBB);
    }
}
void genCastling(CastleRights &rights) {
    if (rights.king) {
        // check left castle
        if (rights.left && (!(rights.leftMask & occupiedSquares)) && (!(rights.leftMask & attackMap))) {
            quietMoveList.emplace_back(
                    encodeMove(bitScanForward(rights.king), bitScanForward(rights.left), 0, 3,
                               KING, EMPTY));
        }

        // check right castle
        if (rights.right && (!(rights.rightMask & occupiedSquares)) && (!(rights.leftMask & attackMap))) {
            quietMoveList.emplace_back(
                    encodeMove(bitScanForward(rights.king), bitScanForward(rights.right), 0, 3,
                               KING, EMPTY));
        }
    }
}
void genAllMoves() {
    /*
     * When generating legal moves you need to consider pinned pieces.
     * For each direction d, take intersection of king moves in d', enemy moves along d, and friendly colours
     *
     * For king moves you can only move to safe squares so you must generate these
     * */
    genBlockers();
    genAttackMap();

    quietMoveList.clear();
    activeMoveList.clear();
    combinedMoveList.clear();

    checkingRay = ~(0); // all 1's

    U64 king = pieceBB[KING] & pieceBB[friendly];

    // We need to see if it is a single check or a double check
    // If it is a double check, only generate king moves
    // If it is a single check, generate moves which block or capture the checking piece.
    unsigned short numAttackers = 0;
    incheck = false;
    // if king is in check
    if (king & attackMap) {
        incheck = true;
        // first we generate attackers to the king square
        U64 attackers = getSquareAttackers(king, currentSide);
        numAttackers = count(attackers);
        // if there is a double check, only king moves are allowed
        if (numAttackers == 2) {
            genKingMoves();
        } else if (numAttackers == 1) {
            // find the piece that is attacking
            short attackingPieceKey = getPieceAt(attackers);

            // check if attacking piece is a slider
            if ((attackingPieceKey == ROOK) || (attackingPieceKey == BISHOP) || (attackingPieceKey == QUEEN)) {
                checkingRay = getRay(king, attackers);
            } else {
                // else the checking ray must only contain the attacker
                checkingRay = attackers;
            }

            genPawnMoves();
            genBishopMoves();
            genKnightMoves();
            genRookMoves();
            genQueenMoves();
            genKingMoves();
        } else {
            printBoardPrettily();
            printBitboard(king, nPiece);
            printBitboard(attackers, nPiece);
            printBitboard(attackMap, nPiece);
            assert(0);
        }
    } else {
        // if not in check generate moves normally
        genPawnMoves();
        genBishopMoves();
        genKnightMoves();
        genRookMoves();
        genQueenMoves();
        genKingMoves();

        if (currentSide == WHITE) {
            genCastling(wCastle);
        } else {
            genCastling(bCastle);
        }
    }

    combinedMoveList.insert(combinedMoveList.begin(), quietMoveList.begin(), quietMoveList.end());
    combinedMoveList.insert(combinedMoveList.begin(), activeMoveList.begin(), activeMoveList.end());
}
MoveList getMoveList() {
    genAllMoves();

    return combinedMoveList;
}