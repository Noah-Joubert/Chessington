//
// Created by Noah Joubert on 2021-04-22.
//

#ifndef FILE_pos_SEEN
#define FILE_pos_SEEN

#include "../types.h"
#include "../Board/bitboards.cpp"
#include "../misc.cpp"
#include "../evaluation.h"
#include "../Board/board.cpp"

class SearchState: public Board{
private:
    /* evaluation variables */
    int material = 0; // holds the value of the material on the board,
    int pst = 0; // the piece square table value
    int castle = 0; // holds the bonus each side has recieved for castling
    bool hasCastled[2] = {false, false}; // worth's for castling for each side

    /* evaluation stuff */
    int openingEval(U64 wAttacks, U64 bAttacks);
    int endgameEval();
    int materialEval();
    int lazyEval();


public:
    /* Evaluation */
    int evaluate(int alpha, int beta);
    int relativeLazy();

    /* Game control */
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
    void makeMove(Move &move) {
        PST_history.emplace_back(pst);

        short from, to, promo, flag, fromType, toType;
        decodeMove(move, from, to, promo, flag, fromType, toType);
        prevStates.emplace_back(getZobristState());
        clearEnPassRights();

        if (flag == ENPASSANT) {
            // update the material balance
            material -= PieceWorths[getCurrentSide() * 6 + PAWN];

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

            /* en passant */
            doEnPass(fromType, toType, from, to);
        } else if (flag == CASTLING) {
            // update king PST
            short newRook, newKing;
            U64 rook = toBB(to);
            getCastleSquares(rook, newRook, newKing, currentSide);
            pst += getPSTValue(newKing, fromType, currentSide);
            pst -= getPSTValue(from, fromType, currentSide);

            // add on bonus for castling
            hasCastled[currentSide] = true;

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
                    zobristState ^= enPassKeys[file];
                }
            } else {
                /* capture */
                doCapture(fromType, toType, from, to);

                material -= PieceWorths[currentSide * 6 + toType]; // update the material balance

                // minus of pst for the taken piece
                pst -= getPSTValue(to, toType, otherSide);
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
            }
        }
        zobristState = prevStates.back();
        prevStates.pop_back();
        pst = PST_history.back();
        PST_history.pop_back();
    }

    /* What does this do? Fuck knows mate google it. */
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

    /* Output */
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

#endif /* !FILE_TYPES_SEEN */