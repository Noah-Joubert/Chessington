//
// Created by Noah Joubert on 2021-04-22.
//

#include "../types.h"
#include "bitboards.cpp"
#include "board.h"
#include "../misc.h"

#ifndef SEARCH_MOVEGENCPP
#define SEARCH_MOVEGENCPP

/*
 * Generates all moves in a certain direction
 */
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

/*
 * This generates a bitboard of squares attacked by any piece (bar pawns) ~ this includes non-legal attacks however
 * The returned bitboard includes friendly/ enemy/ and empty squares
 * It can be used to generate moves for all pieces bar pawns.
 */
template<short T>
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
    if (generatingKnight == 0) return 0;

    return knightMasks[bitScanForward(generatingKnight)];
}
template<> inline U64 genAttack<KING>(U64 generatingKing, U64 &emptySquares) {
    return kingMasks[bitScanForward(generatingKing)];
}
U64 genAttackWrapper(U64 TEMPLATE, U64 generatingPiece, U64 &emptySquares) {
    // a function used to get past annoying template issues

    if (TEMPLATE == ROOK) {
        return genAttack<ROOK>(generatingPiece, emptySquares);

    } else if (TEMPLATE == BISHOP) {
        return genAttack<BISHOP>(generatingPiece, emptySquares);

    } else if (TEMPLATE == QUEEN) {
        return genAttack<QUEEN>(generatingPiece, emptySquares);

    } else if (TEMPLATE == KNIGHT) {
        return genAttack<KNIGHT>(generatingPiece, emptySquares);

    } else if (TEMPLATE == KING) {
        return genAttack<KING>(generatingPiece, emptySquares);

    }
}

/*
 * pawn move generators
 */
U64 genPawnAttacks(U64 generatingPawn, short SIDE) {
    return pawnCaptureMask[SIDE][bitScanForward(generatingPawn)];
}

/*
 * special 'flood' attack generator for a set of pawns/knights
 */
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
U64 genBulkPawnAttacks(U64 pawn, short side) {
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

/* move encoding */
void convertActiveBitboard(short &startSquare, short startType, U64 moveBB, MoveList &activeMoveList, U64 *pieceBB) {
    if (moveBB == 0) return;

    // loop through all of the enemy piece types
    for (short endPiece = PAWN; endPiece <= KING; endPiece ++) {
        U64 subMoveBB = moveBB & pieceBB[endPiece];
        // take all the bits out of the move bitboard
        while (subMoveBB) {
            short endSquare = popIntLSB(subMoveBB); // get the end square

            // encode the move and add it to the chosen move list
            // a move flag is needed to encode en-passant moves
            activeMoveList.emplace_back(encodeMove(startSquare, endSquare, 0, 0, startType, endPiece));
        }
    }
}
void convertQuietBitboard(short &startSquare,short  startType, U64 moveBB, MoveList &quietMoveList) {
    // take all the bits out of the move bitboard
    while (moveBB) {
        const short endSquare = popIntLSB(moveBB); // get the end square

        // encode the move and add it to the chosen move list
        quietMoveList.emplace_back(encodeMove(startSquare, endSquare, 0, 0, startType, EMPTY));
    }
}
void convertPromo(short startSquare, short endSquare, short toPiece, MoveList &moveList) {
    Move m1 = encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, toPiece);
    Move m2 = encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, toPiece);
    Move m3 = encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, toPiece);
    Move m4 = encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, toPiece);
    moveList.emplace_back(m1);
    moveList.emplace_back(m2);
    moveList.emplace_back(m3);
    moveList.emplace_back(m4);
}

