//
// Created by Noah Joubert on 23/07/2023.
//
#include "../types.h"
#include "bitboards.cpp"
#include "../misc.cpp"
#include "../Search/Evaluation/evaluation.h"

/*
 * This is the board class. It runs the game of chess, in a bare-bones form.
 * It only contains methods and attributes that are essential for chess to run.
 * To run chess: use innerMakeMove, and innerUnMakeMove!
 * ~ That is about it.
 * */
class Board {
protected:
    // TODO CORE - THESE WON'T BE STRIPPED BACK

    /* These are the bitboards */
    U64 pieceBB[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // main BB, one redundant
    U64 occupiedSquares, emptySquares; // emptySquares and it's compliment

    U64 blockersNS, blockersEW, blockersNE, blockersNW; // the set of pieces that are preventing a check (they can't move)
    U64 attackMap; // map of attacked squares, used to stop king from moving into check
    U64 checkingRay; // holds the acceptable squares for a move to land

    /* History stuff. This is needed for undoing moves */
    MoveList activeMoveList; // stores the active moves
    MoveList quietMoveList; // stores the quiet moves
    MoveList combinedMoveList; // stores both types of moves
    MoveList moveHistory; // stores past moves
    vector<EnPassantRights> enPassantHistory; // stores past en-passant rights
    vector<CRights> CastleRightsHistory; // stores previous castle rights

    /* Board status stuff */
    // two variables keep track of the current side, as one is needed to index an array.
    short currentSide = WHITE, otherSide = BLACK, friendly = nWhite, enemy = nBlack;
    CRights CastleRights = 15; // the castling rights
    EnPassantRights enPassantRights = 0;
    bool inCheck; // holds whether board in check
    short moveNumber = 1; // how many moves have been made


    /* Move gen stuff (completely self-contained) */
    /* How does move generation work?
     * Call genAllMoves() with the type of move being generate (ie. all/ quiet moves)
     * You don't need to worry about how it generates these :)
     */
    void genBlockers();
    void genAttackMap();
    void genKingMoves();
    void genRookMoves();
    void genBishopMoves();
    void genQueenMoves();
    void genKnightMoves();
    void genLegal(short pieceType, short TYPE);
    U64 genPieceLegal(U64 piece, short pieceType);
    U64 genBishopLegal(U64 piece);
    U64 genKnightLegal(U64 piece);
    U64 genRookLegal(U64 piece);
    void genPawnMoves(short TYPE);
    void genCastlingNew();
    void genAllMoves(short TYPE);
    bool checkKingCheck(short SIDE);
    short getPieceAt(U64 &sq);
    U64 getSquareAttackers(U64 sq, short SIDE);
    U64 getRay(U64 &from, U64 &to);


    /* make move */
    /* How does make move work?
     * You call the innerMakeMove function, or the innerUnMakeMove function with a legal move.
     */
    void innerMakeMove(Move move) {
        // inner function used to make a move

        /* How does this work?
         * 1. Add the move to the moveHistory. Increment moveNumber.
         * 2. Clear the enPassantRights
         * 3. Execute the move, considering what type of move it is (eg. pawn push, capture, promotion)
         * 4. Update castling rights
         * 5. Switch side
         */

        moveHistory.emplace_back(move);
        moveNumber ++;
        clearEnPassRights();

        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);

        if (flag == ENPASSANT) {
            /* en passant */
            doEnPass(fromType, toType, from, to);
        } else if (flag == CASTLING) {
            /* castle */
            doCastle(fromType, toType, from, to);
        } else {
            // normal move
            if (toType == EMPTY) {
                /* quiet move */
                doQuiet(fromType, from, to);

                // check for double pawn push and update en-passant rights
                if ((fromType == PAWN) && ((from - to) % 16 == 0)) {
                    short file = to % 8;
                    enPassantRights ^= toBB(file);
                }
            } else {
                /* capture */
                doCapture(fromType, toType, from, to);
            }

            // if it was a promotion, set the destination square to the promotion piece
            if (flag == PROMOTION) {
                // remove the pawn
                setSquare(fromType, currentSide, to);

                // add in the promoted piece
                short promoPiece = getPromoPiece(promo);
                setSquare(promoPiece, currentSide, to);
            }
        }

        updateCastleRights();
        innerSwitchSide(); // switch the side
    }
    void innerUnMakeMove() {
        // inner function used to un-make a move

        /* How does this work
         * 1. Pop the last move from history
         * 2. Reload the previous enPassantRights & castling rights
         * 3. Switch the side back
         * 4. Run the move in reverse
         * */

        Move move = moveHistory.back(); // get the last move played
        moveHistory.pop_back();
        moveNumber--; // decrease the move number

        undoEnPassRights();
        undoCastleRights();

        innerSwitchSide(); // switch the side

        // decode the move
        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);

