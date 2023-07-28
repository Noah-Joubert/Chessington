//
// Created by Noah Joubert on 2021-04-22.
//

#ifndef FILE_pos_SEEN
#define FILE_pos_SEEN

#include "../types.h"
#include "../Board/bitboards.cpp"
#include "../misc.cpp"
#include "Evaluation/evaluation.h"
#include "../Board/board.cpp"
#include "zobrist.h"



/*
 * This is the SearchState class. It inherits the Board class, which implements chess.
 * It contains the extra things that Board doesn't eg. evaluation and zobrist hasing.
 * */
class SearchState: public Board {
private:
    // TODO NON CORE - WILL BE STRIPPED
    /* evaluation variables */
    int material = 0; // holds the value of the material on the board,
    int pst = 0; // the piece square table value
    int castle = 0; // holds the bonus each side has recieved for castling
    bool hasCastled[2] = {false, false}; // worth's for castling for each side

    /* Zobrist */
    Zobrist zobristState;
    vector<Zobrist> prevStates; // stores previous zobrist hashes

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

    long int searchDepth, searchedNodes;

public:
    // TODO NON CORE - WILL BE STRIPPED

    /* What does this do? Fuck knows mate google it. */
    // TODO sort this
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

    int relativeLazy();

    /* Output */
    Zobrist getZobristState() {
        return zobristState;
    }


public:
    // TODO CORE STUFF - THIS IS SAFE FROM BEING STRIPPED BACK

    /* Evaluation */
    int evaluate();

    SearchState() {
        readFEN(initialFEN);

        //TODO this yeah
        /* this should be looked into/optimised */
        activeMoveList.reserve(100);
        quietMoveList.reserve(100);
        moveHistory.reserve(100);
        combinedMoveList.reserve(200);
        enPassantHistory.reserve(100);
    }

    void makeMove(Move &move) {
        //TODO add in the god damn evaluation stuff
        //TODO Zobrist needs to changed based on enpassant right and castle rights. And in SetSquare. And in SwitchPlayer. ANd updated enpassant rights in inner move on fdouble pawn push.

        /* ACUTALLY MAKE THE MOVE */
        innerMakeMove(move);
    }
    void unMakeMove() {
        /* ACTUALLY UNMAKE THE MOVE */
        innerUnMakeMove();
    }
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
    void readFEN(string FEN) {
        readFENInner(FEN);

        /* the other sections aren't needed for now */
        setZobrist();
        prevStates.clear();
    }
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

    /* Getters */
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
    inline bool checkThreefold() {
        /* check for checkThreefold repetition */

        // loop through every other zobrist state from the current one
        // the current state is only added to the prevStates vector once a move is made
        if (moveNumber < 7) return false;

        int reps = 1;

        return reps >= 3;
    }
    bool inStalemate() {
        // you are in inStalemate if there are no moves and you're not in check
        return (!inCheck) && combinedMoveList.empty();
    }
    bool givesCheck(Move &move) {
        return innerGivesCheck(move);
    }
    short getCurrentSide() {
        return currentSide;
    }
    short getOtherSide() {
        return otherSide;
    }

    void printBoardPrettily() {
        // TODO make this nicer
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

    /* Search Stuff */
    int negaMax(int alpha, int beta, int depth, Move &bestMove);
    bool search(Move &bestMove, char &twice);
};

#endif !FILE_pos_SEEN