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

/*
 * This is the SearchState class. It inherits the Board class, which implements chess.
 * It contains the extra things that Board doesn't eg. evaluation metrics and
 * */
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
public:
    /* Evaluation */
    int evaluate(int alpha, int beta);
    int relativeLazy();

    /* Game control */
    // TODO Fix init
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

    /* Output */
    Zobrist getZobristState() {
        return zobristState;
    }
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