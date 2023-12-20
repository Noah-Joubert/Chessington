#include "Board/board.cpp"
#include "types.h"
#include "perft.cpp"

int main() {
    Board board;
    board.readFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1");
    debugPerft(board);
}
