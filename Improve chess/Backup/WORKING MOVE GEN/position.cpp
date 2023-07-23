//
// Created by Noah Joubert on 2021-04-22.
//

#ifndef FILE_pos_SEEN
#define FILE_pos_SEEN

#include "types.h"
#include "bitboards.cpp"
#include "misc.cpp"

struct Position {
    short moveNumber = 1; // how many moves have been made

    U64 pieceBB[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // one extra redundant board is kept
    U64 occupiedSquares, emptySquares; // emptySquares and it's compliment
    U64 blockersNS, blockersEW, blockersNE, blockersNW; // blockers for all the ray directions
    U64 attackMap;

    MoveList activeMoveList; // stores the active moves
    MoveList quietMoveList; // stores the quiet moves
    MoveList combinedMoveList; // stores both types of moves

    MoveList moveHistory; // stores past moves

    short currentSide = WHITE, otherSide = BLACK, friendly = nWhite, enemy = nBlack;
    CastleRights wCastle, bCastle;
    CRights CstleRights;
    vector<CRights> CastleRightsHistory;
    EnPassantRights enPassantRights = 0;
    EnPassantRightStack enPassantHistory; // stores past enpassant rights

    U64 checkingRay;

    bool incheck;

    /* move gen stuff */
    void genBlockers();
    void genAttackMap();
    void genKingMoves();
    void genRookMoves();
    void genBishopMoves();
    void genQueenMoves();
    void genKnightMoves();
    void genPawnMoves();
    void genPawnMovesNew();
    void genCastlingNew();
    void genCastlingOld(CastleRights &rights);
    void genAllMoves();
    bool checkKingCheck(short SIDE);
    short getPieceAt(U64 &sq);
    U64 getSquareAttackers(U64 sq, short SIDE);
    U64 getRay(U64 &from, U64 &to);
    MoveList getMoveList() {
        genAllMoves();

        return combinedMoveList;
    }

    void initPieceBitboards(bool pawns, bool knights, bool bishops, bool rooks, bool kings, bool queens) {
        // next do pawns
        if (pawns) {
            for (int sq_b = A7, sq_w = A2; sq_b <= H7; sq_w++, sq_b++) {
                // create the power of 2 corresponding to a particular square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[PAWN] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'

                // create the power of 2 corresponding to a particular square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[PAWN] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'
            }
        }

        // next do rooks
        if (rooks) {
            int rookSquares[4] = {A1, H1, A8, H8};
            for (int i = 0; i < 2; i++) {
                int sq_w = rookSquares[i]; // get the rook square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[ROOK] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'

                int sq_b = rookSquares[i + 2]; // get the rook square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[ROOK] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'
            }
        }

        if (bishops) {
            // next do bishops
            int bishopSquares[4] = {C1, F1, C8, F8};
            for (int i = 0; i < 2; i++) {
                int sq_w = bishopSquares[i]; // get the rook square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[BISHOP] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'

                int sq_b = bishopSquares[i + 2]; // get the rook square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[BISHOP] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'
            }
        }

        if (knights) {
            // next do knights
            int knightSquares[4] = {B1, G1, B8, G8};
            for (int i = 0; i < 2; i++) {
                int sq_w = knightSquares[i]; // get the rook square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[KNIGHT] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'

                int sq_b = knightSquares[i + 2]; // get the rook square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[KNIGHT] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'
            }
        }

        if (kings) {
            // next do kings
            int kingSquares[2] = {E1, E8};
            for (int i = 0; i < 1; i++) {
                int sq_w = kingSquares[i]; // get the rook square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[KING] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'

                int sq_b = kingSquares[i + 1]; // get the rook square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[KING] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'
            }
        }

        if (queens) {
            // next do queens
            int queenSquares[2] = {D1, D8};
            for (int i = 0; i < 1; i++) {
                int sq_w = queenSquares[i]; // get the rook square
                U64 singleBit_w = C64(1) << sq_w; // shift a 1 'square' bits right
                pieceBB[QUEEN] |= singleBit_w; // toggle the bit 'on'
                pieceBB[nWhite] |= singleBit_w; // toggle the bit 'on'

                int sq_b = queenSquares[i + 1]; // get the rook square
                U64 singleBit_b = C64(1) << sq_b; // shift a 1 'square' bits right
                pieceBB[QUEEN] |= singleBit_b; // toggle the bit 'on'
                pieceBB[nBlack] |= singleBit_b; // toggle the bit 'on'
            }
        }
    }

    /* init stuff */
    void init() {
        initPieceBitboards(true, true, true, true, true, true);

        initCastling();

        // create the 'empty/occupied squares' BB
        occupiedSquares = (pieceBB[nWhite] | pieceBB[nBlack]);
        emptySquares = ~occupiedSquares;

        /* this should be looked into/optimised */
        activeMoveList.reserve(100);
        quietMoveList.reserve(100);
        moveHistory.reserve(100);
        combinedMoveList.reserve(200);
        enPassantHistory.reserve(100);
    }
    void initCastling() {
        CstleRights = ~(0);

        // set up the castling rights
        wCastle.king = wKing;
        wCastle.left = wLeft;
        wCastle.right = wRight;
        wCastle.leftMask = 1008806316530991104;
        wCastle.rightMask = 6917529027641081856;

        bCastle.king = bKing;
        bCastle.left = bLeft;
        bCastle.right = bRight;
        bCastle.leftMask = 14;
        bCastle.rightMask = 96;
    }

    /* make move */
    void switchSide() {
        if (currentSide == WHITE) {
            currentSide = BLACK;
            otherSide = WHITE;
            friendly = nBlack;
            enemy = nWhite;
        } else {
            currentSide = WHITE;
            otherSide = BLACK;
            friendly = nWhite;
            enemy = nBlack;
        }
    }
    void updateCastleRightsOld() {
        // should be called after every make move.
        wCastle.king &= (pieceBB[KING] & pieceBB[nWhite]);
        wCastle.left &= (pieceBB[ROOK] & pieceBB[nWhite]);
        wCastle.right &= (pieceBB[ROOK] & pieceBB[nWhite]);

        bCastle.king &= (pieceBB[KING] & pieceBB[nBlack]);
        bCastle.left &= (pieceBB[ROOK] & pieceBB[nBlack]);
        bCastle.right &= (pieceBB[ROOK] & pieceBB[nBlack]);

        // see if any castle rights have been lost
        if ((!wCastle.king) && (wCastle.kingBreak == -2)) {
            // check for a king break first of all
            wCastle.kingBreak = moveNumber;
        }
        if ((!wCastle.left) && (wCastle.leftBreak == -2)) {
            // check for a left break first of all
            wCastle.leftBreak = moveNumber;
        }
        if ((!wCastle.right) && (wCastle.rightBreak == -2)) {
            // check for a left break first of all
            wCastle.rightBreak = moveNumber;
        }

        // see if any castle rights have been lost
        if ((!bCastle.king) && (bCastle.kingBreak == -2)) {
            // check for a king break first of all
            bCastle.kingBreak = moveNumber;
        }
        if ((!bCastle.left) && (bCastle.leftBreak == -2)) {
            // check for a left break first of all
            bCastle.leftBreak = moveNumber;
        }
        if ((!bCastle.right && bCastle.rightBreak == -2)) {
            // check for a left break first of all
            bCastle.rightBreak = moveNumber;
        }
    }
    void updateCastleRightsNew() {
        CastleRightsHistory.emplace_back(CstleRights);

        U64 w = (pieceBB[KING] | pieceBB[ROOK]) & pieceBB[nWhite];
        if ((CstleRights & 1) && (w & UpdateCastleMasks[WHITE][0]) != UpdateCastleMasks[WHITE][0]) CstleRights ^= 1;
        if ((CstleRights & 2) && (w & UpdateCastleMasks[WHITE][1]) != UpdateCastleMasks[WHITE][1]) CstleRights ^= 2;

        U64 b = (pieceBB[KING] | pieceBB[ROOK]) & pieceBB[nBlack];
        if ((CstleRights & 4) && (b & UpdateCastleMasks[BLACK][0]) != UpdateCastleMasks[BLACK][0]) CstleRights ^= 4;
        if ((CstleRights & 8) && (b & UpdateCastleMasks[BLACK][1]) != UpdateCastleMasks[BLACK][1]) CstleRights ^= 8;
    }
    void undoCastleRightsNew() {
        CstleRights = CastleRightsHistory.back();
        CastleRightsHistory.pop_back();
    }
    void undoCastleRightsOld() {
        if (wCastle.leftBreak == moveNumber) {
            wCastle.left = wLeft;
        }
        if (wCastle.rightBreak == moveNumber) {
            wCastle.right = wRight;
        }
        if (wCastle.kingBreak == moveNumber) {
            wCastle.king = wKing;
        }

        if (bCastle.leftBreak == moveNumber) {
            bCastle.left = bLeft;
        }
        if (bCastle.rightBreak == moveNumber) {
            bCastle.right = bRight;
        }
        if (bCastle.kingBreak == moveNumber) {
            bCastle.king = bKing;
        }
    }
    inline void decodeMove(Move move, short &from, short &to, short &promo, short &flag, short &fromType, short &toType) {
        from = move & fromMask;
        to = (move & toMask) >> 6;
        promo = (move & promoMask) >> 12;
        flag = (move & flagMask) >> 14;
        fromType = (move & fromTypeMask) >> 16;
        toType = (move & toTypeMask) >> 19;
    }
    void makeMove(Move &move) {
        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);

        enPassantHistory.emplace_back(enPassantRights);
        enPassantRights = 0;

        U64 fromBB = C64(1) << from; // all 0's, with the from square a 1
        U64 fromBBx = ~fromBB; // all 1's, with the from square a 0
        U64 toBB = C64(1) << to; // all 0's from the to square a 1
        U64 toBBx = ~toBB; // all 1's with the from square a -
        U64 fromToBB = fromBB ^ toBB; // all 0's with the from and two squares 1

        if (flag == ENPASSANT) {
            // do an enpassant
            U64 enPassSquare;

            if (currentSide == WHITE) {
                enPassSquare = toBB << 8;
            } else {
                enPassSquare = toBB >> 8;
            }

            // first move the pawn
            pieceBB[PAWN] ^= fromToBB;
            pieceBB[friendly] ^= fromToBB;

            // now take the oppenents pawn
            pieceBB[PAWN] ^= enPassSquare;
            pieceBB[enemy] ^= enPassSquare;

            // update occupied & empty squares
            emptySquares ^= fromToBB | enPassSquare;
            occupiedSquares ^= fromToBB | enPassSquare;
        } else if (flag == CASTLING) {
            // do a castle

            // first reset the castle and king squares
            pieceBB[KING] &= fromBBx;
            pieceBB[ROOK] &= toBBx;

            // reset the colours
            pieceBB[friendly] &= ~fromToBB;

            U64 newRook, newKing;

            if (currentSide == WHITE) {
                if (toBB & castleLeftMask) {
                    newRook = C64(1) << D1;
                    newKing = C64(1) << C1;
                } else {
                    newRook = C64(1) << F1;
                    newKing = C64(1) << G1;
                }
            } else {
                if (toBB & castleLeftMask) {
                    newRook = C64(1) << D8;
                    newKing = C64(1) << C8;
                } else {
                    newRook = C64(1) << F8;
                    newKing = C64(1) << G8;
                }
            }

            pieceBB[ROOK] |= newRook;
            pieceBB[KING] |= newKing;
            pieceBB[friendly] |= newKing | newRook;

            // deal with occupied and empty squares
            U64 allChanges = fromToBB | newKing | newRook;
            occupiedSquares ^= allChanges;
            emptySquares ^= allChanges;
        } else {
            // normal move
            if (toType == EMPTY) {
                /* quiet move */

                // Considering from colour: F and T squares will be toggled - use XOR
                pieceBB[friendly] ^= fromToBB;

                // Considering to colour: No changes

                // Considering from type: F and T squares will be toggled like with from colour - use XOR
                pieceBB[fromType] ^= fromToBB;

                // Considering to type: No change as it's move is to an empty square

                // Considering empty and occupied squares: both will be toggled
                emptySquares ^= fromToBB;
                occupiedSquares ^= fromToBB;

                // check for double pawn push and update enpassant rights
                if ((fromType == PAWN) && ((from - to) % 16 == 0)) {
                    enPassantRights |= 1 << (to % 8);
                }
            } else {
                /* capture */

                // Considering from colour: F and T squares will be toggled - use XOR
                pieceBB[friendly] ^= fromToBB;

                // Considering to colour: To square is reset
                pieceBB[enemy] &= toBBx;

                // When considering F and T types you need to consider the case when the F and T types are the same
                // So first we reset the T square for the T type.
                // Then toggle the F and T squares for the F type.
                pieceBB[toType] &= toBBx;
                pieceBB[fromType] ^= fromToBB;

                // The F square will be toggled for the occupied and empty boards
                emptySquares ^= fromBB;
                occupiedSquares ^= fromBB;
            }

            // if it was a promotion, set the destination square to the promotion piece
            if (flag == PROMOTION) {
                pieceBB[PAWN] ^= toBB;
                if (promo == KNIGHTPROMO) {
                    pieceBB[KNIGHT] ^= toBB;
                } else if (promo == QUEENPROMO) {
                    pieceBB[QUEEN] ^= toBB;
                } else if (promo == ROOKPROMO) {
                    pieceBB[ROOK] ^= toBB;
                } else if (promo == BISHOPPROMO) {
                    pieceBB[BISHOP] ^= toBB;
                }
            }
        }

        updateCastleRightsNew();
        moveHistory.emplace_back(move); // store the move so it can be undone
        switchSide(); // switch the side
        moveNumber++; // increment the move number
    }
    void unMakeMove() {
        Move move = moveHistory.back(); // get the last move played
        moveHistory.pop_back();

        enPassantRights = enPassantHistory.back();
        enPassantHistory.pop_back();

        switchSide(); // switch the side
        moveNumber--; // decrease the move number

        undoCastleRightsNew();

        // decode the move
        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);

