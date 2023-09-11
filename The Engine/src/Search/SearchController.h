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
    SearchParameters *searchParameters;

    /* Search Statistics */
    SearchStats searchStats; // we store internal stats and flush them on to global stats when required
    SearchStats *globalStats; // store a reference to the global SearchStats

    /* Transposition table */
    TranspositionTable *TT; // the TT is accessed through a pointer, so we can link to an external one as required
    TranspositionTable nativeTT; // we store a native TT

public:
    // TODO CORE STUFF - THIS IS SAFE FROM BEING STRIPPED BACK

    /* Evaluation */
    int SEE(short square);
    int SEEMove(Move m);
    int biasedMaterial();
    int evaluate();
    int relativeLazy();

    /* Constructor */
    SearchController(SearchParameters &searchParamsIn);

    void makeMove(Move move);
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
    MoveList getMoveHistory() {return moveHistory;}

    /* Linking to Global data stores */
    void joinTT(TranspositionTable *TTIn) {TT = TTIn;}
    void joinSearchStats(SearchStats &stats) {globalStats = &stats;}
    void joinSearchParams(SearchParameters &params) {searchParameters = &params;}
    TranspositionTable* getTT() {return TT;}
    SearchParameters* getSearchParameters() {return searchParameters;}
    SearchStats getStats() {return searchStats;}
    void clearStats() {
        searchStats.clear();
    }
    void flushStats() {
        // flushes the locally stored stats to the global store
        globalStats->add(searchStats);
    }

    /* Zobrist stuff */
    bool validateZobrist();
    Zobrist getZobristState();
    Zobrist calculateZobristHash();
    void calculateAndSetZobristHash();
    void updateEnPassZobrist();
    void updateCastlingZobrist();

    void printBoardPrettily();

    /* Search Stuff */
    void extractPV(MoveList &moves);
    int quiescence(int alpha, int beta, int depth);
    int negaMax(int alpha, int beta, int depth, Move &bestMove);
    int firstPly(MoveList moves, int depth, Move &bestMove);
};

#endif //SEARCH_CPP_SEARCHCONTROLLER_H
