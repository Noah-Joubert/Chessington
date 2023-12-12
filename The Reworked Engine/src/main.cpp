#include "Board/board.cpp"
#include "types.h"
#include "perft.cpp"

int main() {
    Board board;
    board.readFEN((const string &) "asjfiasf");
    board.debugPrint();
}
