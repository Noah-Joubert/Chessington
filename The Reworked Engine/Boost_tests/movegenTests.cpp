#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "../src/Board/board.cpp" // TODO: import this in a nicer way
#include "../src/perft.cpp" // TODO: import this in a nicer way

BOOST_AUTO_TEST_SUITE(genKingBlockers)
    Board board;
    BOOST_AUTO_TEST_CASE(oneNorthSouthBlocker) {
        board.readFEN("8/8/8/3r4/8/8/3P4/3K4 b - - 0 1");

        Bitboards bb = board.getBitboards();
        MoveGeneration::MoveGenBitboards blockers{};
        MoveGeneration::genKingBlockers(bb, blockers);

        int nsBlockers = count(blockers.NS);

        BOOST_CHECK(nsBlockers == 1);
    }
    BOOST_AUTO_TEST_CASE(twoDiagonalAndTwoHorizontal) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK1P1q/8 w - - 0 1");

        Bitboards bb = board.getBitboards();
        MoveGeneration::MoveGenBitboards blockers{};
        MoveGeneration::genKingBlockers(bb, blockers);

        int totalBlockers = count(blockers.EW) + count(blockers.NE) + count(blockers.NW);

        BOOST_CHECK(totalBlockers == 4);
    }
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(genAttackMap)
    Board board;
    BOOST_AUTO_TEST_CASE(noCheck) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK1P1q/8 w - - 0 1");
        Side side = board.getCurrentSide();
        Bitboards bb = board.getBitboards();
        U64 king = bb.getPieceBB(KING) & bb.getSideBB(side);

        U64 attackMap = MoveGeneration::genAttackMap(side, bb);
        bool inCheck = king & attackMap;

        BOOST_CHECK(inCheck == false);
    }
    BOOST_AUTO_TEST_CASE(oneCheck) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK3q/8 w - - 0 1");
        Side side = board.getCurrentSide();
        Bitboards bb = board.getBitboards();
        U64 king = bb.getPieceBB(KING) & bb.getSideBB(side);

        U64 attackMap = MoveGeneration::genAttackMap(side, bb);
        bool inCheck = king & attackMap;

        BOOST_CHECK(inCheck == true);
    }
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(genLegalMoves)
    Board board;
    bool testFEN(string FEN, int expectedNumberOfMoves) {
        board.readFEN(FEN);
        MoveList moves = board.genMoves();
        board.setMoveList(moves);
        board.debugPrint();

        return moves.size() == expectedNumberOfMoves;
    }
    BOOST_AUTO_TEST_CASE(initialPosn) {
        BOOST_CHECK(testFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", 20));
    }
    BOOST_AUTO_TEST_CASE(KiwiPete) {
        BOOST_CHECK(testFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 48));
    }
    BOOST_AUTO_TEST_CASE(posn3) {
        BOOST_CHECK(testFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 14));
    }
    BOOST_AUTO_TEST_CASE(posn4) {
        BOOST_CHECK(testFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 6));
    }
    BOOST_AUTO_TEST_CASE(posn5) {
        BOOST_CHECK(testFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ", 44));
    }
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(makeMove)
    Board board;
    BOOST_AUTO_TEST_CASE(basicMoves) {
        board.readFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        board.setMoveList(board.genMoves());
        board.makeMove(board.getMoveList()[0]);
        board.debugPrint();
        board.unmakeMove();
        board.debugPrint();
    }

    BOOST_AUTO_TEST_SUITE_END();

