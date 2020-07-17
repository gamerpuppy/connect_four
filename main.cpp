#include <iostream>
#include <chrono>
#include "connect-four.h"


void playFixedDepth(std::string cfef, bool playerIsP1, int depth) {
    Board board = Board::fromCfef(cfef);
    std::cout << board.visualRep();
    while (true) {
        if (board.turnCount() > 41) {
            return;
        }
        if (board.isP1Turn() == playerIsP1) {
            int move;
            std::cin >> move;
            if (board.doesMoveWin(move)) {
                std::cout << "PLAYER WINS\n";

                std::cout << board.forMove(move).visualRep();
                return;
            }
            board = board.forMove(move);


        } else {
            auto start = std::chrono::high_resolution_clock::now();
            auto eval = board.evaluate(depth);
            auto end = std::chrono::high_resolution_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

            std::cout << eval.move << " value: " << eval.score << " millis: " << millis << '\n';

            if (board.doesMoveWin(eval.move)) {
                std::cout << "CPU WINS\n";
                std::cout << board.forMove(eval.move).visualRep();
                return;
            }
            board = board.forMove(eval.move);
            std::cout << board.visualRep();
            std::cout << board.toCfef()<< std::endl;
        }
    }
}

void play(std::string cfef, bool playerIsP1, uint64_t msAllowed) {
    Board board = Board::fromCfef(cfef);
    std::cout << board.visualRep();
    while (true) {
        if (board.turnCount() > 41) {
            return;
        }
        if (board.isP1Turn() == playerIsP1) {
            int move;
            std::cin >> move;
            if (board.doesMoveWin(move)) {
                std::cout << "PLAYER WINS\n";

                std::cout << board.forMove(move).visualRep();
                return;
            }
            board = board.forMove(move);


        } else {
            auto start = std::chrono::high_resolution_clock::now();
            auto eval = evaluateDynamicDepth(board, msAllowed);
            auto end = std::chrono::high_resolution_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

            std::cout << eval.move << " value: " << eval.score << " depth: " << eval.depth << " millis: " << millis << '\n';

            if (board.doesMoveWin(eval.move)) {
                std::cout << "CPU WINS\n";
                std::cout << board.forMove(eval.move).visualRep();
                return;
            }
            board = board.forMove(eval.move);
            std::cout << board.visualRep();
            std::cout << board.toCfef()<< std::endl;
        }
    }
}

int main(int argc, const char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: playerGoesFirst(y/n) startingCfef-optional" << std::endl;
    }
    bool playerIsFirst = std::string(argv[1]) == "y";
    std::string cfef = argc == 3 ? argv[1] : "//////";

    play(cfef, playerIsFirst, 3000);
//    test();
    return 0;
}
