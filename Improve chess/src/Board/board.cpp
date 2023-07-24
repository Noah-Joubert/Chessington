//
// Created by Noah Joubert on 23/07/2023.
//

#include "../types.h"
#include "bitboards.cpp"
#include "../misc.cpp"
#include "../evaluation.h"

struct Board {
    short moveNumber = 1; // how many moves have been made

    U64 pieceBB[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // one extra redundant board is kept
    U64 occupiedSquares, emptySquares; // emptySquares and it's compliment
    U64 blockersNS, blockersEW, blockersNE, blockersNW; // blockers for all the ray directions
    U64 attackMap;

    MoveList activeMoveList; // stores the active moves
    MoveList quietMoveList; // stores the quiet moves
    MoveList combinedMoveList; // stores both types of moves

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

    // a variable which determines which game phase we're in
    const short ENDGAME = 24;
    const short OPENING = 0;
    short PHASE;
    short PHASE_WORTHS[6] = {0, 1, 1, 2, 4, 0};

    // evaluation stuff
    int material = 0; // holds the value of the material on the board,
    int pst = 0; // the piece square table value
    int castle = 0; // holds the bonus each side has recieved for castling
    bool hasCastled[2] = {false, false}; // worth's for castling for each side

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
    MoveList getMoveList() {
        // returns the regular move list

        genAllMoves(ALL_MOVES);

        return combinedMoveList;
    }
    MoveList getCaptures() {
        return activeMoveList;
    }
    MoveList getQuiets() {
        return quietMoveList;
    }
    MoveList getQMoveList() {
        // returns the quiesence move list

        genAllMoves(QUIESENCE_MOVES);

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

        // create the 'empty/occupied squares' BB
        occupiedSquares = (pieceBB[nWhite] | pieceBB[nBlack]);
        emptySquares = ~occupiedSquares;

        setZobrist();

        /* this should be looked into/optimised */
        activeMoveList.reserve(100);
        quietMoveList.reserve(100);
        moveHistory.reserve(100);
        combinedMoveList.reserve(200);
        enPassantHistory.reserve(100);
    }
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

    /* make move */
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
    inline void clearEnPassRights() {
        // update zobrist key by removing any previous en-passants.
        if (enPassantRights) {
            short file = pop8BitIntLSB(enPassantRights);
            zobristState ^= enPassKeys[file];
        }

        enPassantHistory.emplace_back(enPassantRights);
        enPassantRights = 0;
    }
    inline void undoEnPassRights() {
        enPassantRights = enPassantHistory.back();
        enPassantHistory.pop_back();
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
        prevStates.emplace_back(zobristState);
        clearEnPassRights();
        PST_history.emplace_back(pst);

        if (flag == ENPASSANT) {
            /* en passant */
            doEnPass(fromType, toType, from, to);

            // update the material balance
            material -= PieceWorths[currentSide * 6 + PAWN];

            // update piece square tables
            pst -= getPSTValue(from, fromType, currentSide);
            pst += getPSTValue(to, fromType, currentSide);

            short enPassSquare;
            if (currentSide == WHITE) {
                enPassSquare = to + 8;
            } else {
                enPassSquare = to - 8;
            }

            pst -= getPSTValue(enPassSquare, fromType, otherSide);
        } else if (flag == CASTLING) {
            /* castle */
            doCastle(fromType, toType, from, to);

            // update king PST
            short newRook, newKing;
            U64 rook = toBB(to);
            getCastleSquares(rook, newRook, newKing, currentSide);
            pst += getPSTValue(newKing, fromType, currentSide);
            pst -= getPSTValue(from, fromType, currentSide);

            // add on bonus for castling
            hasCastled[currentSide] = true;
        } else {
            // normal move
            if (toType == EMPTY) {
                /* quiet move */
                doQuiet(fromType, from, to);

                // check for double pawn push and update en-passant rights
                if ((fromType == PAWN) && ((from - to) % 16 == 0)) {
                    short file = to % 8;
                    enPassantRights ^= toBB(file);
                    zobristState ^= enPassKeys[file];
                }
            } else {
                /* capture */
                doCapture(fromType, toType, from, to);
                material -= PieceWorths[currentSide * 6 + toType]; // update the material balance

                // minus of pst for the taken piece
                pst -= getPSTValue(to, toType, otherSide);

                // update the game phase
                PHASE += PHASE_WORTHS[toType];
            }

            // update pst worth
            pst -= getPSTValue(from, fromType, currentSide);
            pst += getPSTValue(to, fromType, currentSide);

            // if it was a promotion, set the destination square to the promotion piece
            if (flag == PROMOTION) {
                // remove the pawn
                setSquare(fromType, currentSide, to);

                // add in the promoted piece
                short promoPiece = getPromoPiece(promo);
                setSquare(promoPiece, currentSide, to);

                material -= PieceWorths[currentSide * 6 + promoPiece]; // update the material balance

                // update pst
                pst -= getPSTValue(to, fromType, currentSide);
                pst += getPSTValue(to, promoPiece, currentSide);
            }
        }

        updateCastleRights();
        moveHistory.emplace_back(move); // store the move so it can be undone
        switchSide(); // switch the side
        moveNumber++; // increment the move number
    }
    void unMakeMove() {
        Move move = moveHistory.back(); // get the last move played
        moveHistory.pop_back();

        undoEnPassRights();
        switchSide(); // switch the side

        moveNumber--; // decrease the move number
        undoCastleRights();

        // decode the move
        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);

        if (flag == ENPASSANT) {
            /* en passant */
            doEnPass(fromType, toType, from, to);

            material += PieceWorths[currentSide * 6 + PAWN]; // update the material balance
        } else if (flag == CASTLING) {
            /* castle */
            doCastle(fromType, toType, from, to);

            // remove bonus for un-castling
            hasCastled[currentSide] = false;
        } else {
            // normal move

            // if it's a promotion, turn the to square back to a pawn
            if (flag == PROMOTION) {
                // take out the promoted piece

                short promoPiece = getPromoPiece(promo);

                setSquare(fromType, currentSide, to);
                setSquare(promoPiece, currentSide, to);

                material += PieceWorths[currentSide * 6 + promoPiece]; // update the material balance
            }

            if (toType == EMPTY) {
                /* quiet move */
                doQuiet(fromType, from, to);
            } else {
                /* capture */
                doCapture(fromType, toType, from, to);
                material += PieceWorths[currentSide * 6 + toType]; // update the material balance

                PHASE -= PHASE_WORTHS[toType];
            }
        }
        zobristState = prevStates.back();
        prevStates.pop_back();
        pst = PST_history.back();
        PST_history.pop_back();
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
    void setPhase() {
        PHASE = ENDGAME;
        for (short pc = PAWN; pc < KING; pc++) {
            PHASE -= count(pieceBB[pc]) * PHASE_WORTHS[pc];
        }
    }

    /* evaluation stuff */
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
    int evaluate(int alpha, int beta);
    int openingEval(U64 wAttacks, U64 bAttacks);
    int endgameEval();
    int materialEval();
    int lazyEval();
    int relativeLazy();
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
    int SEE(int square) {
        int value = 0;
        int startSquare = getSmallestAttacker(square);


        // if there are no more pieces return 0
        if (startSquare != -1) {
            U64 from = toBB(startSquare);
            U64 to = toBB(square);

            int fromPiece = getPieceAt(from);
            int toPiece = getPieceAt(to);

            Move m = encodeMove(startSquare, square, 0, 0, fromPiece, toPiece);
            makeMove(m);
            value = PieceWorths[toPiece] - SEE(square);
            unMakeMove();
        }

        return value;
    }

    /* getters - a lot of these can only be used once moves have been generated */
    inline bool checkThreefold() {
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
    bool inCheckMate() {
        // you are in inCheckMate if you are in check without any moves
        return inCheck && combinedMoveList.empty();
    }
    bool getInCheck() {
        return inCheck;
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
    Move getLastMove() {
        return moveHistory.back();
    }
    bool canCastle(short SIDE) {
        if (SIDE == WHITE) {
            return CastleRights & 1 || CastleRights & 2;
        } else if (SIDE == BLACK) {
            return CastleRights & 4 || CastleRights && 8;
        }
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
                if (pieceBB[nBlack] & toBB(index)) {
                    string subPieceString = pieceStr;
                    for (int i = 0; i < pieceStr.size(); i++) {
                        if (isalpha(pieceStr[i])) {
                            subPieceString[i] = tolower(pieceStr[i]);
                        }
                    }
                    mailbox[index] = subPieceString;
                } else {
                    mailbox[index] = pieceStr;
                }
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


};