        U64 fromBB = C64(1) << from; // all 0's, with the from square a 1
        U64 fromBBx = ~fromBB; // all 1's, with the from square a 0
        U64 toBB = C64(1) << to; // all 0's from the to square a 1
        U64 toBBx = ~toBB; // all 1's with the from square a -
        U64 fromToBB = fromBB ^toBB; // all 0's with the from and two squares 1

        if (flag == ENPASSANT) {
            // do an enpassant
            U64 enPassSquare;

            if (currentSide == WHITE) {
                enPassSquare = toBB << 8;
            } else {
                enPassSquare = toBB >> 8;
            }

            // first move back the pawn
            pieceBB[PAWN] ^= fromToBB;
            pieceBB[friendly] ^= fromToBB;

            // now take the openents pawn
            pieceBB[PAWN] ^= enPassSquare;
            pieceBB[enemy] ^= enPassSquare;

            // update occupied & empty squares
            emptySquares ^= fromToBB | enPassSquare;
            occupiedSquares ^= fromToBB | enPassSquare;
        } else if (flag == CASTLING) {
            // do a castle

            U64 newRook, newKing;
            if (currentSide == WHITE) {
                if (toBB & castleLeftMask) {
                    newRook = C64(1) << D1;
                    newKing = C64(1) << C1;
                } else {
                    newRook = C64(1) << F1;
                    newKing = C64(1) << G1;
                }
            } else {
                if (toBB & castleLeftMask) {
                    newRook = C64(1) << D8;
                    newKing = C64(1) << C8;
                } else {
                    newRook = C64(1) << F8;
                    newKing = C64(1) << G8;
                }
            }

            // deal with the moving colour
            pieceBB[friendly] |= fromToBB;
            pieceBB[friendly] ^= newKing | newRook;

            pieceBB[ROOK] ^= newRook;
            pieceBB[ROOK] |= toBB;

            pieceBB[KING] ^= newKing;
            pieceBB[KING] |= fromBB;

            // deal with occupied and empty squares
            U64 allChanges = fromToBB | newKing | newRook;
            occupiedSquares ^= allChanges;
            emptySquares ^= allChanges;
        } else {
            // normal move

            // if it's a promotion, turn the to square back to a pawn
            if (flag == PROMOTION) {
                pieceBB[PAWN] |= toBB;
                if (promo == KNIGHTPROMO) {
                    pieceBB[KNIGHT] &= toBBx;
                } else if (promo == QUEENPROMO) {
                    pieceBB[QUEEN] &= toBBx;
                } else if (promo == BISHOPPROMO) {
                    pieceBB[BISHOP] &= toBBx;
                } else if (promo == ROOKPROMO) {
                    pieceBB[ROOK] &= toBBx;
                }
            }

            if (toType == EMPTY) {
                /* quiet move */

                // Considering from colour: F and T squares will be toggled - use XOR
                pieceBB[friendly] ^= fromToBB;

                // Considering to colour: No changes

                // Considering from type: F and T squares will be toggled like with from colour - use XOR
                pieceBB[fromType] ^= fromToBB;

                // Considering to type: No change as it's move is to an empty square

                // Considering empty and occupied squares: both will be toggled
                emptySquares ^= fromToBB;
                occupiedSquares ^= fromToBB;
            } else {
                /* capture */

                // Considering from colour: F and T squares will be toggled - use XOR
                pieceBB[friendly] ^= fromToBB;

                // Considering to colour: To square is set
                pieceBB[enemy] |= toBB;

                // When considering F and T types you need to consider the case when the F and T types are the same
                // First we toggle the F type, then set the to type
                pieceBB[fromType] ^= fromToBB;
                pieceBB[toType] |= toBB;

                // The F square will be toggled for the occupied and empty boards
                emptySquares ^= fromBB;
                occupiedSquares ^= fromBB;
            }
        }
    }
    void setSquare(short type, short side, short sq) {
        U64 square = C64(1) << sq;
        pieceBB[side] |= square;
        pieceBB[type] |= square;
        emptySquares ^= square;
        occupiedSquares ^= square;
    }

    /* getters */
    Move getLastMove() {
        return moveHistory.back();
    }

    /* other stuff */
    void printBoardPrettily() {
        U64 board;
        string mailbox[64];
        for (int i = 0; i < 64; i++) {
            mailbox[i] = " ";
        }

        cout << "_-~-_ Prettily printed board _-~-_\n";

        for (short int piece = PAWN; piece <= KING; piece++) {
            board = pieceBB[piece];
            string pieceStr = getPieceString(piece);

            while (board) {
                int index = popIntLSB(board);
                assert(mailbox[index] == " ");
                mailbox[index] = pieceStr;
            }
        }

        for (int i = 0; i < 8; i++) {
            int A = int('A');
            cout << 8 - i << "  || ";
            for (int j = 0; j < 8; j++) {
                cout << mailbox[i * 8 + j] << " | ";
            }
            cout << "\n";
        }
        cout << "   ----------------------------------\n";
        cout << "      A   B   C   D   E   F   G   H \n";
    }
    void printEverything() {
        cout << "Occupied squares: \n";
        printBitboard(occupiedSquares, nPiece);

        cout << "Empty squares: \n";
        printBitboard(emptySquares, nPiece);

        cout << "White squares: \n";
        printBitboard(pieceBB[nWhite], nPiece);

        cout << "Black squares: \n";
        printBitboard(pieceBB[nBlack], nPiece);

        cout << "Pawn squares: \n";
        printBitboard(pieceBB[PAWN], nPiece);

        cout << "Bishop squares: \n";
        printBitboard(pieceBB[BISHOP], nPiece);

        cout << "Knight squares: \n";
        printBitboard(pieceBB[KNIGHT], nPiece);

        cout << "Rook squares: \n";
        printBitboard(pieceBB[ROOK], nPiece);

        cout << "Queen squares: \n";
        printBitboard(pieceBB[QUEEN], nPiece);

        cout << "King squares: \n";
        printBitboard(pieceBB[KING], nPiece);
    }
    void printOccupied() {
        printBitboard(occupiedSquares, nPiece);
    }
};

#endif /* !FILE_TYPES_SEEN */