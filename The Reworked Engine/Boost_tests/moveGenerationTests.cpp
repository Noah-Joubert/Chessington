#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include "../src/perft.cpp"

BOOST_AUTO_TEST_SUITE(perftTests)
    Board board;

    BOOST_AUTO_TEST_CASE(initialPosition) {
        board.readFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        BOOST_CHECK(4865609	 == perft(0, board, 5, false));
    }
    BOOST_AUTO_TEST_CASE(kiwiPete) {
        board.readFEN("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ");
        BOOST_CHECK(4085603 == perft(0, board, 4, false));
    }
    BOOST_AUTO_TEST_CASE(posn3) {
        board.readFEN("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ");
        BOOST_CHECK(178633661 == perft(0, board, 7, false));
    }
    BOOST_AUTO_TEST_CASE(posn4) {
        board.readFEN("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1");
        BOOST_CHECK(15833292 == perft(0, board, 5, false));
    }
    BOOST_AUTO_TEST_CASE(posn5) {
        board.readFEN("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8  ");
        BOOST_CHECK(89941194 == perft(0, board, 5, false));
    }
    BOOST_AUTO_TEST_CASE(posn6) {
        board.readFEN("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10 ");
        BOOST_CHECK(3894594 == perft(0, board, 4, false));
    }
BOOST_AUTO_TEST_SUITE_END();