//
// Created by Noah Joubert on 28/07/2023.
//

#ifndef SEARCH_CPP_SEARCH_H
#define SEARCH_CPP_SEARCH_H

#define EXACT_EVAL 1
#define LOWER_EVAL 2
#define UPPER_EVAL 0

#define INFIN 100000
#define MATE 10000

struct SearchParameters {
    //TODO look into the setting of these
    struct TTParameters{
        int TTSizeMb = 0; // size of the TT in mb  (0 BY DEFAULT)
        int replaceDepth = 1; // the extra depth needed to overwrite a node (must be at least 1)
        int replaceAge = 7; // the extra age needed to overwrite a node (must be at least 1)

        int minTTInsertDepth = -50; // the minimum depth at which a search will be inputted into the transposition table
        bool useTT = true; // whether we are using the TT in regular search
        bool useTTInQSearch = false; // whether we are using the TT in the quiescence search
    };

    TTParameters ttParameters;

    float minSearchTime = 0.1; // the minimum time of a search in the iterative deepening framework
    int startingDepth = 1; // the depth at which iterative deepening is started

    int stalemateEvaluation = -1000; // the evaluation of a stalemate position

    bool useSEE = true; // whether we use SEE/ delta pruning
    bool useDelta = true;
    int deltaMargin = 200; // the margin used for delta pruning

    bool useQuiescence = true; // whether we use a quiescence search
};

struct SearchStats {
    int totalNodesSearched = 0;
    int totalQuiescenceSearched = 0;
    int totalNonCaptureQSearched = 0; // count how many quiescence nodes aren't captures (ie. checks/ promos)

    void clear(){
        totalNodesSearched = 0;
        totalQuiescenceSearched = 0;
        totalNonCaptureQSearched = 0;
    }
};

inline short getEvaluationType(int eval, int alpha, int beta) {
    if (eval <= alpha) {
        return UPPER_EVAL;
    } else if (eval >= beta) {
        return LOWER_EVAL;
    } else {
        return EXACT_EVAL;
    }
}
inline string evaltypeToString(short evalType) {
    if (evalType == EXACT_EVAL) {return "Exact";}
    else if (evalType == LOWER_EVAL) {return "Lower";}
    else {return "Upper";}
}

#endif //SEARCH_CPP_SEARCH_H
