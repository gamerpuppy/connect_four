//
// Created by Keegan Millard on 2020-07-08.
//

#ifndef CONNECT_FOUR_CONNECT_FOUR_H
#define CONNECT_FOUR_CONNECT_FOUR_H

#include <cstdint>
#include <array>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <vector>
#include <cstdio>
#include <bitset>
#include <unordered_map>

#include "hash-map.h"

const uint64_t ROW_4_MASK = 0b1000001000001000001;
const uint64_t ROW_7_MASK = 0b1000001000001000001000001000001000001;

const uint64_t UP_DIAG_4_MASK = 0b1000010000100001;
const uint64_t UP_DIAG_5_MASK = 0b100001000010000100001;
const uint64_t UP_DIAG_6_MASK = 0b10000100001000010000100001;

const uint64_t DN_DIAG_4_MASK = 0b1000000100000010000001;
const uint64_t DN_DIAG_5_MASK = 0b10000001000000100000010000001;
const uint64_t DN_DIAG_6_MASK = 0b100000010000001000000100000010000001;

const char PLAYER_1 = 'r';
const char PLAYER_2 = 'y';

const int HASH_TABLE_DEPTH = 10;
const int HASH_TABLE_CAPACITY = 50000;

static int connectFourHashUses = 0;
static int connectFourHashInserts = 0;
static int connectFourHashFailedInserts = 0;

static uint64_t connectFoursEvaluated = 0;
static uint64_t leafNodesReached = 0;

struct EvaluationPart {
    int8_t score = 0;
    uint8_t winIn = 0;
    EvaluationPart() = default;
    EvaluationPart(const EvaluationPart &rhs) = default;
    EvaluationPart(int score, uint32_t winIn);
};

struct Evaluation {
    int score;
    int move;
    uint32_t winIn;

    Evaluation(int score, int move, uint32_t winIn);
};

struct Board {
    std::array<uint64_t, 2> pieces;

    explicit Board(const std::array<uint64_t, 2> &piecesCols);
    Board(const Board &rhs) = default;

    Board forMove(uint32_t rIdx, uint32_t cIdx) const;
    Board forMove(uint32_t cIdx) const;

    static Board fromCfef(const std::string &cfef);

    std::string toCfef() const;
    std::string visualRep() const;

    int turnCount() const;
    bool isP1Turn() const;
    Evaluation evaluate(int depth);
    bool doesMoveWin(int move);
    bool operator==(const Board &rhs) const;
};

void test();






#endif //CONNECT_FOUR_CONNECT_FOUR_H
