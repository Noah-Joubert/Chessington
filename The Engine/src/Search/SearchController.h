//
// Created by Noah Joubert on 28/07/2023.
//

#include "../Board/board.cpp"
#include "Transposition Table/zobrist.h"
#include "Transposition Table/TT.cpp"

#ifndef SEARCH_CPP_SEARCHCONTROLLER_H
#define SEARCH_CPP_SEARCHCONTROLLER_H


/*
 * This is the SearchState class. It inherits the Board class, which implements chess.
 * It contains the extra things that Board doesn't eg. evaluation and zobrist hashing.
 * */
class SearchController: public Board {
private:
    // TODO NON CORE - WILL BE STRIPPED
    /* evaluation variables */
    int materialEvaluation = 0; // holds the value of the material on the board,

    /* Zobrist */
    Zobrist zobristState; // current zobrist hash
    vector<Zobrist> prevZobristStates; // stores previous zobrist hashes
    vector<int> prevMaterialEvaluations; // stores previous material evaluations
    void updateAfterMove(Move move);
    void updateSideZobrist();
    inline void zobristXOR(short piece, short square, Side side);

    /* Search parameters */
    SearchParameters searchParameters;

    /* Search Statistics */
    SearchStats searchStats;

    /* Transposition table */
    TranspositionTable TT;

public:
    //TODO NON CORE - WILL BE STRIPPED

    /* What does this do? Fuck knows mate google it. */
    // TODO sort this
    int SEE(int square);

public:
    // TODO CORE STUFF - THIS IS SAFE FROM BEING STRIPPED BACK

    /* Evaluation */
    int biasedMaterial();
    int evaluate();

    /* Constructor */
    SearchController(SearchParameters searchParamsIn);

    void makeMove(Move &move);
    void unMakeMove();
    MoveList getMoveList();
    MoveList getQMoveList();
    void readFEN(string FEN);
    void switchSide();

    /* Getters */
    int getMoveNumber();
    bool getInCheck();
    bool inCheckMate();
    inline bool checkThreefold();
    bool inStalemate();
    bool givesCheck(Move &move);
    short getCurrentSide();
    short getOtherSide();
    int getMaterialEvaluation() {return materialEvaluation;}

    /* Zobrist stuff */
    bool validateZobrist();
    Zobrist getZobristState();
    Zobrist calculateZobristHash();
    void calculateAndSetZobristHash();
    void updateEnPassZobrist();
    void updateCastlingZobrist();

    void printBoardPrettily();

    /* Search Stuff */
    int negaMax(int alpha, int beta, int depth, Move &bestMove);
    int quiescence(int alpha, int beta, int depth);
    bool search(Move &bestMove, string &FENFlag , bool DEBUG_MODE);
};

#endif //SEARCH_CPP_SEARCHCONTROLLER_H
