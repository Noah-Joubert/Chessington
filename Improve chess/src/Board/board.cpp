//
// Created by Noah Joubert on 23/07/2023.
//

#include "../types.h"
#include "bitboards.cpp"
#include "../misc.cpp"
#include "../evaluation.h"

class Board {
protected:
    short moveNumber = 1; // how many moves have been made

    U64 pieceBB[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // one extra redundant board is kept
    U64 occupiedSquares, emptySquares; // emptySquares and it's compliment
    MoveList activeMoveList; // stores the active moves
    MoveList quietMoveList; // stores the quiet moves
    MoveList combinedMoveList; // stores both types of moves
    U64 blockersNS, blockersEW, blockersNE, blockersNW; // blockers for all the ray directions
    U64 attackMap;

    MoveList moveHistory; // stores past moves
    vector<EnPassantRights> enPassantHistory; // stores past en-passant rights
    vector<CRights> CastleRightsHistory; // stores previous castle rights
    vector<Zobrist> prevStates; // stores previous zobrist hashes
    vector<short> PST_history; // stores previous piece square table values

    short currentSide = WHITE, otherSide = BLACK, friendly = nWhite, enemy = nBlack;
    CRights CastleRights = 15;
    EnPassantRights enPassantRights = 0;
    Zobrist zobristState;

    U64 checkingRay; // holds the acceptable squares for a move to land
    bool inCheck; // holds whether board in check

    /* move gen stuff */
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
    MoveList getCaptures() {
        return activeMoveList;
    }
    MoveList getQuiets() {
        return quietMoveList;
    }

    /* make move */
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
        if (enPassantRights) {
            short file = pop8BitIntLSB(enPassantRights);
            zobristState ^= enPassKeys[file];
        }

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
            zobristState ^= castleKeys[0];
        }
        if ((CastleRights & 2) && (w & CastleMasks[WHITE][1]) != CastleMasks[WHITE][1]) {
            CastleRights ^= 2;
            zobristState ^= castleKeys[1];
        }

        U64 b = (pieceBB[KING] | pieceBB[ROOK]) & pieceBB[nBlack];
        if ((CastleRights & 4) && (b & CastleMasks[BLACK][0]) != CastleMasks[BLACK][0]) {
            CastleRights ^= 4;
            zobristState ^= castleKeys[2];
        }
        if ((CastleRights & 8) && (b & CastleMasks[BLACK][1]) != CastleMasks[BLACK][1]) {
            CastleRights ^= 8;
            zobristState ^= castleKeys[3];
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

        // update zobrist
        zobristState ^= pieceKeys[type + 6 * side][sq];
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

    /* Possibly need to be reworked */
    U64 getEmptySquares() {
        return emptySquares;
    }

    /* Init stuff */
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

    /* Zobrist stuff */
    bool validateZobrist() {
        Zobrist created = createZobrist();
        return zobristState == created;
    }
    Zobrist createZobrist() {
        Zobrist key = 0;

        // first do pieces
        for (short pc = PAWN; pc <= KING; pc ++) {
            vector<short> w_pc = toArray(pieceBB[pc] & pieceBB[nWhite]);
            vector<short> b_pc = toArray(pieceBB[pc] & pieceBB[nBlack]);

            for (short sq: w_pc) {key ^= pieceKeys[pc][sq];}
            for (short sq: b_pc) {key ^= pieceKeys[pc + 6][sq];}
        }

        // now do current player
        key ^= sideKey[currentSide];

        // next do en-pass rights
        for (int i = 0; i < 8; i++) {
            if (enPassantRights & toBB(i)) {
                key ^= enPassKeys[i];
                break;
            }
        }

        // next do castling rights
        for (int i = 0; i < 4; i++) {
            if (CastleRights & toBB(i)) {key ^= castleKeys[i];}
        }

        return key;
    }
    void setZobrist() {
        zobristState = createZobrist();
        prevStates.emplace_back(zobristState);
    }

public:
    /* Getters */
    MoveList getMoveList() {
        // returns the regular move list

        genAllMoves(ALL_MOVES);

        return combinedMoveList;
    }
    MoveList getQMoveList() {
        // returns the quiescence move list

        genAllMoves(QUIESENCE_MOVES);

        return combinedMoveList;
    }
    short getCurrentSide() {
        return currentSide;
    }
    short getOtherSide() {
        return otherSide;
    }
    Zobrist getZobristState() {
        return zobristState;
    }
    int getMoveNumber() {
        return moveNumber;
    }

    bool getInCheck() {
        return inCheck;
    }
    bool inCheckMate() {
        // you are in inCheckMate if you are in check without any moves
        return inCheck && combinedMoveList.empty();
    }
    U64 getPieces(short PIECE, short SIDE) {
        short sideKey;
        if (SIDE == WHITE) {
            sideKey = nWhite;
        } else {
            sideKey = nBlack;
        }

        return pieceBB[PIECE] & pieceBB[sideKey];
    }
    inline bool checkThreefold() {
        /* check for checkThreefold repetition */

        // loop through every other zobrist state from the current one
        // the current state is only added to the prevStates vector once a move is made
        if (moveNumber < 7) return false;

        int reps = 1;
        for (int i = moveNumber - 3; i >= 0; i -= 2) {
            if (zobristState == prevStates[i]) reps ++;
        }

        return reps >= 3;
    }
    bool inStalemate() {
        // you are in inStalemate if there are no moves and you're not in check
        return (!inCheck) && combinedMoveList.empty();
    }
    bool givesCheck(Move &move);


    /* Game function */
    void switchSide() {
        // xor out the old player
        zobristState ^= sideKey[currentSide];

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

        // xor in the new player
        zobristState ^= sideKey[currentSide];
    }
    void innerMakeMove() {

    }
    void innerUnMakeMove() {

    }


    /* Setters */
    void readFEN(string FEN) {
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
        if (side != currentSide) switchSide();

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

        /* the other sections aren't needed for now */
        setZobrist();
        prevStates.clear();
        moveHistory.clear();
        enPassantHistory.clear();
        CastleRightsHistory.clear();
    }
};
