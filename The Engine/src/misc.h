//
// Created by Noah Joubert on 2021-04-23.
//
#include <string>
#include "types.h"

#ifdef _WIN32
#include <direct.h>
    #define getcwd _getcwd // stupid MSFT "deprecation" warning
#else
#include <unistd.h>
#endif

#ifndef FILE_misc_SEEN
#define FILE_misc_SEEN

/* other stuff */
string getCWDString() {
    // returns the current working directory address

    int SIZE = 1000;
    char buffer[SIZE];
    char *answer = getcwd(buffer, sizeof(buffer));
    string s_cwd;
    if (answer)
    {
        s_cwd = answer;

        // remove the '/cmake-build-debug' from the end
        int stringLength = s_cwd.size();
        return s_cwd.substr(0, stringLength - 18);
    } else {
        cout << "Error with fetching cwd.";
        return " ";
    }
}
string getPieceString(short int key) {
    switch (key) {
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
        case nPiece:
            return "occu  ";
    }
    return "";
}
void printBitboard(U64 BB, short key) {
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
inline void decodeMove(Move move, short &from, short &to, short &promo, short &flag, short &fromType, short &toType) {
    from = move & fromMask;
    to = (move & toMask) >> 6;
    promo = (move & promoMask) >> 12;
    flag = (move & flagMask) >> 14;
    fromType = (move & fromTypeMask) >> 16;
    toType = (move & toTypeMask) >> 19;
}
bool compareMoves(Move m1, Move m2) {
    short from, to, promo, flag, fromType, toType;
    decodeMove(m1, from, to, promo, flag, fromType, toType);

    short from1;
    decodeMove(m2, from1, to, promo, flag, fromType, toType);

    return from < from1;
}
void printMovesPrettily(MoveList moves) {
    int num_moves = moves.size();
    string allStrings[7][num_moves + 1];

    allStrings[0][0] = "From type   | ";
    allStrings[1][0] = "To type     | ";
    allStrings[2][0] = "From square | ";
    allStrings[3][0] = "To square   | ";
    allStrings[4][0] = "Promo piece | ";
    allStrings[5][0] = "Move code   | ";
    allStrings[6][0] = "Move ID     | ";

    int i = 0;
    while (!moves.empty()) {
        i++;
        Move move = moves.back();
        moves.pop_back();

        int start = move & fromMask;
        int end = (move & toMask) >> 6;
        short promoPiece = (move & promoMask) >> 12;
        short moveCode = (move & flagMask) >> 14;
        short fromType = (move & fromTypeMask) >> 16;
        short toType = (move & toTypeMask) >> 19;

        allStrings[0][i] = getPieceString(fromType);
        allStrings[1][i] = getPieceString(toType);

        allStrings[2][i] = SquareStrings[start];
        allStrings[3][i] = SquareStrings[end];

        allStrings[4][i] = to_string(promoPiece);
        allStrings[5][i] = to_string(moveCode);
        allStrings[6][i] = to_string(num_moves - (i));
    }

    cout << "_-~-_ Pretty move printing _-~-_ \n";
    for (int i = 0; i < 7; i++) {
        cout << allStrings[i][0];
        for (int j = 1; j < num_moves + 1; j++) {
            cout << setw(7) << allStrings[i][j] << " | ";
        }
        cout << "\n";
    }
}
void printMoveCounts(MoveList moves, vector<int> counts) {
    cout << "Moves       | ";
    for (int i = 0; i < moves.size(); i++) {
        cout << setw(7) << (moves.size() - i - 1) << " | ";
    }
    cout << "\nCounts      | ";
    for (int count: counts) {
        cout << setw(7) << count << " | ";
    }
    cout << "\n";
}
void printMoveBitboard(Move move) {
    short start, end, promoPiece, moveCode, fromType, toType;
    decodeMove(move, start, end, promoPiece, moveCode, fromType, toType);

    cout << "Start: " << SquareStrings[start] << " <> ";
    cout << "End: " << (SquareStrings[end]) << " <> ";
    cout << "PromoPiece: " << getPieceString(promoPiece) << " <> ";
    cout << "MoveCode: " << (moveCode) << " <> ";
    cout << "FromType: " << getPieceString(fromType) << " <> ";
    cout << "ToType: " << getPieceString(toType) << "\n";
}
string moveToFEN(Move move, string twice) {
    short fromSq, toSq, promo, flag, fromPc, toPc;
    decodeMove(move, fromSq, toSq, promo, flag, fromPc, toPc);

    string moveString = "";

    if (flag == CASTLING) {
        // castle queenside
        if (toSq == A1 || toSq == A8) {
            moveString += "O-O-O";
        } else {
            moveString += "O-O";
        }
    } else if (flag == PROMOTION) {
        string promoStr;
        if (promo == QUEENPROMO) {
            promoStr = "Q";
        } else if (promo == BISHOPPROMO) {
            promoStr = "B";
        } else if (promo == KNIGHTPROMO) {
            promoStr = "N";
        } else if (promo == ROOKPROMO) {
            promoStr = "R";
        }

        // get the start file of the pawn
        char startFile = char(int('a') + fromSq % 8);

        // see if this was a capture
        if (toPc != EMPTY) {
            // capture
            moveString.push_back(startFile);
            moveString += "x" + SquareStrings[toSq] + "=" + promoStr;
        } else {
            moveString += SquareStrings[toSq] + "=" + promoStr;
        }
    } else if (flag == ENPASSANT) {
        char startFile = char(int('a') + fromSq % 8);

        moveString.push_back(startFile);
        moveString += "x" + SquareStrings[toSq];
    } else {
        // see if it is a pawn move
        if (fromPc == PAWN) {
            char startFile = char(int('a') + fromSq % 8);

            if (toPc != EMPTY) {
                // capture
                moveString.push_back(startFile);
                moveString += "x" + SquareStrings[toSq];
            } else {
                // not capture
                moveString += SquareStrings[toSq];
            }
        } else {
            // get the piece string
            string pcString;
            if (fromPc == KNIGHT) {
                pcString = "N";
            } else if (fromPc == BISHOP) {
                pcString = "B";
            } else if (fromPc == ROOK) {
                pcString = "R";
            } else if (fromPc == KING) {
                pcString = "K";
            } else {
                pcString = "Q";
            }

            moveString += pcString;

            if (twice != "-") {
                moveString += twice;
            }

            // see if it was a capture
            if (toPc != EMPTY) {
                // capture
                moveString += "x" + SquareStrings[toSq];
            } else {
                moveString += SquareStrings[toSq];
            }
        }
    }

    return moveString;
}
void splitString(string str, string *arr, string brk, int num) {
    for (int i = 0; i < num; i ++) {
        int space = str.find(brk);
        arr[i] = str.substr(0, space);
        str = str.substr(space + 1, -1);
    }
}

#endif /* !FILE_TYPES_SEEN */