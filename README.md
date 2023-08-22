# My Chess Engine
This is a Bitboard based chess engine. It's coded from scratch solely by me, using C++. It works pretty well. It can comfortable beat me (which isn't neccessarily saying much). It also comes with a python program which runs tournaments between different versions in order to see if they are improving. Right now it is just a dodgy command line interface but this could change in the future.

# Code structure
My implementation of chess is entirely contained within the Board class. So theoretically, if one wanted to build up an engine on the back of my chess code, you could use this code as a foundation. The SearchController class is a wrapper around the Board class which includes various fields and methods needed to for a chess engine which aren't necessarily needed for a pure implenetation of chess (eg. Zobrist hash, transposition table ect.)
