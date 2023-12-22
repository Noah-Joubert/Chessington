#ifndef MOVEGEN_CPP
#define MOVEGEN_CPP

#include "board.h"
#include "bitboards.cpp"

namespace Masks {
    const U64 AFile = 72340172838076673, BFile = AFile << 1, CFile = AFile << 2, DFile = AFile << 3,
            EFile = AFile << 4, FFile = AFile << 5, GFile = AFile << 6, HFile = AFile << 7;
    const U64 notAFile = ~AFile, notBFile = ~BFile, notCFile = ~CFile, notDFile = ~DFile, noEFile = ~EFile,
            notFFile = ~FFile, notGFile = ~GFile, notHFile = ~HFile;
    const U64 ABFile = AFile | BFile, notABFile = ~ABFile, GHFile = GFile | HFile, notGHFile = ~GHFile;

    constexpr U64 Rank8 = 255, Rank7 = Rank8 << 8, Rank6 = Rank7 << 8, Rank5 = Rank6 << 8, Rank4 = Rank5 << 8,
            Rank3 = Rank4 << 8, Rank2 = Rank3 << 8, Rank1 = Rank2 << 8;
    constexpr U64 notRank8 = ~Rank8, notRank7 = ~Rank7, notRank6 = ~Rank6, notRank5 = ~Rank5, notRank4 = ~Rank4,
            notRank3 = ~Rank3, notRank2 = ~Rank2, notRank1 = ~Rank1;

    constexpr short west = -1, east = -1, south = 8, north = -8;
    constexpr short noWe = -9, noEa = -7, soWe = 7, soEa = 9;

    /* The mask is to check that there aren't any pieces between the rook and king, so we can legally castle */
    constexpr U64 ClearCastleLaneMasks[2][2] = {{1008806316530991104, 6917529027641081856}, {14, 96}};

    U64 knightMasks[64];
    U64 kingMasks[64];
    U64 pawnCaptureMask[2][64]; // for white attacks, and for black attacks

    U64 genKnightPatten(U64 knight) {
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
    U64 genKingPatten(U64 king) {
        U64 moves = C64(0);

        moves |= ((king >> 1) | (king >> 9) | (king << 7)) & notHFile;
        moves |= ((king << 1) | (king << 9) | (king >> 7)) & notAFile;
        moves |= (king >> 8) | (king << 8);

        return moves;
    }
    U64 genPawnPatten(U64 pawn, Side side) {
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
    inline U64 pawnPush(U64 BB, short side) {
        return side == WHITE ? (BB >> 8) : (BB << 8);
    }
    inline U64 shiftBitboard(U64 BB, short D) {
        return D == noEa ? (BB >> 7) & notAFile:
               D == noWe ? (BB >> 9) & notHFile:
               D == soEa ? (BB << 9) & notAFile:
               D == soWe ? (BB << 7) & notHFile:
               D == north ? (BB >> 8):
               D == south ? (BB << 8):
               0;
    }

    // This function must be called upon startup!
    void genMasks() {
        for (int sq = A8; sq <= H1; sq++) {
            knightMasks[sq] = genKnightPatten(C64(1) << sq);
            kingMasks[sq] = genKingPatten(C64(1) << sq);
            pawnCaptureMask[WHITE][sq] = genPawnPatten(C64(1) << sq, WHITE);
            pawnCaptureMask[BLACK][sq] = genPawnPatten(C64(1) << sq, BLACK);
        }
    }
}

namespace MoveGeneration {
    struct MoveGenBitboards {
        /* Set of pieces which are preventing a check on our king from in a sliding piece direction. north-east, east-west etc. */
        U64 NS, EW, NE, NW;
        U64 attackMap;
        U64 validDestinationSquares;
        Side friendly, enemy;
    };
    struct MoveListsContainer {
        MoveList *quietMoveList, *activeMoveList, *combinedMoveList;
        MoveListsContainer(MoveList* quiet, MoveList* active, MoveList* combined) {
            this->quietMoveList = quiet;
            this->activeMoveList = active;
            this->combinedMoveList = combined;
        }
        void combineLists() {
            combinedMoveList->insert(combinedMoveList->begin(), quietMoveList->begin(), quietMoveList->end());
            combinedMoveList->insert(combinedMoveList->begin(), activeMoveList->begin(), activeMoveList->end());
        }
    };

    /* Generates sliding moves in a certain direction */
    inline U64 genNorthBB(U64 generatingPieces, U64 &emptySquares) {
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
    inline U64 genSouthBB(U64 generatingPieces, U64 &emptySquares) {
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
    inline U64 genWestBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notHFile;

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
        flood &= Masks::notHFile;

        return flood;
    }
    inline U64 genEastBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notAFile;

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
        flood &= Masks::notAFile;

        // shift once more to include the blocker, and exclude the start square
        return flood;
    }
    inline U64 genNwBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notHFile;

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
        flood &= Masks::notHFile;

        return flood;
    }
    inline U64 genSwBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notHFile;

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
        flood &= Masks::notHFile;

        return flood;
    }
    inline U64 genNeBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notAFile;

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
        flood &= Masks::notAFile;

