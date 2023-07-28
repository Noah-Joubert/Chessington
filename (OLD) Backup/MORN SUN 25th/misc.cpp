//
// Created by Noah Joubert on 2021-04-23.
//
#include <string>
#include "types.h"

#ifndef FILE_misc_SEEN
#define FILE_misc_SEEN

/* other stuff */
string getPieceString(short int key) {
    switch (key) {
        case nWhite:
            return "white ";
        case nBlack:
            return "black ";
        case PAWN:
            return "P";
        case KNIGHT:
            return "K";
        case BISHOP:
            return "B";
        case QUEEN:
            return "Q";
        case KING:
            return "+";
        case ROOK:
            return "R";
        case EMPTY:
            return " ";
        case nPiece:
            return "occu  ";
    }
}
void printBitboard(U64 BB,unsigned short  key) {
    string key_str = getPieceString(key);
    cout << "-----------------\n";
    for (int sq = A8; sq <= H1; sq++) {
        U64 singleBit = C64(1) << sq; // shift a 1 'square' bits right
        if (BB & singleBit) {
            cout << key_str << " | ";
        } else {
            cout << "      " << " | ";
        }

        if ((sq + 1) % 8 == 0) {
            cout << "\n";
        }
    }
}
void printBoardPrettily() {
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
            mailbox[index] = pieceStr;
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
void printMovesPrettily(MoveList moves) {
    int num_moves = moves.size();
    string allStrings[6][num_moves + 1];

    allStrings[0][0] = "From type   | ";
    allStrings[1][0] = "To type     | ";
    allStrings[2][0] = "From square | ";
    allStrings[3][0] = "To square   | ";
    allStrings[4][0] = "Promo piece | ";
    allStrings[5][0] = "Move code   | ";

    int i = 0;
    while (!moves.empty()) {
        i++;
        Move move = moves.back();
        moves.pop_back();

        unsigned int start = move & fromMask;
        unsigned int end = (move & toMask) >> 6;
        unsigned short promoPiece = (move & promoMask) >> 12;
        unsigned short moveCode = (move & flagMask) >> 14;
        unsigned short fromType = (move & fromTypeMask) >> 16;
        unsigned short toType = (move & toTypeMask) >> 19;

        allStrings[0][i] = getPieceString(fromType);
        allStrings[1][i] = getPieceString(toType);

        allStrings[2][i] = SquareStrings[start];
        allStrings[3][i] = SquareStrings[end];

        allStrings[4][i] = to_string(promoPiece);
        allStrings[5][i] = to_string(moveCode);
    }

    cout << "_-~-_ Pretty move printing _-~-_ \n";
    for (int i = 0; i < 6; i++) {
        cout << allStrings[i][0];
        for (int j = 1; j < num_moves + 1; j++) {
            cout << setw(3) << allStrings[i][j] << " | ";
        }
        cout << "\n";
    }
}
void printEverything() {
    cout << "Occupied squares: \n";
    printBitboard(occupiedSquares, nPiece);

    cout << "Empty squares: \n";
    printBitboard(emptySquares, nPiece);

    cout << "White squares: \n";
    printBitboard(pieceBB[nWhite], nPiece);

    cout << "Black squares: \n";
    printBitboard(pieceBB[nBlack], nPiece);

    cout << "Pawn squares: \n";
    printBitboard(pieceBB[PAWN], nPiece);

    cout << "Bishop squares: \n";
    printBitboard(pieceBB[BISHOP], nPiece);

    cout << "Knight squares: \n";
    printBitboard(pieceBB[KNIGHT], nPiece);

    cout << "Rook squares: \n";
    printBitboard(pieceBB[ROOK], nPiece);

    cout << "Queen squares: \n";
    printBitboard(pieceBB[QUEEN], nPiece);

    cout << "King squares: \n";
    printBitboard(pieceBB[KING], nPiece);
}
void printOccupied() {
    printBitboard(occupiedSquares, nPiece);
}
void printMoveBitboard(Move move) {
    unsigned int start = move & fromMask;
    unsigned int end = (move & toMask) >> 6;
    unsigned short promoPiece = (move & promoMask) >> 12;
    unsigned short moveCode = (move & flagMask) >> 14;
    unsigned short fromType = (move & fromTypeMask) >> 16;
    unsigned short toType = (move & toTypeMask) >> 19;

    cout << "-- move --\n";
    cout << "start: " << SquareStrings[start] << "\n";
    cout << "end: " << (SquareStrings[end]) << "\n";
    cout << "promoPiece: " << (promoPiece) << "\n";
    cout << "moveCode: " << (moveCode) << "\n";
    cout << "fromType: " << (fromType) << "\n";
    cout << "toType: " << (toType) << "\n";
}

#endif /* !FILE_TYPES_SEEN */