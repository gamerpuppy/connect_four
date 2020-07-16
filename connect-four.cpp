//
// Created by Keegan Millard on 2020-07-08.
//

#include <c++/4.8.3/chrono>
#include "connect-four.h"


// Bit Manipulation
uint32_t getBitIdx(uint32_t rIdx, uint32_t cIdx) {
    return cIdx * 6u + rIdx;
}

void setBitAtPos(uint64_t &bits, uint32_t rIdx, uint32_t cIdx) {
    bits |= (1llu << getBitIdx(rIdx, cIdx));
}

uint64_t getWithSetBit(uint64_t bits, uint32_t rIdx, uint32_t cIdx) {
    return bits | (1llu << getBitIdx(rIdx, cIdx));
}

bool getBitAtPos(uint64_t bits, uint32_t rIdx, uint32_t cIdx) {
    return bits & (1llu << getBitIdx(rIdx, cIdx));
}

// Board Bit Manipulation

int getOpenRowIdx(uint32_t col) {
    return __builtin_ffs(col | 0b1000000u)-2;
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
    uint32_t diagIdx = (6-cIdx)+rIdx;
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
    uint64_t row = (pieces >> rIdx) & ROW_7_MASK;
    while (row >= ROW_4_MASK) {
        if ((row & ROW_4_MASK) == ROW_4_MASK) {
            return true;
        }
        row >>= 6u;
    }
    return false;
}



bool connectedFour(uint64_t pieces, uint32_t rIdx, uint32_t cIdx) {
    connectFoursEvaluated++;
    return
        colConnectsFour(pieces, cIdx) ||
        rowConnectsFour(pieces, rIdx) ||
        upDiagConnectsFour(pieces, rIdx, cIdx) ||
        dnDiagConnectsFour(pieces, rIdx, cIdx);
}

// evaluation


// Methods

Board::Board(const std::array<uint64_t, 2> &piecesCols) : pieces(piecesCols) {}

Board Board::forMove(uint32_t rIdx, uint32_t cIdx) const {
    if (isP1Turn()) {
        return Board({getWithSetBit(pieces[0], rIdx, cIdx), pieces[1]});
    } else {
        return Board({pieces[0], getWithSetBit(pieces[1], rIdx, cIdx)});
    }
}

Board Board::forMove(uint32_t cIdx) const {
    int rIdx = getOpenRowIdx(getCol(pieces[0]|pieces[1], cIdx));
    return forMove(rIdx, cIdx);
}


