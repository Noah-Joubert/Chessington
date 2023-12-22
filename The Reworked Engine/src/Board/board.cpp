#ifndef SEARCH_CPP_NEW_BOARD_CPP
#define SEARCH_CPP_NEW_BOARD_CPP

#include "board.h"
#include "movegen.cpp"
#include "makemove.cpp"
#include <iomanip>

Pieces FENToPieceCode(char c) {
    c = tolower(c);
    switch (c) {
        case 'p':
            return PAWN;
        case 'n':
            return KNIGHT;
        case 'b':
            return BISHOP;
        case 'r':
            return ROOK;
        case 'q':
            return QUEEN;
        case 'k':
            return KING;
    }
    return EMPTY;
}
string getPieceString(Pieces piece) {
    switch (piece) {
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
    }
    return "";
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

        allStrings[0][i] = getPieceString(static_cast<Pieces>(fromType));
        allStrings[1][i] = getPieceString(static_cast<Pieces>(toType));

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
            cout << std::setw(7) << allStrings[i][j] << " | ";
        }
        cout << "\n";
    }
}
void printBoard(Bitboards bitboards, Side currentSide) {
    cout << "_-~-_ Rights _-~-_\n";
    cout << "EnPassantRights: " << bitboards.enPassantRights << "\n";
    cout << "CastlingRights: " << bitboards.castleRights << "\n";
    cout << "Current side: " << (currentSide == WHITE ? "White" : "Black") << "\n";

    cout << "_-~-_ Prettily printed board _-~-_\n";
    string mailbox[64];
    for (auto & i : mailbox) {
        i = " ";
    }
    for (Pieces piece: {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING}) {
        U64 board = bitboards.getPieceBB(piece);
        string pieceStr = getPieceString(piece);

        while (board) {
            short index = popIntLSB(board);
            assert(mailbox[index] == " ");

            // if the piece is black, make the string lower case
            // TODO: tidy this up
            if (bitboards.getSideBB(BLACK) & toBB(index)) {
                string subPieceString = pieceStr;
                for (int i = 0; i < pieceStr.size(); i++) {
                    if (isalpha(pieceStr[i])) {
                        subPieceString[i] = tolower(pieceStr[i]);
                    }
                }
                mailbox[index] = subPieceString;
            } else {
                mailbox[index] = pieceStr;
            }
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
void Board::debugPrint() {
    printBoard(this->bitboards, this->currentSide);
    printMovesPrettily(this->genMoves());
}

vector<string> splitString(string str, string splitChar) {
    vector<string> split;
    while (size(str) > 0) {
        unsigned long space = str.find(splitChar);
        if (space == string::npos) {
            split.emplace_back(str);
            break;
        }
        split.emplace_back(str.substr(0, space));
        str = str.substr(space + 1, -1);
    }
    return split;
}
void Board::readFEN(string FEN) {
    /* Reset bitboards */
    this->bitboards = Bitboards();

    /* Parse the FEN string*/
    vector<string> splitFEN = splitString(FEN, " ");

    /* Go through each rank */
    vector<string> ranks = splitString(splitFEN[0], "/");
    short rank = 0;
    for (const string& rankString: ranks) {
        short index = 0; // how far through the rank we are (1-8)
        for (char c: rankString) {
            if (isdigit(c)) {
                // set the index
                index += int(c) - 48;
            } else {
                // add the piece to the board
                Pieces piece = FENToPieceCode(c);

                Side side = isupper(c) ? WHITE : BLACK;
                this->bitboards.setSquare(piece, side, rank * 8 + index);
                index ++;
            }
        }
        rank ++;
    }

    /* Who's turn is it? */
    this->currentSide = splitFEN[1] == "w" ? WHITE : BLACK;
    this->otherSide = splitFEN[1] == "b" ? WHITE : BLACK;

    /* Castling Rights */
    this->bitboards.castleRights = 0;
    for (char c: splitFEN[2]) {
        switch (c) {
            case 'K':
                // white can castle king side
                this->bitboards.castleRights |= 2;
            case 'Q':
                this->bitboards.castleRights |= 1;
            case 'k':
                this->bitboards.castleRights |= 8;
            case 'q':
                this->bitboards.castleRights |= 4;
        }
    }

    /* En-passant rights */
    this->bitboards.enPassantRights = 0;
    string enpassSquare = splitFEN[3];
    if (enpassSquare != "-") {
        this->bitboards.enPassantRights = toBB(short(enpassSquare[0]) - short('a'));
    }
}

Board::Board() {
    this->moveHistory.reserve(30);
    this->CastleRightsHistory.reserve(30);
    this->enPassantHistory.reserve(30);

    Masks::genMasks();

    this->readFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
}

#endif //SEARCH_CPP_NEW_BOARD_H