#include "Board/board.cpp"
#include "types.h"

int main() {
    Board board;
    board.readFEN((const string &) "asjfiasf");
    board.debugPrint();
}
