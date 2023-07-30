//
// Created by Noah Joubert on 2021-04-22.
//

#ifndef FILE_SearchControllerCPP_SEEN
#define FILE_SearchControllerCPP_SEEN

#include "Evaluation/evaluation.cpp"
#include "Transposition Table/zobrist.cpp"
#include "search.cpp"
#include "SearchController.h"

/* What does this do? Fuck knows mate google it. */
// TODO sort this
int SearchController::SEE(int square) {
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

/* Constructor */
SearchController::SearchController() {
    initZobristKeys();
    readFEN(initialFEN);

    //TODO this yeah
    /* this should be looked into/optimised */
    activeMoveList.reserve(100);
    quietMoveList.reserve(100);
    moveHistory.reserve(100);
    combinedMoveList.reserve(200);
    enPassantHistory.reserve(100);
}

void SearchController::makeMove(Move &move) {
    //TODO add in the god damn evaluation stuff
    //TODO Zobrist needs to changed based on enpassant right and castle rights. And in SetSquare. And in SwitchPlayer. ANd updated enpassant rights in inner move on fdouble pawn push.

    /* Update the zobrist hash. We do this first so the side doesn't switch */
    prevZobristStates.emplace_back(zobristState);
    updateMoveZobrist(move);
    updateEnPassZobrist();
    updateCastlingZobrist();
    updateSideZobrist();

    /* ACUTALLY MAKE THE MOVE */
    innerMakeMove(move);

    /* These functions are called twice to xor our the existing rights, and xor in the new ones */
    updateEnPassZobrist();
    updateCastlingZobrist();
}
void SearchController::unMakeMove() {
    /* ACTUALLY UNMAKE THE MOVE */
    innerUnMakeMove();

    /* Reload the previous Zobrist hash */
    zobristState = prevZobristStates.back();
    prevZobristStates.pop_back();
}
MoveList SearchController::getMoveList() {
    // returns the regular move list

    genAllMoves(ALL_MOVES);

    return combinedMoveList;
}
MoveList SearchController::getQMoveList() {
    // returns the quiescence move list

    genAllMoves(QUIESENCE_MOVES);

    return combinedMoveList;
}
void SearchController::readFEN(string FEN) {
    readFENInner(FEN);

    /* clear the search's history */
    prevZobristStates.clear();
    moveHistory.clear();
    enPassantHistory.clear();
    CastleRightsHistory.clear();
    moveNumber = 1;

    /* Recalculate the Zobrist hash */
    calculateAndSetZobristHash();
}
void SearchController::switchSide() {
    innerSwitchSide();
}

/* Getters */
int SearchController::getMoveNumber() {
    return moveNumber;
}
bool SearchController::getInCheck() {
    return inCheck;
}
bool SearchController::inCheckMate() {
    // you are in inCheckMate if you are in check without any moves
    return inCheck && combinedMoveList.empty();
}
inline bool SearchController::checkThreefold() {
    /* check for checkThreefold repetition */

    // loop through every other zobrist state from the current one
    // the current state is only added to the prevStates vector once a move is made
    if (moveNumber < 7) return false;

    int reps = 1;
    // loop backwards through the previous states. moveNumber starts at 1, hence we start at moveNumber - 2
    for (int i = moveNumber - 2; i >= 0; i -= 2) {
        if (zobristState == prevZobristStates[i]) reps ++;
    }

    return reps >= 3;
}
bool SearchController::inStalemate() {
    // you are in inStalemate if there are no moves and you're not in check
    return (!inCheck) && combinedMoveList.empty();
}
bool SearchController::givesCheck(Move &move) {
    return innerGivesCheck(move);
}
short SearchController::getCurrentSide() {
    return currentSide;
}
short SearchController::getOtherSide() {
    return otherSide;
}
Zobrist SearchController::getZobristState() {
    return zobristState;
}

void SearchController::printBoardPrettily() {
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

#endif !FILE_SearchControllerCPP_SEEN