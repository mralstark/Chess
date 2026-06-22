#pragma once
#include "Board.h"
#include "Rules.h"
#include "Eval.h"

namespace chess {

class Search {
public:
    Move findBestMove(Board& b, int depth);

private:
    int negamax(Board& b, int depth, int alpha, int beta);
    bool isCaptureLike(const Board& b, const Move& m) const;

    Rules rules;
    Eval eval;
};

}
