#pragma once
#include "Board.h"

namespace chess {

class Eval {
public:
    int evaluateWhitePOV(const Board& b) const; // + хорошо белым
    int evaluateForSideToMove(const Board& b) const;
};

}
