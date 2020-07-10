//
// Created by Keegan Millard on 2020-07-08.
//

#include "connect-four.h"


// Bit Manipulation
uint32_t getBitIdx(uint32_t rIdx, uint32_t cIdx) {
    return cIdx * 6u + rIdx;
}

void setBitAtPos(uint64_t &bits, uint32_t rIdx, uint32_t cIdx) {
    bits |= (1lu << getBitIdx(rIdx, cIdx));
}

uint64_t getWithSetBit(uint64_t bits, uint32_t rIdx, uint32_t cIdx) {
    return bits | (1lu << getBitIdx(rIdx, cIdx));
}

bool getBitAtPos(uint64_t bits, uint32_t rIdx, uint32_t cIdx) {
    return bits & (1lu << getBitIdx(rIdx, cIdx));
}

// Board Bit Manipulation

int getOpenRowIdx(uint32_t col) {
    return __builtin_ffs(col | 0b1000000)-2;
}

uint64_t getCol(uint64_t pieces, uint32_t cIdx) {
    return (pieces >> (cIdx * 6)) & 0x3Fu;
}

// Connect Four Checks

bool upDiagHelper(uint64_t pieces, uint32_t offset, uint64_t mask) {
    uint64_t diag = (pieces >> offset) & mask;
    while (diag >= UP_DIAG_4_MASK) {
        if ((diag & UP_DIAG_4_MASK) == UP_DIAG_4_MASK) {
            return true;
        }
        diag >>= 5u;
    }
    return false;
}

bool upDiagConnectsFour(uint64_t pieces, uint32_t rIdx, uint32_t cIdx) {
    int diagIdx = cIdx+rIdx;
    switch(diagIdx) {
        case 3: return upDiagHelper(pieces, 3, UP_DIAG_4_MASK);
        case 4: return upDiagHelper(pieces, 4, UP_DIAG_5_MASK);
        case 5: return upDiagHelper(pieces, 5, UP_DIAG_6_MASK);
        case 6: return upDiagHelper(pieces, 11, UP_DIAG_6_MASK);
        case 7: return upDiagHelper(pieces, 17, UP_DIAG_5_MASK);
        case 8: return upDiagHelper(pieces, 23, UP_DIAG_4_MASK);
        default: return false;
    }
}

bool dnDiagHelper(uint64_t pieces, uint32_t offset, uint64_t mask) {
    uint64_t diag = (pieces >> offset) & mask;
    while (diag >= DN_DIAG_4_MASK) {
        if ((diag & DN_DIAG_4_MASK) == DN_DIAG_4_MASK) {
            return true;
        }
        diag >>= 7u;
    }
    return false;
}

bool dnDiagConnectsFour(uint64_t pieces, uint32_t rIdx, uint32_t cIdx) {
    int diagIdx = (6-cIdx)+rIdx;
    switch(diagIdx) {
        case 3: return dnDiagHelper(pieces, 18, DN_DIAG_4_MASK);
        case 4: return dnDiagHelper(pieces, 12, DN_DIAG_5_MASK);
        case 5: return dnDiagHelper(pieces, 6, DN_DIAG_6_MASK);
        case 6: return dnDiagHelper(pieces, 0, DN_DIAG_6_MASK);
        case 7: return dnDiagHelper(pieces, 1, DN_DIAG_5_MASK);
        case 8: return dnDiagHelper(pieces, 2, DN_DIAG_4_MASK);
        default: return false;
    }
}

bool colConnectsFour(uint64_t pieces, uint32_t cIdx) {
    uint32_t col = getCol(pieces, cIdx);
    while (col >= 0xFu) {
        if ((col & 0xFu) == 0xFu) {
            return true;
        }
        col >>= 1u;
    }
    return false;
}

bool rowConnectsFour(uint64_t pieces, uint32_t rIdx) {
    uint32_t row = (pieces >> rIdx) & ROW_7_MASK;
    while (row >= ROW_4_MASK) {
        if ((row & ROW_4_MASK) == ROW_4_MASK) {
            return true;
        }
        row >>= 6u;
    }
    return false;
}



bool connectedFour(uint64_t pieces, uint32_t rIdx, uint32_t cIdx) {
    return
        colConnectsFour(pieces, cIdx) ||
        rowConnectsFour(pieces, rIdx) ||
        upDiagConnectsFour(pieces, rIdx, cIdx) ||
        dnDiagConnectsFour(pieces, rIdx, cIdx);
}

// evaluation

Evaluation evaluateHelperEndNode(Board b) {
    Evaluation tieEval(TIE, {});

    uint64_t  combinedPieces = b.pieces[0]|b.pieces[1];
    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {

        uint64_t col = getCol(combinedPieces, cIdx);
        int rIdx = getOpenRowIdx(col);
        if (rIdx < 0) {
            continue;
        }

        if (b.isP1Turn()) {
            uint64_t piecesAfterMove = getWithSetBit(b.pieces[0], rIdx, cIdx);
            if (connectedFour(piecesAfterMove, rIdx, cIdx)) {
                return Evaluation(PLAYER_1_WIN, {cIdx});
            } else if (tieEval.moves.empty()){
                tieEval.moves.push_back(cIdx);
            }


        } else {
            uint64_t piecesAfterMove = getWithSetBit(b.pieces[1], rIdx, cIdx);
            if (connectedFour(piecesAfterMove, rIdx, cIdx)) {
                return Evaluation(PLAYER_2_WIN, {cIdx});
            } else if (tieEval.moves.empty()){
                tieEval.moves.push_back(cIdx);
            }
        }
    }

    return tieEval;
}

