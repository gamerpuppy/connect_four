#include <iostream>
#include <chrono>
#include "connect-four.h"


void play(std::string cfef, bool playerIsP1, int depth) {
    Board board = Board::fromCfef(cfef);
    std::cout << board.visualRep();

    while (true) {
        if (board.turn > 41) {
            return;
        }
        if (board.isP1Turn() == playerIsP1) {
            int move;
            std::cin >> move;
            board = board.forMove(move);
            auto eval = evaluate(board, 1);

            if (eval.value != TIE) {
                return;
            }

        } else {
            auto start = std::chrono::high_resolution_clock::now();
            auto eval = evaluate(board, depth);
            auto end = std::chrono::high_resolution_clock::now();
            auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

            int bestMove = eval.moves[0];
            std::cout << bestMove << " value: " << eval.value << " millis: " << millis << '\n';
            board = board.forMove(bestMove);
            std::cout << board.visualRep();
            std::cout << board.toCfef()<< std::endl;

            if (eval.value != TIE && eval.moves.size() == 1) {
                return;
            }
        }
    }
}


int main() {
//    play("//////", true, 9);
test();
    return 0;
}