Board Board::fromCfef(const std::string &cfef) {
    std::array<uint64_t, 2> pieces{0,0};
    uint32_t cIdx = 0;
    std::istringstream ss(cfef);
    std::string token;

    while(std::getline(ss, token, '/')) {
        int rIdx = 5;
        for (char c : token) {
            int player = c == PLAYER_1 ? 0 : 1;
            setBitAtPos(pieces[player], rIdx, cIdx);
            rIdx--;
        }
        cIdx++;
    }

    return Board(pieces);
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

int Board::turnCount() const {
    return __builtin_popcountll(pieces[0]|pieces[1]);
}

bool Board::isP1Turn() const {
    return turnCount() % 2 == 0;
}

bool isBetter(const EvaluationPart &p1, const EvaluationPart &p2) {
    if (p1.score > p2.score) {
        return true;
    } else if (p1.score == p2.score) {
        if (p1.score > 0) {
            return p1.winIn < p2.winIn;
        } else {
            return p1.winIn > p2.winIn;
        }
    } else {
        return false;
    }
}

EvaluationPart evaluateHelper(uint64_t piecesTurn, uint64_t piecesOther, int depthRem) {
    if (depthRem == 0) {
        leafNodesReached++;
        return {0,0};
    }

    uint64_t  combinedPieces = piecesTurn | piecesOther;
    EvaluationPart best(-2, 0);

    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {
        uint64_t col = getCol(combinedPieces, cIdx);
        int rIdx = getOpenRowIdx(col);
        if (rIdx < 0) {
            continue;
        }

        uint64_t piecesTurnAfter = getWithSetBit(piecesTurn, rIdx, cIdx);
        if (connectedFour(piecesTurnAfter, rIdx, cIdx)) {
            return {1,1};
        }
        auto res = evaluateHelper(piecesOther, piecesTurnAfter, depthRem-1);
        res.winIn++;
        res.score *= -1;
        if (isBetter(res, best)) {
            best = res;
        }
    }

    return best;
}

EvaluationPart evaluateHelperHashed(const uint64_t piecesTurn, const uint64_t piecesOther, const int depthRem, const uint32_t hashDepth, const int hashDepthRem, MultiHashMap<EvaluationPart> &table) {
    if (depthRem == 0) {
        return {0,0};
    }

    if (hashDepthRem == 0) {
        return evaluateHelper(piecesTurn, piecesOther, depthRem);
    }

    const EvaluationPart* evalPtr = table.get(piecesTurn, piecesOther);
    if (evalPtr != nullptr) {
        connectFourHashUses++;
        return *evalPtr;
    }

    uint64_t  combinedPieces = piecesTurn | piecesOther;
    EvaluationPart best(-2, 0);

    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {
        uint64_t col = getCol(combinedPieces, cIdx);
        int rIdx = getOpenRowIdx(col);
        if (rIdx < 0) {
            continue;
        }

        uint64_t piecesTurnAfter = getWithSetBit(piecesTurn, rIdx, cIdx);
        if (connectedFour(piecesTurnAfter, rIdx, cIdx)) {
            best = {1,1};
            break;
        }
        auto res = evaluateHelperHashed(piecesOther, piecesTurnAfter, depthRem-1, hashDepth+1, hashDepthRem-1, table);
        res.winIn++;
        res.score *= -1;
        if (isBetter(res, best)) {
            best = res;
        }
    }
    bool didPut = table.put(piecesTurn, piecesOther, best, hashDepth);
    if (didPut) {
        connectFourHashInserts++;
    } else {
        connectFourHashFailedInserts++;
    }
    return best;
}

Evaluation Board::evaluate(int depth) {
    MultiHashMap<EvaluationPart> table(HASH_TABLE_CAPACITY);
    EvaluationPart best(-2, 0);
    int bestMove = -1;

    uint64_t piecesTurn = isP1Turn() ? pieces[0] : pieces[1];
    uint64_t piecesOther = isP1Turn() ? pieces[1] : pieces[0];

    uint64_t  combinedPieces = piecesTurn | piecesOther;
    for (uint32_t cIdx = 0; cIdx < 7; cIdx++) {
        uint64_t col = getCol(combinedPieces, cIdx);
        int rIdx = getOpenRowIdx(col);
        if (rIdx < 0) {
            continue;
        }

        uint64_t piecesTurnAfter = getWithSetBit(piecesTurn, rIdx, cIdx);
        if (connectedFour(piecesTurnAfter, rIdx, cIdx)) {
            best = EvaluationPart(1, 1);
            bestMove = cIdx;
            break;
        }

        auto res = evaluateHelperHashed(piecesOther, piecesTurnAfter, depth-1, 1, HASH_TABLE_DEPTH-1, table);
        res.winIn++;
        res.score *= -1;
        if (isBetter(res, best)) {
            best = res;
            bestMove = cIdx;
        }
    }

    return {best.score, bestMove, best.winIn};
}

bool Board::doesMoveWin(int move) {
    int rIdx = getOpenRowIdx(getCol(pieces[0] | pieces[1], move));
    uint64_t piecesTurnAfter = getWithSetBit(isP1Turn() ? pieces[0] : pieces[1], rIdx, move);
    return connectedFour(piecesTurnAfter, rIdx, move);
}

bool Board::operator==(const Board &rhs) const {
    return pieces == rhs.pieces;
}

void test() {
    std::string cfef1 = "rrry/yr/ry/yry/yyry/yrry/r";
    std::string cfef2 = "r/r/r/r/y/y/y";
    std::string cfef3 = "/ry/yry/ryr/ryr/yrr/yryy";
    std::string cfef4 = "yyyry//yr/rr/rr/r/y";
    std::string cfef5 = "//////";

    Board board = Board::fromCfef(cfef5);
    std::cout << board.visualRep() << std::endl;

    const int depth = 13;

    auto start = std::chrono::high_resolution_clock::now();
    auto eval = board.evaluate(depth);
    auto end = std::chrono::high_resolution_clock::now();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    std::cout << "depth:" << depth << " move:" << eval.move << " value: " << eval.score << " win in: " << eval.winIn << " millis: " << millis << '\n';
//    std::cout << "hashTableDepth:" << HASH_TABLE_DEPTH << " hashTableCapacity:" << HASH_TABLE_CAPACITY << " subMapCapacity" << MultiHashMap<int>::SUBMAP_CAPACITY << std::endl;
    std::cout << "connectFoursEvaluated:" << connectFoursEvaluated << " leafNodesReached:" << leafNodesReached << std::endl;
    std::cout << "hashTableDepth:" << HASH_TABLE_DEPTH << " hashTableCapacity:" << HASH_TABLE_CAPACITY << std::endl;
    std::cout << "hashInserts:" << connectFourHashInserts << " failedHashInserts:" << connectFourHashFailedInserts << " hashUses:" << connectFourHashUses << std::endl;
    std::cout <<  "tooManySubmaps:" << tooManySubmaps << " tooManyEntries:" << tooManyEntries << std::endl;
}

EvaluationPart::EvaluationPart(int score, uint32_t winIn) : score(score), winIn(winIn) {}

Evaluation::Evaluation(int score, int move, uint32_t winIn) : score(score), move(move), winIn(winIn) {}