Evaluation evaluateHelper(Board b, int depth) {
    if (depth == 1) {
        return evaluateHelperEndNode(b);
    }

    Evaluation bestEvaluation(b.isP1Turn() ? PLAYER_2_WIN : PLAYER_1_WIN, {});

    uint64_t  combinedPieces = b.pieces[0]|b.pieces[1];
    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {

        uint64_t col = getCol(combinedPieces, cIdx);
        int rIdx = getOpenRowIdx(col);
        if (rIdx < 0) {
            continue;
        }

        if (b.isP1Turn()) {

            Board boardAfterMove = b.forMove(rIdx, cIdx);
            if (connectedFour(boardAfterMove.pieces[0], rIdx, cIdx)) {
                return Evaluation(PLAYER_1_WIN, {cIdx});
            }


            Evaluation eval = evaluateHelper(boardAfterMove, depth - 1);
            if (eval.value == PLAYER_1_WIN) {
                eval.moves.push_back(cIdx);
                return eval;
            } else if (eval.value > bestEvaluation.value) {
                eval.moves.push_back(cIdx);
                bestEvaluation = eval;
            }

        } else {
            Board boardAfterMove = b.forMove(rIdx, cIdx);
            if (connectedFour(boardAfterMove.pieces[1], rIdx, cIdx)) {
                return Evaluation(PLAYER_2_WIN, {cIdx});
            }

            Evaluation eval = evaluateHelper(b.forMove(rIdx, cIdx), depth - 1);
            if (eval.value == PLAYER_2_WIN) {
                eval.moves.push_back(cIdx);
                return eval;
            } else if (eval.value < bestEvaluation.value) {
                eval.moves.push_back(cIdx);
                bestEvaluation = eval;
            }

        }

    }

    if (bestEvaluation.moves.empty()) {
        bestEvaluation.value = TIE;
    }
    return bestEvaluation;
}

Evaluation evaluate(Board b, int depth) {
    Evaluation evaluation = evaluateHelper(b, depth);
    std::reverse(evaluation.moves.begin(), evaluation.moves.end());
    return evaluation;
}

// Methods

Board::Board(uint32_t turn, const std::array<uint64_t, 2> &piecesCols) : turn(turn), pieces(piecesCols) {}

Board Board::forMove(uint32_t rIdx, uint32_t cIdx) const {
    if (isP1Turn()) {
        return Board(turn+1, {getWithSetBit(pieces[0], rIdx, cIdx), pieces[1]});
    } else {
        return Board(turn+1, {pieces[0], getWithSetBit(pieces[1], rIdx, cIdx)});
    }
}

Board Board::forMove(uint32_t cIdx) const {
    int rIdx = getOpenRowIdx(getCol(pieces[0]|pieces[1], cIdx));
    return forMove(rIdx, cIdx);
}


Board Board::fromCfef(const std::string &cfef) {
    uint32_t turn{0};
    std::array<uint64_t, 2> pieces{0,0};
    uint32_t cIdx = 0;
    std::istringstream ss(cfef);
    std::string token;

    while(std::getline(ss, token, '/')) {
        int rIdx = 5;
        for (char c : token) {
            int player = c == PLAYER_1 ? 0 : 1;
            setBitAtPos(pieces[player], rIdx, cIdx);
            turn++;
            rIdx--;
        }
        cIdx++;
    }

    return Board(turn, pieces);
}

std::string Board::toCfef() const {
    std::string cfefOut;
    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {
        for (uint32_t rIdx = 5; rIdx >= 0; rIdx--) {

            if (getBitAtPos(pieces[0], rIdx, cIdx)) {
                cfefOut += PLAYER_1;
            } else if (getBitAtPos(pieces[1], rIdx, cIdx)) {
                cfefOut += PLAYER_2;
            } else {
                break;
            }

        }
        if (cIdx != 6) {
            cfefOut += '/';
        }
    }
    return cfefOut;
}

std::string Board::visualRep() const {
    std::string ret;
    for (uint32_t rIdx = 0; rIdx < 6; rIdx++) {
        for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {

            char out;
            if (getBitAtPos(pieces[0], rIdx, cIdx)) {
                out = PLAYER_1;
            } else if (getBitAtPos(pieces[1], rIdx, cIdx)) {
                out = PLAYER_2;
            } else {
                out = 'O';
            }

            ret += out;
            ret += " ";
        }
        ret += '\n';
    }
    return ret;
}

bool Board::isP1Turn() const {
    return turn % 2 == 0;
}


void test() {

    std::string cfef1 = "rrry/yr/ry/yry/yyry/yrry/r";
    std::string cfef2 = "r/r/r/r/y/y/y";
    std::string cfef3 = "/ry/yry/ryr/ryr/yrr/yryy";
    std::string cfef4 = "yy//y/r/r/r/";

    Board board = Board::fromCfef(cfef4);
    std::cout << board.visualRep() << std::endl;

    auto eval = evaluate(board, 2);

    std::cout << eval.value << std::endl;
}

Evaluation::Evaluation(int value, const std::vector<uint32_t> &moves) : value(value), moves(moves) {}
