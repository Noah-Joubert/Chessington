//
// Created by Noah Joubert on 2021-04-22.
//

#ifndef FILE_SearchControllerCPP_SEEN
#define FILE_SearchControllerCPP_SEEN

#include "Evaluation/evaluation.cpp"
#include "Transposition Table/zobrist.h"
#include "search.cpp"
#include "SearchController.h"

/* Constructor */
SearchController::SearchController(SearchParameters &searchParamsIn): nativeTT(searchParamsIn) {
    /* init the TT */
    joinTT(&nativeTT);

    searchParameters = &searchParamsIn;

    /* Init the Zobrist Keys and FEN */
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

/* Make Move */
void SearchController::makeMove(Move move) {
    //TODO add in the god damn evaluation stuff
    //TODO Zobrist needs to changed based on enpassant right and castle rights. And in SetSquare. And in SwitchPlayer. ANd updated enpassant rights in inner move on fdouble pawn push.

    /* Update the zobrist hash. We do this first so the side doesn't switch */
    prevZobristStates.emplace_back(zobristState);
    prevMaterialEvaluations.emplace_back(materialEvaluation);
    updateAfterMove(move);
    updateEnPassZobrist();
    updateCastlingZobrist();
    updateSideZobrist();

    /* ACTUALLY MAKE THE MOVE */
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

    /* reload previous material evaluation */
    materialEvaluation = prevMaterialEvaluations.back();
    prevMaterialEvaluations.pop_back();
}
MoveList SearchController::getMoveList() {
    // returns the regular move list

    genAllMoves();

    return combinedMoveList;
}
void SearchController::readFEN(string FEN) {
    readFENInner(FEN);

    /* Clear the TT table, if initialised */
    TT->clear();

    /* clear the search's history */
    prevZobristStates.clear();
    moveHistory.clear();
    enPassantHistory.clear();
    CastleRightsHistory.clear();
    moveNumber = 1;

    /* Recalculate the Zobrist hash */
    calculateAndSetZobristHash();

    /* Recalculate the material balance */
    materialEvaluation = biasedMaterial();
}
void SearchController::switchSide() {
    innerSwitchSide();
}

/* Zobrist and Board Tracking stuff */
bool SearchController::validateZobrist() {
    // this is used to see whether the incrementally calculated zobrist hash is equal to the one calculated from scratch

    Zobrist created = calculateZobristHash();
    return zobristState == created;
}
Zobrist SearchController::calculateZobristHash() {
    // recalculates the zobrist hash from scratch

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
void SearchController::calculateAndSetZobristHash() {
    // recalculates the zobrist hash from scratch, and sets it as the current hash
    zobristState = calculateZobristHash();
    prevZobristStates.emplace_back(zobristState);
}
void SearchController::updateAfterMove(Move move) {
    /* Updates the zobrist hash after a move has been made. We also update material evaluation
     * Switching the side is done separately, as we may want to switch sides without making a move
     * */
    short from, to, promo, flag, fromType, toType;
    decodeMove(move, from, to, promo, flag, fromType, toType);

    int PSTIncriment = 0; // the change to the PST values. this needs to be multiplied by 1/ -1 depending on the currentSide

    if (flag == ENPASSANT) {
        short enPassSquare; // the square of the pawn we are taking
        if (currentSide == WHITE) {
            enPassSquare = to + 8;
        } else {
            enPassSquare = to - 8;
        }

        // update material balance
        materialEvaluation += PieceWorths[PAWN + currentSide * 6];
        PSTIncriment += PST[PAWN][currentSide][to];
        PSTIncriment -= PST[PAWN][currentSide][from];
        PSTIncriment -= PST[PAWN][otherSide][enPassSquare] * -1;


        // xor in and out our pawn
        zobristXOR(PAWN, from, currentSide);
        zobristXOR(PAWN, to, currentSide);

        // xor out the taken pawn
        zobristXOR(PAWN, enPassSquare, otherSide); // xor out the taken pawn
    } else if (flag == CASTLING) {
        // xor out the castle and king from their old positions
        zobristXOR(KING, from, currentSide); // xor out the king
        zobristXOR(ROOK, to, currentSide); // xor out the rook

        // xor them into their new positions
        short newRook, newKing;
        U64 rook = toBB(to); // all 0's from the to square a 1
        getCastleSquares(rook, newRook, newKing);
        zobristXOR(KING, newKing, currentSide); // xor out the king
        zobristXOR(ROOK, newRook, currentSide); // xor out the rook

        // update material balance
        PSTIncriment += PST[ROOK][currentSide][newRook];
        PSTIncriment += PST[KING][currentSide][newKing];
        PSTIncriment -= PST[KING][currentSide][from];
        PSTIncriment -= PST[ROOK][currentSide][to];
    } else {
        zobristXOR(fromType, from, currentSide); // xor out the start square, start player

        PSTIncriment -= PST[fromType][currentSide][from];  // update material balance

        // xor out the end square, if occupied
        if (toType != EMPTY) {
            materialEvaluation += PieceWorths[toType + currentSide * 6]; // update material balance
            PSTIncriment -= PST[toType][otherSide][to] * -1;

            zobristXOR(toType, to, otherSide); // xor out the end square, end player
        }

        if (flag == PROMOTION) {
            fromType = getPromoPiece(promo);

            materialEvaluation -= PieceWorths[PAWN + currentSide * 6]; // update material balance
            materialEvaluation += PieceWorths[fromType + currentSide * 6];
        }

        PSTIncriment += PST[fromType][currentSide][to];

        // xor out the end square, start player.
        zobristXOR(fromType, to, currentSide);
    }

    materialEvaluation += PSTIncriment * (currentSide == WHITE ? 1 : -1); // add the PST increment to the material balance
}
void SearchController::updateEnPassZobrist() {
    // xor out en-pass rights
    for (int i = 0; i < 8; i++) {
        if (enPassantRights & toBB(i)) {
            zobristState ^= enPassKeys[i];
            break;
        }
    }
}
void SearchController::updateCastlingZobrist() {
    // xor out the castle rights into the zobrist hash
    for (int i = 0; i < 4; i++) {
        if (CastleRights & toBB(i)) {zobristState ^= castleKeys[i];}
    }
}
inline void SearchController::zobristXOR(short piece, short square, Side side) {
    // XOR's a piece at a certain square
    zobristState ^= pieceKeys[piece + 6 * side][square];
}
void SearchController::updateSideZobrist() {
    /* Updates the zobrist hash after switching sides */
    zobristState ^= sideKey[currentSide];
    zobristState ^= sideKey[otherSide];
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