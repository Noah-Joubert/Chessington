//
// Created by Noah Joubert on 28/07/2023.
//

#ifndef SEARCH_CPP_SEARCH_H
#define SEARCH_CPP_SEARCH_H

#define EXACT_EVAL 1
#define LOWER_EVAL 2
#define UPPER_EVAL 0

#define INFIN 1000000
#define MATE 100000

struct SearchParameters {
    //TODO look into the setting of these
    struct TTParameters{
        int TTSizeMb = 0; // size of the TT in mb  (0 BY DEFAULT)
        int replaceDepth = 1; // the extra depth needed to overwrite a node
        int replaceAge = 5; // the extra age needed to overwrite a node

        int minTTInsertDepth = 0; // the minimum depth at which a search will be inputted into the transposition table
        bool useTT = true; // whether we are using the TT
    };

    TTParameters ttParameters;

    float minSearchTime = 0.1; // the minimum time of a search in the iterative deepening framework
    int startingDepth = 1; // the depth at which iterative deepening is started

    int stalemateEvaluation = -1000; // the evaluation of a stalemate position

    bool useSEE = false;
    bool useQuiescence = true;
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


#endif //SEARCH_CPP_SEARCH_H
