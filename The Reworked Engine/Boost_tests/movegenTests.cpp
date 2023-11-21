#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "../src/Board/board.cpp" // TODO: import this in a nicer way

BOOST_AUTO_TEST_SUITE(genKingBlockers)
    Board board;
    BOOST_AUTO_TEST_CASE(OneNorthSouthBlocker) {
        board.readFEN("8/8/8/3r4/8/8/3P4/3K4 b - - 0 1");

        Bitboards bb = board.getBitboards();
        MoveGeneration::MoveGenBitboards blockers{};
        MoveGeneration::genKingBlockers(bb, blockers);

//        printBitboard(blockers.NS);

        int nsBlockers = count(blockers.NS);

        BOOST_CHECK(nsBlockers == 1);
    }
    BOOST_AUTO_TEST_CASE(TwoDiagonalAndTwoHorizontal) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK1P1q/8 w - - 0 1");

        Bitboards bb = board.getBitboards();
        MoveGeneration::MoveGenBitboards blockers{};
        MoveGeneration::genKingBlockers(bb, blockers);

//        printBitboard(blockers.EW);
//        printBitboard(blockers.NE);
//        printBitboard(blockers.NW);

        int totalBlockers = count(blockers.EW) + count(blockers.NE) + count(blockers.NW);

        BOOST_CHECK(totalBlockers == 4);
    }
BOOST_AUTO_TEST_SUITE_END();

BOOST_AUTO_TEST_SUITE(genAttackMap)
    Board board;
    BOOST_AUTO_TEST_CASE(NoCheck) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK1P1q/8 w - - 0 1");
        Side side = board.getCurrentSide();
        Bitboards bb = board.getBitboards();
        U64 king = bb.getPieceBB(KING) & bb.getSideBB(side);

        U64 attackMap = MoveGeneration::genAttackMap(side, bb);
        bool inCheck = king & attackMap;

//        printBitboard(attackMap);

        BOOST_CHECK(inCheck == false);
    }
    BOOST_AUTO_TEST_CASE(OneCheck) {
        board.readFEN("8/8/8/q5q1/8/2P1P3/q1PK3q/8 w - - 0 1");
        Side side = board.getCurrentSide();
        Bitboards bb = board.getBitboards();
        U64 king = bb.getPieceBB(KING) & bb.getSideBB(side);

        U64 attackMap = MoveGeneration::genAttackMap(side, bb);
        bool inCheck = king & attackMap;

//        printBitboard(attackMap);

        BOOST_CHECK(inCheck == true);
    }
BOOST_AUTO_TEST_SUITE_END();