        if (flag == ENPASSANT) {
            /* en passant */
            doEnPass(fromType, toType, from, to);

        } else if (flag == CASTLING) {
            /* castle */
            doCastle(fromType, toType, from, to);

        } else {
            // normal move

            // if it's a promotion, turn the to square back to a pawn
            if (flag == PROMOTION) {
                // take out the promoted piece

                short promoPiece = getPromoPiece(promo);

                setSquare(fromType, currentSide, to);
                setSquare(promoPiece, currentSide, to);

            }
            if (toType == EMPTY) {
                /* quiet move */
                doQuiet(fromType, from, to);
            } else {
                /* capture */
                doCapture(fromType, toType, from, to);
            }
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
    inline void clearEnPassRights() {
        // update zobrist key by removing any previous en-passants.
        enPassantHistory.emplace_back(enPassantRights);
        enPassantRights = 0;
    }
    inline void doEnPass(short &fromType, short &toType, short &from, short &to) {
        short enPassSquare;

        if (currentSide == WHITE) {
            enPassSquare = to + 8;
        } else {
            enPassSquare = to - 8;
        }

        // deal with the moving pawn
        setSquare(fromType, currentSide, from);
        setSquare(fromType, currentSide, to);

        // deal with the taken pawn
        setSquare(toType, otherSide, enPassSquare);
    }
    void updateCastleRights() {
        CastleRightsHistory.emplace_back(CastleRights);

        U64 w = (pieceBB[KING] | pieceBB[ROOK]) & pieceBB[nWhite];
        if ((CastleRights & 1) && (w & CastleMasks[WHITE][0]) != CastleMasks[WHITE][0]) {
            CastleRights ^= 1;
        }
        if ((CastleRights & 2) && (w & CastleMasks[WHITE][1]) != CastleMasks[WHITE][1]) {
            CastleRights ^= 2;
        }

        U64 b = (pieceBB[KING] | pieceBB[ROOK]) & pieceBB[nBlack];
        if ((CastleRights & 4) && (b & CastleMasks[BLACK][0]) != CastleMasks[BLACK][0]) {
            CastleRights ^= 4;
        }
        if ((CastleRights & 8) && (b & CastleMasks[BLACK][1]) != CastleMasks[BLACK][1]) {
            CastleRights ^= 8;
        }
    }
    inline void undoCastleRights() {
        CastleRights = CastleRightsHistory.back();
        CastleRightsHistory.pop_back();
    }
    inline void undoEnPassRights() {
        enPassantRights = enPassantHistory.back();
        enPassantHistory.pop_back();
    }
    inline void doCastle(short &fromType, short &toType, short &from, short &to) {
        // first reset the castle and king squares
        setSquare(fromType, currentSide, from);
        setSquare(toType, currentSide, to);

        short newRook, newKing;
        U64 rook = toBB(to); // all 0's from the to square a 1
        getCastleSquares(rook, newRook, newKing, currentSide);

        // now set the new castle and king squares
        setSquare(fromType, currentSide, newKing);
        setSquare(toType, currentSide, newRook);
    }
    inline void doQuiet(short &fromType, short &from, short &to) {
        setSquare(fromType, currentSide, from);
        setSquare(fromType, currentSide, to);
    }
    inline void doCapture(short &fromType, short &toType, short &from, short &to) {
        // reset the from and too square
        setSquare(fromType, currentSide, from);
        setSquare(toType, otherSide, to);

        // set the too square
        setSquare(fromType, currentSide, to);
    }
    inline void setSquare(short &type, short &side, short &sq) {
        // is used to set/reset a square as only xor's are used
        short sideKey = ((side == WHITE) ? nWhite : nBlack);

        // make the change
        U64 square = toBB(sq);
        pieceBB[sideKey] ^= square;
        pieceBB[type] ^= square;
        emptySquares ^= square;
        occupiedSquares ^= square;
    }


    /* getters - a lot of these can only be used once moves have been generated */
    bool canCastle(short SIDE) {
        if (SIDE == WHITE) {
            return CastleRights & 1 || CastleRights & 2;
        } else if (SIDE == BLACK) {
            return CastleRights & 4 || CastleRights && 8;
        }
    }
    static inline short getPSTValue(short &square, short &type, short &side) {
        short sign;
        if (side == WHITE) {sign = 1;}
        else if (side == BLACK) {sign = -1;}

        switch (type) {
            case PAWN:
                return sign * PST_Pawn[side][square];
            case KNIGHT:
                return sign * PST_Knight[side][square];
            case BISHOP:
                return sign * PST_Bishop[side][square];
        }

        return 0;
    }
    int getSmallestAttacker(int square) {
        U64 attackers = getSquareAttackers(toBB(square), otherSide);
        attackers &= pieceBB[friendly]; // make sure only friendly piece

        if (attackers == 0) return -1;

        // loop through all pieces in order of worth
        for (int piece = PAWN; piece <= KING; piece++) {
            // get the pieces that are also attackers
            U64 pieceAttackers = pieceBB[piece] & attackers;

            if (pieceAttackers == 0) continue;

            return bitScanForward(pieceAttackers);
        }
    }
    U64 getEmptySquares() {
        return emptySquares;
    }


    /* More getters */
    U64 getPieces(short PIECE, short SIDE) {
        short sideKey;
        if (SIDE == WHITE) {
            sideKey = nWhite;
        } else {
            sideKey = nBlack;
        }

        return pieceBB[PIECE] & pieceBB[sideKey];
    }
    short innerGetCurrentSide() {
        return currentSide;
    }
    short innerGetOtherSide() {
        return otherSide;
    }
    bool innerGivesCheck(Move &move);


    /* other stuff */
    short FENToPieceCode(char c) {
        c = tolower(c);
        switch (c) {
            case 'p':
                return PAWN;
            case 'n':
                return KNIGHT;
            case 'b':
                return BISHOP;
            case 'r':
                return ROOK;
            case 'q':
                return QUEEN;
            case 'k':
                return KING;
        }
        return EMPTY;
    }
    void readFENInner(string FEN) {
        // This is the inner function called to set the board to a given FEN code.

        /* reset the board */
        for (int i = 0; i < 9; i++) pieceBB[i] = 0;
        occupiedSquares = 0;
        emptySquares = ~0;

        /* split the FEN into it's components */
        string sections[6];
        splitString(FEN, sections, " ", 6);

        /* split up the position into ranks */
        string ranks[8];
        splitString(sections[0], ranks, "/", 8);
        for (int r = 0; r < 8; r ++) {
            // reset the 8 different piece bitboards ~ 6 pawns + 2 colours
            string rank = ranks[r]; // get the rank
            // loop through the characters in the string
            int index = 0;
            for (char c: rank) {
                if (isdigit(c)) {
                    index += int(c) - 48;
                } else {
                    // add the piece to the board
                    short piece = FENToPieceCode(c);
                    short square = r * 8 + index;
                    short side = isupper(c) ? WHITE : BLACK;
                    setSquare(piece, side, square);
                    index ++;
                }
            }
        }

        /* see who's turn it is */
        short side = sections[1] == "w" ? WHITE : BLACK;
        if (side != currentSide) innerSwitchSide();

        /* now deal with castling rights */
        // start of by setting castling rights to 0
        CastleRights = 0;
        for (char c: sections[2]) {
            switch (c) {
                case 'K':
                    // white can castle king side
                    CastleRights |= 2;
                case 'Q':
                    CastleRights |= 1;
                case 'k':
                    CastleRights |= 8;
                case 'q':
                    CastleRights |= 4;
            }
        }

        /* now deal with en-passant rights */
        string enpassSquare = sections[3];
        U64 enPassantRights = 0;
        if (enpassSquare != "-") {
            enPassantRights = int(enpassSquare[0]) - int('a');
            enPassantRights = toBB(enPassantRights);
        }
        enPassantRights = enPassantRights;



        moveHistory.clear();
        enPassantHistory.clear();
        CastleRightsHistory.clear();
    }

    /* Setters */
    void innerSwitchSide() {
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
};
