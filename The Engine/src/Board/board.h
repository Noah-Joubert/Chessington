//
// Created by Noah Joubert on 28/07/2023.
//

#ifndef SEARCH_CPP_BOARD_H
#define SEARCH_CPP_BOARD_H

/*
 * This is the board class. It runs the game of chess, in a bare-bones form.
 * It only contains methods and attributes that are essential for chess to run.
 * To run chess: use innerMakeMove, and innerUnMakeMove!
 * ~ That is about it.
 * */
class Board {
protected:
    // TODO CORE - THESE WON'T BE STRIPPED BACK

    /* These are the bitboards */
    U64 pieceBB[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0}; // main BB, one redundant
    U64 occupiedSquares, emptySquares; // emptySquares and it's compliment

    U64 blockersNS, blockersEW, blockersNE, blockersNW; // the set of pieces that are preventing a check (they can't move)
    U64 attackMap; // map of attacked squares, used to stop king from moving into check
    U64 checkingRay; // holds the acceptable squares for a move to land

    /* History stuff. This is needed for undoing moves */
    MoveList activeMoveList; // stores the active moves
    MoveList quietMoveList; // stores the quiet moves
    MoveList combinedMoveList; // stores both types of moves
    MoveList moveHistory; // stores past moves
    vector<EnPassantRights> enPassantHistory; // stores past en-passant rights
    vector<CRights> CastleRightsHistory; // stores previous castle rights

    /* Board status stuff */
    // two variables keep track of the current side, as one is needed to index an array.
    Side currentSide = WHITE, otherSide = BLACK;
    short friendly = nWhite, enemy = nBlack;
    CRights CastleRights = 15; // the castling rights
    EnPassantRights enPassantRights = 0;
    bool inCheck; // holds whether board in check
    short moveNumber = 1; // how many moves have been made


    /* Move gen stuff (completely self-contained) */
    /* How does move generation work?
     * Call genAllMoves() with the type of move being generate (ie. all/ quiet moves)
     * You don't need to worry about how it generates these :)
     */
    void genBlockers();
    void genAttackMap();
    void genKingMoves();
    void genRookMoves();
    void genBishopMoves();
    void genQueenMoves();
    void genKnightMoves();
    void genLegal(short pieceType, short TYPE);
    U64 genPieceLegal(U64 piece, short pieceType);
    U64 genBishopLegal(U64 piece);
    U64 genKnightLegal(U64 piece);
    U64 genRookLegal(U64 piece);
    void genPawnMoves(short TYPE);
    void genCastlingNew();
    void genAllMoves(short TYPE);
    bool checkKingCheck(short SIDE);
    short getPieceAt(U64 &sq);
    U64 getSquareAttackers(U64 sq, short SIDE);
    U64 getRay(U64 &from, U64 &to);


    /* make move */
    void innerMakeMove(Move move);
    void innerUnMakeMove();
    inline void decodeMove(Move move, short &from, short &to, short &promo, short &flag, short &fromType, short &toType);
    inline void clearEnPassRights();
    inline void doEnPass(short &fromType, short &toType, short &from, short &to);
    void updateCastleRights();
    inline void undoCastleRights();
    inline void undoEnPassRights();
    inline void doCastle(short &fromType, short &toType, short &from, short &to);
    inline void doQuiet(short &fromType, short &from, short &to);
    inline void doCapture(short &fromType, short &toType, short &from, short &to);
    inline void setSquare(short &type, Side &side, short &sq);


    /* getters - a lot of these can only be used once moves have been generated */
    bool canCastle(short SIDE);
    void getSmallestAttacker(short attackedSquare, short &smallestAttackerSquare, short &cheapestPiece);
    U64 getEmptySquares();


    /* More getters */
    U64 getPieces(short PIECE, short SIDE);
    short innerGetCurrentSide();
    short innerGetOtherSide();
    bool innerGivesCheck(Move &move);


    /* other stuff */
    short FENToPieceCode(char c);
    void readFENInner(string FEN);

    /* Setters */
    void innerSwitchSide();
};

#endif //SEARCH_CPP_BOARD_H