        return flood;
    }
    inline U64 genSeBB(U64 generatingPieces, U64 emptySquares) {
        // get the flooded bit board of north moves
        U64 flood = 0; // set of possible locations

        // apply the blocker to the H file to stop wrap around's
        emptySquares &= Masks::notAFile;

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
        flood &= Masks::notAFile;

        return flood;
    }

    /* This generates a bitboard of squares attacked by a single piece */
    template<Pieces T>
    inline U64 genAttackBB(U64 generatingPiece, U64 &emptySquares);
    template<> inline U64 genAttackBB<ROOK>(U64 generatingPiece, U64 &emptySquares) {
        return (genNorthBB(generatingPiece, emptySquares) | genSouthBB(generatingPiece, emptySquares) |
                genWestBB(generatingPiece, emptySquares) | genEastBB(generatingPiece, emptySquares));
    };
    template<> inline U64 genAttackBB<BISHOP>(U64 generatingPiece, U64 &emptySquares) {
        return (genNwBB(generatingPiece, emptySquares) | genNeBB(generatingPiece, emptySquares) |
                genSeBB(generatingPiece, emptySquares) | genSwBB(generatingPiece, emptySquares));
    }
    template<> inline U64 genAttackBB<QUEEN>(U64 generatingPiece, U64 &emptySquares) {
        return genAttackBB<ROOK>(generatingPiece, emptySquares) | genAttackBB<BISHOP>(generatingPiece, emptySquares);
    }
    template<> inline U64 genAttackBB<KNIGHT>(U64 generatingPiece, U64 &emptySquares) {
        if (generatingPiece == 0) return 0;

        return Masks::knightMasks[bitScanForward(generatingPiece)];
    }
    template<> inline U64 genAttackBB<KING>(U64 generatingPiece, U64 &emptySquares) {
        return Masks::kingMasks[bitScanForward(generatingPiece)];
    }

    /* This generates the bitboard of squares attacked by a set of pieces */
    template <Pieces T>
    inline U64 genBulkAttackBB(U64 generatingPieces, Side side);
    template<> inline U64 genBulkAttackBB<PAWN>(U64 generatingPieces, Side side) {
        U64 attacks = 0;

        if (side == WHITE) {
            // generate the takes
            attacks |= ((generatingPieces >> 9) & Masks::notHFile) | ((generatingPieces >> 7) & Masks::notAFile);
        } else {
            // generate the takes
            attacks |= (((generatingPieces << 9) & Masks::notAFile) | ((generatingPieces << 7) & Masks::notHFile));
        }

        return attacks;
    }
    template<> inline U64 genBulkAttackBB<KNIGHT>(U64 generatingPieces, Side side) {
        U64 attacks = 0;

        // get one left moves
        attacks |= ((generatingPieces >> 17) | (generatingPieces << 15)) & Masks::notHFile;

        // get two left moves
        attacks |= ((generatingPieces >> 10) | (generatingPieces << 6)) & Masks::notGHFile;

        // get one right move
        attacks |= ((generatingPieces >> 15) | (generatingPieces << 17)) & Masks::notAFile;

        // get two right moves
        attacks |= ((generatingPieces >> 6) | (generatingPieces << 10)) & Masks::notABFile;

        return attacks;
    }

    /* Create a Move object from bitboards of moves */
    enum MoveTypes {
        Quiet = 0,
        Active = 1,
        Promo = 2,
        EnPassant
    };
    inline Move encodeMove(short startSquare, short endSquare, short promoCode, short moveFlag, short startType, short endType) {
        return (startSquare) | (endSquare << 6) | (promoCode << 12) | (moveFlag << 14) | (startType << 16) | (endType << 19);
    }
    template <MoveTypes T>
    inline void fillMoveList(MoveList *moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB);
    template<> inline void fillMoveList<Quiet>(MoveList *moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        while (moveBB) {
            const short endSquare = popIntLSB(moveBB);
            Move move = encodeMove(startSquare, endSquare, 0, 0, startPiece, EMPTY);
            moveList->emplace_back(move);
        }
    }
    template<> inline void fillMoveList<Active>(MoveList *moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        if (moveBB == 0) return;

        for (Pieces endPiece: {PAWN, KNIGHT, KING, QUEEN, BISHOP, ROOK}) {
            U64 attackedPieces = moveBB & bitboards.getPieceBB(endPiece);

            while (attackedPieces) {
                short endSquare = popIntLSB(attackedPieces);
                Move move = encodeMove(startSquare, endSquare, 0, 0, startPiece, endPiece);

                moveList->emplace_back(move);
            }

            moveBB &= (~attackedPieces);
            if (!moveBB) break;
        }
    }
    template<> inline void fillMoveList<Promo>(MoveList *moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        U64 quietPromos = moveBB & bitboards.EmptySquares;
        while (quietPromos) {
            const short endSquare = popIntLSB(quietPromos);

            Move m1 = encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, EMPTY);
            Move m2 = encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, EMPTY);
            Move m3 = encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, EMPTY);
            Move m4 = encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, EMPTY);
            moveList->emplace_back(m1);
            moveList->emplace_back(m2);
            moveList->emplace_back(m3);
            moveList->emplace_back(m4);
        }

        U64 activePromos = moveBB & (~bitboards.EmptySquares);
        if (activePromos == 0) { return; }
        for (Pieces endPiece: {PAWN, BISHOP, KNIGHT, QUEEN, KING, ROOK}) {
            U64 attackedPieces = activePromos & bitboards.getPieceBB(endPiece);

            while (attackedPieces) {
                short endSquare = popIntLSB(attackedPieces);

                Move m1 = encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, endPiece);
                Move m2 = encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, endPiece);
                Move m3 = encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, endPiece);
                Move m4 = encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, endPiece);
                moveList->emplace_back(m1);
                moveList->emplace_back(m2);
                moveList->emplace_back(m3);
                moveList->emplace_back(m4);
            }

            activePromos &= (~attackedPieces);
            if (!activePromos) break;
        }
    }
    template<> inline void fillMoveList<EnPassant>(MoveList *moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        while (moveBB) {
            const short endSquare = popIntLSB(moveBB);
            Move move = encodeMove(startSquare, endSquare, 0, ENPASSANT, PAWN, PAWN);

            moveList->emplace_back(move);
        }
    }

    /* Returns the set of semi-legal moves for a set of pieces - meaning there are no discovered checks */
    template <Pieces T>
    inline U64 genSemiLegalBB(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards);
    template<> inline U64 genSemiLegalBB<BISHOP>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 horizontalBlockers = ~(blockers.NS | blockers.EW);
        piece &= horizontalBlockers;

        // consider blockers
        U64 bishopNW = piece & (~blockers.NE);
        U64 bishopNE = piece & (~blockers.NW);

        // combine the moves
        U64 moves = genNeBB(bishopNE, bitboards.EmptySquares) | genSwBB(bishopNE, bitboards.EmptySquares)
                    | genNwBB(bishopNW, bitboards.EmptySquares) | genSeBB(bishopNW, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegalBB<KNIGHT>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 generatingPiece = piece & ~(blockers.NS | blockers.EW | blockers.NE | blockers.NW); // consider blockers
        U64 moves = genAttackBB<KNIGHT>(generatingPiece, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegalBB<ROOK>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 diagonalBlockers = blockers.NE | blockers.NW;
        U64 generatingPiece = piece & (~diagonalBlockers); // get the set of pieces

        U64 rookNS = generatingPiece & (~blockers.EW);
        U64 rookEW = generatingPiece & (~blockers.NS);
        U64 moves = genNorthBB(rookNS, bitboards.EmptySquares) | genSouthBB(rookNS, bitboards.EmptySquares)
                    | genEastBB(rookEW, bitboards.EmptySquares) | genWestBB(rookEW, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegalBB<KING>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 moves = genAttackBB<KING>(piece, bitboards.EmptySquares);

        return moves & ~blockers.attackMap;
    }
    template<> inline U64 genSemiLegalBB<QUEEN>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        return genSemiLegalBB<ROOK>(piece, blockers, bitboards) |
               genSemiLegalBB<BISHOP>(piece, blockers, bitboards);
    }
    template<> inline U64 genSemiLegalBB<PAWN>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        Side friendly = blockers.friendly, enemy = blockers.enemy;
        short upLeft, upRight;
        U64 doublePushRank;
        if (friendly == WHITE) {
            doublePushRank = Masks::Rank3;
            upLeft = Masks::noWe;
            upRight = Masks::noEa;
        } else {
            doublePushRank = Masks::Rank6;
            upLeft = Masks::soEa;
            upRight = Masks::soWe;
        }

        U64 moves = 0;

        // do pushes and captures
        U64 pushingPawn = piece & ~(blockers.NE | blockers.NW | blockers.EW);
        if (pushingPawn) {
            U64 firstPush = Masks::pawnPush(pushingPawn, friendly) & bitboards.EmptySquares;
            U64 secondPush = Masks::pawnPush(firstPush & doublePushRank, friendly) & bitboards.EmptySquares;
            moves |= (firstPush | secondPush);
        }

        U64 capturingPawn = piece & ~(blockers.NS | blockers.EW);
        U64 leftCaptures = Masks::shiftBitboard(capturingPawn & ~(blockers.NE), upLeft);
        U64 rightCaptures = Masks::shiftBitboard(capturingPawn & ~(blockers.NW), upRight);
        moves |= bitboards.getSideBB(enemy) & (leftCaptures | rightCaptures);

        return moves & blockers.validDestinationSquares;
    }
    template<> inline U64 genSemiLegalBB<ENPASSANTPAWNS>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        if (!bitboards.enPassantRights) return 0;

        // TODO
        U64 enPassantRank; // rank on which pawns can en-passant
        U64 enPassantRightsMask = bitboards.enPassantRights;
        U64 subValidationSquares;
        short upLeft, upRight, up; // directions relative to the current player
        if (blockers.friendly == WHITE) {
            up = Masks::north;
            upLeft = Masks::noWe;
            upRight = Masks::noEa;
            enPassantRank = Masks::Rank5;
            enPassantRightsMask <<= (2 * 8);
            subValidationSquares = blockers.validDestinationSquares | (blockers.validDestinationSquares >> 8);
        } else {
            up = Masks::south;
            upLeft = Masks::soEa;
            upRight = Masks::soWe;
            enPassantRank = Masks::Rank4;
            enPassantRightsMask <<= (5 * 8);
            subValidationSquares = blockers.validDestinationSquares | (blockers.validDestinationSquares << 8);
        }

        // check the capturing-pawn is on the correct rank, and not a blocker
        U64 pawn = piece & enPassantRank & ~(blockers.NS | blockers.EW);
        if (!pawn) return 0;

        // generate the left and right en-passant capture. make sure we have the rights for the capture
        U64 enPassantLeftBB = Masks::shiftBitboard(pawn & ~blockers.NE, upLeft) & enPassantRightsMask;
        U64 enPassantRightBB = Masks::shiftBitboard(pawn & ~blockers.NW, upRight) & enPassantRightsMask;

        // now check the taken piece wasn't a diagonal blocker, and that the destination square is valid
        U64 diagBlockers = ~(blockers.NE | blockers.NW);

        enPassantLeftBB &= subValidationSquares & diagBlockers;
        enPassantRightBB &= subValidationSquares & diagBlockers;

        // check for the rare double horizontal discovered check TODO: optimise this
        U64 king = bitboards.getPieceBB(KING) & bitboards.getSideBB(blockers.friendly);
        U64 postLeftEmptySquares = bitboards.EmptySquares ^ (Masks::shiftBitboard(enPassantLeftBB, -up) | Masks::shiftBitboard(enPassantLeftBB, -upLeft));
        U64 postRightEmptySquares = bitboards.EmptySquares ^ (Masks::shiftBitboard(enPassantRightBB, -up) | Masks::shiftBitboard(enPassantRightBB, -upRight));

        U64 postLeftKingAttackers = genEastBB(king, postLeftEmptySquares) | genWestBB(king, postLeftEmptySquares);
        U64 postRightKingAttackers = genEastBB(king, postRightEmptySquares) | genWestBB(king, postRightEmptySquares);
        postLeftKingAttackers &= (bitboards.getPieceBB(QUEEN) | bitboards.getPieceBB(ROOK)) & bitboards.getSideBB(blockers.enemy);
        postRightKingAttackers &= (bitboards.getPieceBB(QUEEN) | bitboards.getPieceBB(ROOK)) & bitboards.getSideBB(blockers.enemy);

        if ((!postLeftKingAttackers) && (enPassantLeftBB)) return enPassantLeftBB;
        if ((!postRightKingAttackers) && (enPassantRightBB)) return enPassantRightBB;

        return 0;
    }
    U64 genSemiLegalBBWrapper(U64 pieceBB, Pieces piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 moves;

        switch (piece) {
            case BISHOP:
                moves = genSemiLegalBB<BISHOP>(pieceBB, blockers, bitboards);
                break;
            case KNIGHT:
                moves = genSemiLegalBB<KNIGHT>(pieceBB, blockers, bitboards);
                break;
            case ROOK:
                moves = genSemiLegalBB<ROOK>(pieceBB, blockers, bitboards);
                break;
            case QUEEN:
                moves = genSemiLegalBB<QUEEN>(pieceBB, blockers, bitboards);
                break;
            case KING:
                moves = genSemiLegalBB<KING>(pieceBB, blockers, bitboards);
                moves &= (~blockers.attackMap);
                break;
        }

        return moves;
    }

    U64 genAttackMap(Side &friendly, Bitboards &bitboards) {
        U64 attackMap = 0;
        Side enemy = friendly == WHITE ? BLACK : WHITE;

        // exclude the king from empty squares to stop moves backwards along rays from being generated
        U64 enemyPieces = bitboards.getSideBB(enemy);
        U64 friendlyKing = bitboards.getPieceBB(KING) & bitboards.getSideBB(friendly);
        U64 emptySquares = bitboards.EmptySquares | friendlyKing; // set the king square to empty

        // generate rook direction attacks
        U64 rook_and_queen = enemyPieces & (bitboards.getPieceBB(ROOK) | bitboards.getPieceBB(QUEEN));
        U64 horizontalAttacks = MoveGeneration::genAttackBB<ROOK>(rook_and_queen, emptySquares);
        attackMap |= horizontalAttacks;

        // generate bishop direction attacks
        U64 bishop_and_queen = enemyPieces & (bitboards.getPieceBB(BISHOP) | bitboards.getPieceBB(QUEEN));
        U64 diagAttacks = MoveGeneration::genAttackBB<BISHOP>(bishop_and_queen, emptySquares);
        attackMap |= diagAttacks;

        // generate pawn attacks
        U64 pawns = enemyPieces & bitboards.getPieceBB(PAWN);
        U64 pawnAttacks = MoveGeneration::genBulkAttackBB<PAWN>(pawns, enemy);
        attackMap |= pawnAttacks;

        // generate knight attacks
        U64 knights = enemyPieces & bitboards.getPieceBB(KNIGHT);
        U64 knightAttacks = MoveGeneration::genBulkAttackBB<KNIGHT>(knights, WHITE); // it doesn't matter what side we pass
        attackMap |= knightAttacks;

        // generate king attacks
        U64 king = enemyPieces & bitboards.getPieceBB(KING);
        if (king) {
            U64 kingAttacks = MoveGeneration::genAttackBB<KING>(king, emptySquares);
            attackMap |= kingAttacks;
        }

        emptySquares ^= friendlyKing; // set the king square to occupied

        return attackMap;
    }
    void genKingBlockers(Bitboards &bitboards, MoveGeneration::MoveGenBitboards &blockers) {
        Side enemy = blockers.friendly == WHITE ? BLACK : WHITE;

        // Returns the set of pieces which prevent a check. Includes both players' pieces for en-passant gen.
        U64 king = bitboards.getPieceBB(KING) & bitboards.getSideBB(blockers.friendly);
        U64 enemyPieces = bitboards.getSideBB(enemy);

        U64 rook_and_queen = enemyPieces & (bitboards.getPieceBB(ROOK) | bitboards.getPieceBB(QUEEN));
        U64 bishop_and_queen = enemyPieces & (bitboards.getPieceBB(BISHOP) | bitboards.getPieceBB(QUEEN));

        U64 kingMoves, enemyMoves;

        // first consider north and south moves
        kingMoves = MoveGeneration::genNorthBB(king, bitboards.EmptySquares) |
                MoveGeneration::genSouthBB(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNorthBB(rook_and_queen, bitboards.EmptySquares) |
                MoveGeneration::genSouthBB(rook_and_queen, bitboards.EmptySquares);
        blockers.NS = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider east and west moves - note uses the same pieces as up
        kingMoves = MoveGeneration::genWestBB(king, bitboards.EmptySquares) |
                MoveGeneration::genEastBB(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genWestBB(rook_and_queen, bitboards.EmptySquares) |
                MoveGeneration::genEastBB(rook_and_queen, bitboards.EmptySquares);
        blockers.EW = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider NE and SW moves
        kingMoves = MoveGeneration::genNeBB(king, bitboards.EmptySquares) |
                    MoveGeneration::genSwBB(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNeBB(bishop_and_queen, bitboards.EmptySquares) |
                     MoveGeneration::genSwBB(bishop_and_queen, bitboards.EmptySquares);
        blockers.NE = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider NW and SE moves
        kingMoves = MoveGeneration::genNwBB(king, bitboards.EmptySquares) |
                MoveGeneration::genSeBB(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNwBB(bishop_and_queen, bitboards.EmptySquares) |
                MoveGeneration::genSeBB(bishop_and_queen, bitboards.EmptySquares);
        blockers.NW = kingMoves & enemyMoves & bitboards.OccupiedSquares;
    }
    U64 getRayBetweenSquares(U64 &from, U64 &to, Bitboards &bitboards){
        U64 ray;
        ray = genNorthBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genSouthBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genEastBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genWestBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genNeBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genNwBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genSeBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;

        ray = genSwBB(from, bitboards.EmptySquares);
        if (ray & to) return ray | from;
    }
    U64 getSquareAttackers(U64 sq, Bitboards &bitboards, MoveGenBitboards &blockers) {
        // returns all the attacking pieces of square, by the current mover

        // treat this square as a 'super piece' and get all the possible moves from it
        // this sort of needs to be done in the reverse direction e.g. pawn pushes opposite
        U64 attacks = 0;
        U64 knight, diag, horiz, king, pawn;
        knight = genSemiLegalBB<KNIGHT>(sq, blockers, bitboards) & bitboards.getPieceBB(KNIGHT);
        diag = genSemiLegalBB<BISHOP>(sq, blockers, bitboards) & (bitboards.getPieceBB(BISHOP) | bitboards.getPieceBB(QUEEN));
        horiz = genSemiLegalBB<ROOK>(sq, blockers, bitboards) & (bitboards.getPieceBB(ROOK) | bitboards.getPieceBB(QUEEN));
        king = genSemiLegalBB<KING>(sq, blockers, bitboards) & bitboards.getPieceBB(KING);
        pawn = Masks::pawnCaptureMask[blockers.friendly][bitScanForward(sq)] & bitboards.getPieceBB(PAWN) & bitboards.getSideBB(blockers.enemy); // pawns need to move in opposite direction
        attacks = (knight | diag | horiz | king | pawn);

        attacks &= bitboards.getSideBB(blockers.enemy);

        return attacks;
    }

    inline void genPawnLegalMoves(MoveGenBitboards &blockers, Bitboards &bitboards, MoveListsContainer&moveLists) {
        U64 promotingPawnsRank = blockers.friendly == WHITE ? Masks::Rank7 : Masks::Rank2;

        U64 pawns = bitboards.getPieceBB(PAWN) & bitboards.getSideBB(blockers.friendly);
        U64 nonPromotingPawns = pawns & ~promotingPawnsRank;
        U64 promotingPawns = pawns & promotingPawnsRank;

        while (nonPromotingPawns) {
            U64 generatingPawn = popLSB(nonPromotingPawns);
            short generatingPawnSquare = bitScanForward(generatingPawn);

            U64 nonPromoPawnMoves = genSemiLegalBB<PAWN>(generatingPawn, blockers, bitboards);
            U64 quietMoves = nonPromoPawnMoves & bitboards.EmptySquares, activeMoves = nonPromoPawnMoves & ~bitboards.EmptySquares;
            if (quietMoves) fillMoveList<Quiet>(moveLists.quietMoveList, bitboards, PAWN, generatingPawnSquare, quietMoves);
            if (activeMoves) fillMoveList<Active>(moveLists.activeMoveList, bitboards, PAWN, generatingPawnSquare,activeMoves);

            U64 enPassantMove = genSemiLegalBB<ENPASSANTPAWNS>(generatingPawn, blockers, bitboards);
            if (enPassantMove) fillMoveList<EnPassant>(moveLists.activeMoveList, bitboards, PAWN, generatingPawnSquare, enPassantMove);
        }

        while (promotingPawns) {
            U64 generatingPawn = popLSB(promotingPawns);
            short generatingPawnSquare = bitScanForward(generatingPawn);

            U64 promoPawnMoves = genSemiLegalBB<PAWN>(generatingPawn, blockers, bitboards);
            if (promoPawnMoves) fillMoveList<Promo>(moveLists.activeMoveList, bitboards, PAWN, generatingPawnSquare, promoPawnMoves);
        }
    }
    inline void genCastling(MoveGenBitboards &blockers, Bitboards &bitboards, MoveListsContainer&moveLists) {
        Side friendly = blockers.friendly;
        SpecialMoveRights subRights;
        short king, left, right;
        if (friendly == WHITE) {
            subRights = bitboards.castleRights;
            king = E1;
            left = A1;
            right = H1;
        } else {
            subRights = bitboards.castleRights >> 2;
            king = E8;
            left = A8;
            right = H8;
        }

        if ((subRights & 1) &&
            !(Masks::ClearCastleLaneMasks[friendly][0] & bitboards.OccupiedSquares) &&
            !((Masks::ClearCastleLaneMasks[friendly][0] << 1) & blockers.attackMap))
        {
            moveLists.quietMoveList->emplace_back(encodeMove(king, left, 0, 3, KING, ROOK));
        }

        if ((subRights & 2) &&
            !(Masks::ClearCastleLaneMasks[friendly][1] & bitboards.OccupiedSquares) &&
            !(Masks::ClearCastleLaneMasks[friendly][1] & blockers.attackMap))
        {
            moveLists.quietMoveList->emplace_back(encodeMove(king, right, 0, 3, KING, ROOK));
        }
    }
    inline void genStandardLegalMoves(MoveGenBitboards &blockers, Bitboards &bitboards, MoveListsContainer&moveLists, short numKingAttackers) {
        Side friendly = blockers.friendly;

        if (numKingAttackers <= 1) genPawnLegalMoves(blockers, bitboards, moveLists);
        if (numKingAttackers == 0) genCastling(blockers, bitboards, moveLists);
        for (Pieces piece: {KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
            // when there is a double check, we can only move the king
            if ((numKingAttackers >= 2) && (piece != KING)) continue;

            U64 generatingPieces = bitboards.getPieceBB(piece) & bitboards.getSideBB(friendly);

            while (generatingPieces) {
                U64 pieceBB = popLSB(generatingPieces);
                short pieceIndex = bitScanForward(pieceBB);

                U64 moves = genSemiLegalBBWrapper(pieceBB, piece, blockers, bitboards);
                U64 legalMoves = moves;

                if (piece != KING) legalMoves &= blockers.validDestinationSquares;
                U64 activeMoves = legalMoves & bitboards.getSideBB(blockers.enemy);
                U64 quietMoves = legalMoves & bitboards.EmptySquares;

                if (quietMoves) fillMoveList<Quiet>(moveLists.quietMoveList, bitboards, piece, pieceIndex, quietMoves);
                if (activeMoves) fillMoveList<Active>(moveLists.activeMoveList, bitboards, piece, pieceIndex,
                                     activeMoves);
            }
        }
    }
}

// The best way to do pawns is loop through each individual pawn and generate promo/ en-passant/ captures ect one at a time!
MoveList Board::genMoves() {
    // build the move-lists
    MoveList activeMoveList, quietMoveList, combinedMoveList;
    activeMoveList.reserve(10);
    quietMoveList.reserve(40);
    combinedMoveList.reserve(50);

    MoveGeneration::MoveListsContainer moveLists(&quietMoveList, &activeMoveList, &combinedMoveList);

    // build the blockers
    MoveGeneration::MoveGenBitboards blockers{};
    blockers.friendly = this->currentSide;
    blockers.enemy = this->otherSide;
    MoveGeneration::genKingBlockers(this->bitboards, blockers); // find pinned pieces
    blockers.attackMap = MoveGeneration::genAttackMap(this->currentSide, this->bitboards); // to see if we are in check
    blockers.validDestinationSquares = ~(0);

    U64 king = this->bitboards.getPieceBB(KING) & this->bitboards.getSideBB(this->currentSide);
    bool inCheck = king & blockers.attackMap;
    short numKingAttackers;

    if (inCheck) {
        // first we generate attackers to the king square
        U64 attackers = MoveGeneration::getSquareAttackers(king, bitboards, blockers);
        numKingAttackers = count(attackers);

        if (numKingAttackers == 1) {
            // find the piece that is attacking
            short attackingPiece = bitboards.getPieceAt(attackers);

            // check if attacking piece is a slider
            if ((attackingPiece == ROOK) || (attackingPiece == BISHOP) || (attackingPiece == QUEEN)) {
                blockers.validDestinationSquares &= MoveGeneration::getRayBetweenSquares(king, attackers, bitboards);
            } else {
                // else the checking ray must only contain the attacker
                blockers.validDestinationSquares &= attackers;
            }
        }
    } else {
        numKingAttackers = 0;
    }
    MoveGeneration::genStandardLegalMoves(blockers, this->bitboards, moveLists, numKingAttackers);

    moveLists.combineLists();
    return *moveLists.combinedMoveList;
}

#endif