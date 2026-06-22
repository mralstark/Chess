#pragma once
#include <vector>
#include "Types.h"
#include "Board.h"

namespace chess {

class MoveGen {
public:
    std::vector<Move> generatePseudoLegal(const Board& b) const;

private:
    void genPawn(const Board& b, int from, std::vector<Move>& out) const;
    void genKnight(const Board& b, int from, std::vector<Move>& out) const;
    void genKing(const Board& b, int from, std::vector<Move>& out) const;
    void genSliding(const Board& b, int from, const int* dirs, int dirCount, std::vector<Move>& out) const;

    void addIfValid(const Board& b, int from, int to, std::vector<Move>& out) const;
};

}
