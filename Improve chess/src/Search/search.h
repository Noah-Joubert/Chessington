//
// Created by Noah Joubert on 2021-06-17.
//

#ifndef IMPROVE_CHESS_SEARCH_H
#define IMPROVE_CHESS_SEARCH_H


#define STALEMATE 1000

long int searchDepth, negaNodes;

bool search(SearchState &SuperBoard, Move &bMove, char &twice);

#endif //IMPROVE_CHESS_SEARCH_H
