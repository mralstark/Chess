#pragma once
#include <vector>
#include "Types.h"
#include "Board.h"
#include "MoveGen.h"

namespace chess {

class Rules {
public:
    bool isSquareAttacked(const Board& b, int sq, Color by) const;
    bool isKingInCheck(const Board& b, Color kingColor) const;

    std::vector<Move> generateLegalMoves(Board& b) const;
    std::vector<Move> legalMovesFrom(Board& b, int from) const;

private:
    bool castlePassesAttack(const Board& b, const Move& m) const;
};

} 
