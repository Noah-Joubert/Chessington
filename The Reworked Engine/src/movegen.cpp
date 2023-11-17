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

    U64* knightMasks;
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


    // TODO call this function
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
    };

    /* Generates sliding moves in a certain direction */
    inline U64 genNorthMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genSouthMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genWestMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genEastMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genNWMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genSWMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genNEMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genSEMoves(U64 &generatingPieces, U64 &emptySquares) {
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
    inline U64 genAttack(U64 generatingPiece, U64 &emptySquares);
    template<> inline U64 genAttack<ROOK>(U64 generatingPiece, U64 &emptySquares) {
        return (genNorthMoves(generatingPiece, emptySquares) | genSouthMoves(generatingPiece, emptySquares) |
                genWestMoves(generatingPiece, emptySquares) | genEastMoves(generatingPiece, emptySquares));
    };
    template<> inline U64 genAttack<BISHOP>(U64 generatingPiece, U64 &emptySquares) {
        return (genNWMoves(generatingPiece, emptySquares) | genNEMoves(generatingPiece, emptySquares) |
                genSEMoves(generatingPiece, emptySquares) | genSWMoves(generatingPiece, emptySquares));
    }
    template<> inline U64 genAttack<QUEEN>(U64 generatingPiece, U64 &emptySquares) {
        return genAttack<ROOK>(generatingPiece, emptySquares) | genAttack<BISHOP>(generatingPiece, emptySquares);
    }
    template<> inline U64 genAttack<KNIGHT>(U64 generatingPiece, U64 &emptySquares) {
        if (generatingPiece == 0) return 0;

        return Masks::knightMasks[bitScanForward(generatingPiece)];
    }
    template<> inline U64 genAttack<KING>(U64 generatingPiece, U64 &emptySquares) {
        return Masks::kingMasks[bitScanForward(generatingPiece)];
    }
    U64 genAttackWrapper(Pieces TEMPLATE, U64 generatingPiece, U64 &emptySquares) {
        // a function used to get past annoying template issues
        //TODO try a switch -> bitwise
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

        assert("Invalid piece for genAttackWrapper");
        return 0;
    }

    /* This generates the bitboard of squares attacked by a set of pieces */
    template <Pieces T>
    inline U64 genBulkAttack(U64 generatingPieces, Side side);
    template<> inline U64 genBulkAttack<PAWN>(U64 generatingPieces, Side side) {
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
    template<> inline U64 genBulkAttack<KNIGHT>(U64 generatingPieces, Side side) {
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
        Promo = 2
    };
    inline Move encodeMove(short startSquare, short endSquare, short promoCode, short moveFlag, short startType, short endType) {
        return (startSquare) | (endSquare << 6) | (promoCode << 12) | (moveFlag << 14) | (startType << 16) | (endType << 19);
    }
    template <MoveTypes T> // TODO: clean up this logic
    void fillMoveList(MoveList &moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB);
    template<> void fillMoveList<Quiet>(MoveList &moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        while (moveBB) {
            const short endSquare = popIntLSB(moveBB);
            Move move = encodeMove(startSquare, endSquare, 0, 0, startPiece, EMPTY);

            moveList.emplace_back(move);
        }
    }
    template<> void fillMoveList<Active>(MoveList &moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        if (moveBB == 0) return;

        for (Pieces endPiece: {PAWN, KNIGHT, KING, QUEEN, BISHOP, ROOK}) {
            U64 attackedPieces = moveBB & bitboards.getPieceBB(endPiece);

            while (attackedPieces) {
                short endSquare = popIntLSB(attackedPieces);
                Move move = encodeMove(startSquare, endSquare, 0, 0, startPiece, endPiece);

                moveList.emplace_back(move);
            }

            moveBB &= (~attackedPieces);
            if (!moveBB) break;
        }
    }
    template<> void fillMoveList<Promo>(MoveList &moveList, Bitboards &bitboards, Pieces startPiece, short startSquare, U64 moveBB) {
        U64 quietPromos = moveBB & bitboards.EmptySquares;
        while (quietPromos) {
            const short endSquare = popIntLSB(quietPromos);

            Move m1 = encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, EMPTY);
            Move m2 = encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, EMPTY);
            Move m3 = encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, EMPTY);
            Move m4 = encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, EMPTY);
            moveList.emplace_back(m1);
            moveList.emplace_back(m2);
            moveList.emplace_back(m3);
            moveList.emplace_back(m4);
        }

        U64 activePromos = moveBB & (~bitboards.EmptySquares);
        for (Pieces endPiece: {PAWN, BISHOP, KNIGHT, QUEEN, KING, ROOK}) {
            U64 attackedPieces = activePromos & bitboards.getPieceBB(endPiece);

            while (attackedPieces) {
                short endSquare = popIntLSB(attackedPieces);

                Move m1 = encodeMove(startSquare, endSquare, KNIGHTPROMO, PROMOTION, PAWN, endPiece);
                Move m2 = encodeMove(startSquare, endSquare, QUEENPROMO, PROMOTION, PAWN, endPiece);
                Move m3 = encodeMove(startSquare, endSquare, BISHOPPROMO, PROMOTION, PAWN, endPiece);
                Move m4 = encodeMove(startSquare, endSquare, ROOKPROMO, PROMOTION, PAWN, endPiece);
                moveList.emplace_back(m1);
                moveList.emplace_back(m2);
                moveList.emplace_back(m3);
                moveList.emplace_back(m4);
            }

            activePromos &= (~attackedPieces);
            if (!activePromos) break;
        }
    }

    /* Returns the set of semi-legal moves for a set of pieces - meaning there are no discovered checks */
    template <Pieces T>
    inline U64 genSemiLegal(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards);
    template<> inline U64 genSemiLegal<BISHOP>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 horizontalBlockers = ~(blockers.NS | blockers.EW);
        piece &= horizontalBlockers;

        // consider blockers
        U64 bishopNW = piece & (~blockers.NE);
        U64 bishopNE = piece & (~blockers.NW);

        // combine the moves
        U64 moves = genNEMoves(bishopNE, bitboards.EmptySquares) | genSWMoves(bishopNE, bitboards.EmptySquares)
                    | genNWMoves(bishopNW, bitboards.EmptySquares) | genSEMoves(bishopNW, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegal<KNIGHT>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 generatingPiece = piece & ~(blockers.NS | blockers.EW | blockers.NE | blockers.NW); // consider blockers
        U64 moves = genAttack<KNIGHT>(generatingPiece, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegal<ROOK>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 diagonalBlockers = blockers.NE | blockers.NW;
        U64 generatingPiece = piece & (~diagonalBlockers); // get the set of pieces

        U64 rookNS = generatingPiece & (~blockers.EW);
        U64 rookEW = generatingPiece & (~blockers.NS);
        U64 moves = genNorthMoves(rookNS, bitboards.EmptySquares) | genSouthMoves(rookNS, bitboards.EmptySquares)
                    | genEastMoves(rookEW, bitboards.EmptySquares) | genWestMoves(rookEW, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegal<KING>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 moves = genAttack<KING>(piece, bitboards.EmptySquares);

        return moves;
    }
    template<> inline U64 genSemiLegal<QUEEN>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        return genSemiLegal<ROOK>(piece, blockers, bitboards) |
               genSemiLegal<BISHOP>(piece, blockers, bitboards);
    }
    template<> inline U64 genSemiLegal<PAWN>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        Side friendly = blockers.friendly, enemy = blockers.enemy;
        short upLeft, upRight;
        U64 doublePushRank;
        if (friendly == WHITE) {
            doublePushRank = Masks::Rank3;
            upLeft = Masks::noWe;
            upRight = Masks::noEa;
        } else if (friendly == BLACK) {
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
            moves |= (firstPush | secondPush) & blockers.validDestinationSquares;
        }

        U64 capturingPawn = piece & ~(blockers.NS | blockers.EW);
        U64 leftCaptures = Masks::shiftBitboard(capturingPawn & ~(blockers.NE), upLeft);
        U64 rightCaptures = Masks::shiftBitboard(capturingPawn & ~(blockers.NW), upRight);
        moves |= (bitboards.getSideBB(enemy) & blockers.validDestinationSquares) & (leftCaptures | rightCaptures);

        return moves;
    }
    template<> inline U64 genSemiLegal<PROMOPAWNS>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        return genSemiLegal<PAWN>(piece, blockers, bitboards);
    }
    template<> inline U64 genSemiLegal<NONPROMOPAWNS>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        return genSemiLegal<PAWN>(piece, blockers, bitboards);
    }
    template<> inline U64 genSemiLegal<ENPASSANTPAWNS>(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        // TODO
        Move moves = 0;
        if (!bitboards.enPassantRights) return moves;

        Side friendly = blockers.friendly;
        U64 enPassantRank;
        U64 enPassantRightsMask = bitboards.enPassantRights;
        U64 leftCapturedPawn, rightCapturedPawn;
        short upLeft, upRight;
        if (friendly == WHITE) {
            upLeft = Masks::noWe;
            upRight = Masks::noEa;
            enPassantRank = Masks::Rank5;
            enPassantRightsMask <<= (2 * 8);
        } else {
            upLeft = Masks::soEa;
            upRight = Masks::soWe;
            enPassantRank = Masks::Rank4;
            enPassantRightsMask <<= (5 * 8);
        }

        // check the pawn is on the correct rank, and not a blocker
        U64 pawn = piece & enPassantRank & ~(blockers.NS | blockers.EW);
        if (!pawn) return moves;

        // generate the left and right en-passant take. make sure we have the rights for the capture
        U64 enPassantLeft = Masks::shiftBitboard(pawn & ~blockers.NE, upLeft) & enPassantRightsMask;
        U64 enPassantRight = Masks::shiftBitboard(pawn & ~blockers.NW, upRight) & enPassantRightsMask;

        // now check the taken piece wasn't a blocker

        return 0;
    }
    U64 genSemiLegalWrapper(U64 piece, MoveGenBitboards &blockers, Bitboards &bitboards) {
        U64 moves;

        switch (piece) {
            case BISHOP:
                moves = genSemiLegal<BISHOP>(piece, blockers, bitboards);
                break;
            case KNIGHT:
                moves = genSemiLegal<KNIGHT>(piece, blockers, bitboards);
                break;
            case ROOK:
                moves = genSemiLegal<ROOK>(piece, blockers, bitboards);
                break;
            case QUEEN:
                moves = genSemiLegal<QUEEN>(piece, blockers, bitboards);
                break;
            case KING:
                moves = genSemiLegal<KING>(piece, blockers, bitboards);
                moves &= (~blockers.attackMap);
                break;
        }

        return moves;
    }

    inline void genLegalMoves(MoveGenBitboards &blockers, Bitboards &bitboards, MoveListsContainer&moveLists) {
        Side friendly = blockers.friendly, enemy = blockers.enemy;
        MoveList quietMoveList, activeMoveList;

        for (Pieces piece: {KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
            // TODO castles
            U64 generatingPieces = bitboards.getPieceBB(piece) & bitboards.getSideBB(friendly);

            U64 promoRank = blockers.friendly == WHITE ? Masks::Rank7 : Masks::Rank2;
            if (piece == PROMOPAWNS) {
                generatingPieces &= promoRank;
            } else if (piece == NONPROMOPAWNS) {
                generatingPieces &= ~promoRank;
            }

            while (generatingPieces) {
                U64 generatingPiece = popLSB(generatingPieces);
                short generatingPieceIndex = bitScanForward(generatingPiece);

                U64 moves = genSemiLegalWrapper(piece, blockers, bitboards);
                U64 legalMoves = moves & blockers.validDestinationSquares;
                U64 activeMoves = legalMoves & bitboards.getSideBB(blockers.enemy);
                U64 quietMoves = legalMoves & bitboards.EmptySquares;

                if (piece == PROMOPAWNS) {
                    fillMoveList<Promo>(*moveLists.activeMoveList, bitboards, piece, generatingPieceIndex, quietMoves);
                    fillMoveList<Promo>(*moveLists.activeMoveList, bitboards, piece, generatingPieceIndex, activeMoves);
                } else {
                    fillMoveList<Quiet>(*moveLists.quietMoveList, bitboards, piece, generatingPieceIndex, quietMoves);
                    fillMoveList<Active>(*moveLists.activeMoveList, bitboards, piece, generatingPieceIndex,
                                         activeMoves);
                }
            }
        }
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
        U64 horizontalAttacks = MoveGeneration::genAttack<ROOK>(rook_and_queen, emptySquares);
        attackMap |= horizontalAttacks;

        // generate bishop direction attacks
        U64 bishop_and_queen = enemyPieces & (bitboards.getPieceBB(BISHOP) | bitboards.getPieceBB(QUEEN));
        U64 diagAttacks = MoveGeneration::genAttack<BISHOP>(bishop_and_queen, emptySquares);
        attackMap |= diagAttacks;

        // generate pawn attacks
        U64 pawns = enemyPieces & bitboards.getPieceBB(PAWN);
        U64 pawnAttacks = MoveGeneration::genBulkAttack<PAWN>(pawns, enemy);
        attackMap |= pawnAttacks;

        // generate knight attacks
        U64 knights = enemyPieces & bitboards.getPieceBB(KNIGHT);
        U64 knightAttacks = MoveGeneration::genBulkAttack<KNIGHT>(knights, WHITE); // it doesn't matter what side we pass
        attackMap |= knightAttacks;

        // generate king attacks
        U64 king = enemyPieces & bitboards.getPieceBB(KING);
        if (king) {
            U64 kingAttacks = MoveGeneration::genAttack<KING>(king, emptySquares);
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
        kingMoves = MoveGeneration::genNorthMoves(king, bitboards.EmptySquares) | MoveGeneration::genSouthMoves(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNorthMoves(rook_and_queen, bitboards.EmptySquares) | MoveGeneration::genSouthMoves(rook_and_queen, bitboards.EmptySquares);
        blockers.NS = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider east and west moves - note uses the same pieces as up
        kingMoves = MoveGeneration::genWestMoves(king, bitboards.EmptySquares) | MoveGeneration::genEastMoves(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genWestMoves(rook_and_queen, bitboards.EmptySquares) | MoveGeneration::genEastMoves(rook_and_queen, bitboards.EmptySquares);
        blockers.EW = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider NE and SW moves
        kingMoves = MoveGeneration::genNEMoves(king, bitboards.EmptySquares) | MoveGeneration::genSWMoves(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNEMoves(bishop_and_queen, bitboards.EmptySquares) | MoveGeneration::genSWMoves(bishop_and_queen, bitboards.EmptySquares);
        blockers.NE = kingMoves & enemyMoves & bitboards.OccupiedSquares;

        // next consider NW and SE moves
        kingMoves = MoveGeneration::genNWMoves(king, bitboards.EmptySquares) | MoveGeneration::genSEMoves(king, bitboards.EmptySquares);
        enemyMoves = MoveGeneration::genNWMoves(bishop_and_queen, bitboards.EmptySquares) | MoveGeneration::genSEMoves(bishop_and_queen, bitboards.EmptySquares);
        blockers.NW = kingMoves & enemyMoves & bitboards.OccupiedSquares;
    }
}

/* 1). Add read FEN functionality
 *
 * */

/* I now want to do a shit tonne of unit testing */

MoveList Board::genMoves() {
    MoveList activeMoveList, quietMoveList, combinedMoveList;
    MoveGeneration::MoveListsContainer moveLists(&quietMoveList, &activeMoveList, &combinedMoveList);
    MoveGeneration::MoveGenBitboards blockers{};

    genKingBlockers(this->bitboards, blockers); // find pinned pieces
    blockers.attackMap = MoveGeneration::genAttackMap(this->currentSide, this->bitboards); // to see if we are in check
    blockers.validDestinationSquares = ~(0);
    blockers.friendly = this->currentSide;
    blockers.enemy = this->otherSide;

    U64 king = this->bitboards.getPieceBB(KING) & this->bitboards.getSideBB(this->currentSide);
    short numAttackers = 0;
    bool inCheck = king & blockers.attackMap;
    if (inCheck) {

    } else {
        MoveGeneration::genLegalMoves(blockers, this->bitboards, moveLists);
    }
}