/* board status stuff */
//bool Board::checkKingCheck(short SIDE) {
//    U64 king = pieceBB[KING];
//    if (SIDE == WHITE) {
//        king &= pieceBB[nWhite];
//    } else {
//        king &= pieceBB[nBlack];
//    }
//
//    return getSquareAttackers(king, SIDE);
//}
short Board::getPieceAt(U64 &sq) {
    if (pieceBB[PAWN] & sq) return PAWN;
    if (pieceBB[KNIGHT] & sq) return KNIGHT;
    if (pieceBB[BISHOP] & sq) return BISHOP;
    if (pieceBB[ROOK] & sq) return ROOK;
    if (pieceBB[QUEEN] & sq) return QUEEN;
    if (pieceBB[KING] & sq) return KING;
    return -1;
}
U64 Board::getRay (U64 &from, U64 &to){
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
void Board::genBlockers() {
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
void Board::genAttackMap() {
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
void Board::genKingMoves() {
    U64 moves = 0, actives = 0, quiets = 0;
    U64 generatingPiece = pieceBB[friendly] & pieceBB[KING]; // get the king
    if (generatingPiece) {
        short generatingPieceIndex = bitScanForward(generatingPiece);

        // combine the moves - note the for king moves you must only include safe squares
        moves = genAttack<KING>(generatingPiece, emptySquares) & (~attackMap);
        actives = moves & pieceBB[enemy]; // get the active moves
        quiets = moves & emptySquares; // get the passive moves

        convertQuietBitboard(generatingPieceIndex, KING, quiets, quietMoveList);
        convertActiveBitboard(generatingPieceIndex, KING, actives, activeMoveList, pieceBB);
    }
}
void Board::genPawnMoves(short TYPE) {
    //TODO Try and digest the genius that I displayed in producing this code

    U64 generatingPawns = pieceBB[PAWN] & pieceBB[friendly];

    // get the en-passant and the promotion rank
    U64 enPassantRank = enPassantRights, promotionRank, doublePushRank, upLeft, upRight;
    short up;
    if (currentSide == WHITE) {
        enPassantRank <<= (2 * 8);
        promotionRank = Rank7;
        doublePushRank = Rank3;
        up = -8;
        upLeft = -9;
        upRight = -7;
    } else {
        enPassantRank <<= (5 * 8);
        promotionRank = Rank2;
        doublePushRank = Rank6;
        up = 8;
        upLeft = 9;
        upRight = 7;
    }

    // if we are generating quiescence moves, AND the validSquares with squares which put the enemy in check
    // and the enemy pieces, and the promotion rank (rank 1 or rank 8)
    // or promotions
    U64 validSquares = checkingRay;
    if (TYPE == QUIESENCE_MOVES) {
        U64 enemyKing = getPieces(KING, otherSide);
        U64 checkingSquares = shift(enemyKing, - upRight) | shift(enemyKing, - upLeft);
        
        validSquares &= (push(promotionRank, currentSide) | pieceBB[enemy]);
    }

    // split up the promotion and regular pawns
    U64 regPawns = generatingPawns & (~promotionRank);
    U64 promoPawns = generatingPawns & (promotionRank);

    /****** first deal with regular pawns ******/
    /* pushes first */
    U64 pushingPawns = regPawns & ~(blockersNE | blockersNW | blockersEW);
    U64 firstPush = push(pushingPawns, currentSide) & emptySquares;
    U64 secondPush = push(firstPush & doublePushRank, currentSide)  & emptySquares;
    firstPush &= validSquares;
    secondPush &= validSquares;
    while (firstPush) {
        short to = popIntLSB(firstPush);
        Move move = encodeMove(to - up, to, 0, 0, PAWN, EMPTY);
        quietMoveList.emplace_back(move);
    }
    while (secondPush) {
        short to = popIntLSB(secondPush);
        Move move = encodeMove(to - up - up, to, 0, 0, PAWN, EMPTY);
        quietMoveList.emplace_back(move);
    }

    /* now captures */
    /* note that up-left and upright is relative to the side moving */
    U64 capturingPawns = regPawns & ~(blockersNS | blockersEW);
    U64 leftCaptures = shift(capturingPawns & ~(blockersNE), upLeft) & pieceBB[enemy] & validSquares;
    U64 rightCaptures = shift(capturingPawns & ~(blockersNW), upRight) & pieceBB[enemy] & validSquares;
    for (short piece = PAWN; piece <= KING; piece++) {
        U64 left = leftCaptures & pieceBB[piece];
        U64 right = rightCaptures & pieceBB[piece];
        while (left) {
            short to = popIntLSB(left);
            Move move = encodeMove(to - upLeft, to, 0, 0, PAWN, piece);
            activeMoveList.emplace_back(move);
        }
        while (right) {
            short to = popIntLSB(right);
            Move move = encodeMove(to - upRight, to, 0, 0, PAWN, piece);
            activeMoveList.emplace_back(move);
        }
    }

    /* now take a deep breath and check for en-passants */
    if (enPassantRights) {
        U64 subCheckingRay = validSquares; // used for check evasion
        U64 allowedRank, diagBlockers = ~(blockersNE | blockersNW);
        if (currentSide == WHITE) {
            subCheckingRay = subCheckingRay | (subCheckingRay >> 8);
            diagBlockers = shift(diagBlockers, north); // shift diag blockers
            allowedRank = Rank6;
        } else {
            subCheckingRay = subCheckingRay | (subCheckingRay << 8);
            diagBlockers = shift(diagBlockers, south); // shift diag blockers
            allowedRank = Rank3;
        }
        U64 enPassLeft = 0, enPassRight = 0;

        // first get the left and right captures capture, and it with the allowed rank and enPassantRank
        enPassLeft |= shift(capturingPawns & ~(blockersNE), upLeft) & enPassantRank & allowedRank & subCheckingRay;
        enPassRight |= shift(capturingPawns & ~(blockersNW), upRight) & enPassantRank & allowedRank & subCheckingRay;

        // you also need to check the captured pawn isn't a diagonal blocker, we check if it's a horizontal blocker later
        // note that diagBlockers are shifted earlier
        enPassLeft &= diagBlockers;
        enPassRight &= diagBlockers;

        // we now need to check for the rare double discovered horizontal check
        // we'll only do this if enPassants have been generated
        U64 king = pieceBB[KING] & pieceBB[friendly];
        if (enPassLeft) {
            // generate empty squares without the taken pawn and taking pawn
            U64 subEmptySquares = emptySquares;
            subEmptySquares ^= shift(enPassLeft, -up) | shift(enPassLeft, -upLeft);

            U64 kingAttackers = genEastMoves(king, subEmptySquares) | genWestMoves(king, subEmptySquares);
            kingAttackers &= (pieceBB[ROOK] | pieceBB[QUEEN]) & pieceBB[enemy];

            if (!kingAttackers) {
                short square = popIntLSB(enPassLeft);
                Move move = encodeMove(square - upLeft, square, 0, ENPASSANT, PAWN, PAWN);
                activeMoveList.emplace_back(move);
            }
        }
        if (enPassRight) {
            // generate empty squares without the taken pawn and taking pawn
            U64 subEmptySquares = emptySquares;
            subEmptySquares ^= (shift(enPassRight, -up) | shift(enPassRight, -upRight));

            U64 kingAttackers = genEastMoves(king, subEmptySquares) | genWestMoves(king, subEmptySquares);
            kingAttackers &= (pieceBB[ROOK] | pieceBB[QUEEN]) & pieceBB[enemy];

            if (!kingAttackers) {
                short square = popIntLSB(enPassRight);
                Move move = encodeMove(square - upRight, square, 0, ENPASSANT, PAWN, PAWN);
                activeMoveList.emplace_back(move);
            }
        }
    }

    if (!promoPawns) return;
    /* now deal with the promoting pawns */
    /* pushes first */
    pushingPawns = promoPawns & ~(blockersNE | blockersNW | blockersEW);
    firstPush = push(pushingPawns, currentSide) & validSquares & emptySquares;
    while (firstPush) {
        short to = popIntLSB(firstPush);
        convertPromo(to - up, to, EMPTY, activeMoveList); // we add promos to the activeMoveList
    }

    /* now captures */
    /* note that up-left and upright is relative to the side moving */
    capturingPawns = promoPawns & ~(blockersNS | blockersEW);
    leftCaptures = shift(capturingPawns & ~(blockersNE), upLeft) & pieceBB[enemy] & validSquares;
    rightCaptures = shift(capturingPawns & ~(blockersNW), upRight) & pieceBB[enemy] & validSquares;
    for (short piece = PAWN; piece <= KING; piece++) {
        U64 left = leftCaptures & pieceBB[piece];
        U64 right = rightCaptures & pieceBB[piece];
        while (left) {
            short to = popIntLSB(left);
            convertPromo(to - upLeft, to, piece, activeMoveList);
        }
        while (right) {
            short to = popIntLSB(right);
            convertPromo(to - upRight, to, piece, activeMoveList);
        }
    }
}

U64 Board::genBishopLegal(U64 piece) {
    U64 horizontalBlockers = ~(blockersNS | blockersEW);
    U64 generatingPiece = piece & horizontalBlockers;

    // consider blockers
    U64 bishopNW = generatingPiece & (~blockersNE);
    U64 bishopNE = generatingPiece & (~blockersNW);

    // combine the moves
    U64 moves = genNEMoves(bishopNE, emptySquares) | genSWMoves(bishopNE, emptySquares)
            | genNWMoves(bishopNW, emptySquares) | genSEMoves(bishopNW, emptySquares);

    return moves;
}
U64 Board::genRookLegal(U64 piece) {
    U64 diagonalBlockers = blockersNE | blockersNW;
    U64 generatingPiece = piece & (~diagonalBlockers); // get the set of pieces

    U64 rookNS = generatingPiece & (~blockersEW);
    U64 rookEW = generatingPiece & (~blockersNS);
    U64 moves = genNorthMoves(rookNS, emptySquares) | genSouthMoves(rookNS, emptySquares)
            | genEastMoves(rookEW, emptySquares) | genWestMoves(rookEW, emptySquares);

    return moves;
}
U64 Board::genKnightLegal(U64 piece) {
    U64 generatingPiece = piece & ~(blockersNS | blockersEW | blockersNE | blockersNW); // consider blockers
    U64 moves = genAttack<KNIGHT>(generatingPiece, emptySquares);

    return moves;
}

U64 Board::genPieceLegal(U64 piece, short pieceType) {
    if (pieceType == BISHOP) {
        return genBishopLegal(piece);

    } else if (pieceType == KNIGHT) {
        return genKnightLegal(piece);

    } else if (pieceType == ROOK) {
        return genRookLegal(piece);

    } else if (pieceType == QUEEN) {
        return (genRookLegal(piece) | genBishopLegal(piece));

    }
}
void Board::genLegal(short pieceType, short TYPE) {
    // used to generate moves for all pieces except pawns/king
    assert((pieceType != PAWN) && (pieceType != KING));

    // get the set of all the pieces
    U64 generatingPieces = pieceBB[friendly] & pieceBB[pieceType]; // get the set of pieces

    // get the 'validSquares' bitboard
    // this is the bitboard which holds all the valid squares for a move to land on
    // it's only used for generating check evasions
    U64 validSquares = checkingRay;
    U64 checkingSquares = genAttackWrapper(pieceType, getPieces(KING, otherSide), emptySquares);

    // loop through all of the pieces
    while (generatingPieces) {
        U64 moves = 0, captures = 0, quiets = 0, quietChecks = 0;
        U64 generatingPiece = popLSB(generatingPieces); // get the position of a piece
        short generatingPieceIndex = bitScanForward(generatingPiece); // get the index of the generating piece

        // combine the moves
        moves = genPieceLegal(generatingPiece, pieceType);

        // AND with the valid destination squares
        moves &= validSquares;

        // get the captures
        captures = moves & pieceBB[enemy]; // get the active moves
        moves &= ~(pieceBB[enemy]); // remove the active moves from the moves bitboard

        // get the quiet checks
        quietChecks = checkingSquares & moves & emptySquares; // get the quiet checks
        moves &= ~(checkingSquares); // remove the checking moves from the moves bitboard

        // get the rest of the quiet moves
        quiets = moves & emptySquares; // get the passive moves

        // convert the move bitboards into arrays of moves
        if (TYPE == ALL_MOVES) {
            convertQuietBitboard(generatingPieceIndex, pieceType, quiets, quietMoveList);
            convertQuietBitboard(generatingPieceIndex, pieceType, quietChecks, activeMoveList);
            convertActiveBitboard(generatingPieceIndex, pieceType, captures, activeMoveList, pieceBB);
        } else if (TYPE == QUIESENCE_MOVES) {
//            convertQuietBitboard(generatingPieceIndex, pieceType, quietChecks, activeMoveList);
            convertActiveBitboard(generatingPieceIndex, pieceType, captures, activeMoveList, pieceBB);
        }
    }
}
void Board::genCastlingNew() {
    CRights subRights;
    short king, left, right;
    if (currentSide == WHITE) {
        subRights = CastleRights;
        king = E1;
        left = A1;
        right = H1;
    } else {
        subRights = CastleRights >> 2;
        king = E8;
        left = A8;
        right = H8;
    }

    if ((subRights & 1) &&
        !(ClearCastleLaneMasks[currentSide][0] & occupiedSquares) &&
        !((ClearCastleLaneMasks[currentSide][0] << 1) & attackMap))
    {
        quietMoveList.emplace_back(encodeMove(king, left, 0, 3, KING, ROOK));
    }

    if ((subRights & 2) &&
        !(ClearCastleLaneMasks[currentSide][1] & occupiedSquares) &&
        !(ClearCastleLaneMasks[currentSide][1] & attackMap))
    {
        quietMoveList.emplace_back(encodeMove(king, right, 0, 3, KING, ROOK));
    }
}
void Board::genAllMoves(short TYPE) {
    // this function generates all the moves for the current position
    // the type can be all moves, or quiesence moves for the quiesence search ie. checks and captures, or check evasions

    genBlockers();
    genAttackMap();

    quietMoveList.clear();
    activeMoveList.clear();
    combinedMoveList.clear();

    // this is the valid destination squares of a move
    checkingRay = ~(0); // all 1's

    U64 king = pieceBB[KING] & pieceBB[friendly];

    // We need to see if it is a single check or a double check
    // If it is a double check, only generate king moves
    // If it is a single check, generate moves which block or capture the checking piece.
    short numAttackers = 0;
    inCheck = false;
    // if king is in check
    if (king & attackMap) {
        inCheck = true;

        // first we generate attackers to the king square
        U64 attackers = getSquareAttackers(king, otherSide);
        numAttackers = count(attackers);

        // if there is a double check, only king moves are allowed
        if (numAttackers >= 2) {
            genKingMoves();
        } else if (numAttackers == 1) {
            // find the piece that is attacking
            short attackingPieceKey = getPieceAt(attackers);

            // check if attacking piece is a slider
            if ((attackingPieceKey == ROOK) || (attackingPieceKey == BISHOP) || (attackingPieceKey == QUEEN)) {
                checkingRay &= getRay(king, attackers);
            } else {
                // else the checking ray must only contain the attacker
                checkingRay &= attackers;
            }

            // now generate the moves
            for (short pieceType: {BISHOP, KNIGHT, ROOK, QUEEN}) {
                genLegal(pieceType, ALL_MOVES);
            }
            genPawnMoves(ALL_MOVES);
            genKingMoves();
        }
    } else {
        // if not in check, and only generating active moves, AND checkingRay with enemy squares

        if (TYPE == ALL_MOVES) {
            // generate all moves normally
            for (short pieceType: {BISHOP, KNIGHT, ROOK, QUEEN}) {
                genLegal(pieceType, ALL_MOVES);
            }

            genPawnMoves(ALL_MOVES);
            genKingMoves();
            genCastlingNew();

        } else if (TYPE == QUIESENCE_MOVES) {
            // only generate captures/ checks
            for (short pieceType: {BISHOP, KNIGHT, ROOK, QUEEN}) {
                genLegal(pieceType, QUIESENCE_MOVES);
            }

            genPawnMoves(QUIESENCE_MOVES);
            genCastlingNew();
        }

    }

    combinedMoveList.insert(combinedMoveList.begin(), quietMoveList.begin(), quietMoveList.end());
    combinedMoveList.insert(combinedMoveList.begin(), activeMoveList.begin(), activeMoveList.end());
}

bool Board::innerGivesCheck(Move &move) {
    // see if the move is a check
    short piece = (move & fromTypeMask) >> 16;

    if (piece != KING) {
        U64 destinationSquare = toBB((move & toMask) >> 6);
        U64 enemyKing = getPieces(KING, innerGetOtherSide());
        U64 kingAttacks;
        if (piece != PAWN) {
            U64 emptySquares = getEmptySquares();
            kingAttacks = genAttackWrapper(piece, enemyKing, emptySquares);
        } else {
            kingAttacks = genPawnAttacks(enemyKing, innerGetOtherSide());
        }
        if (destinationSquare & kingAttacks) return true;
    }

    return false;
}

#endif SEARCH_MOVEGENCPP