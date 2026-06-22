#pragma once
#include <vector>
#include "Board.h"
#include "Rules.h"
#include "Search.h"

namespace chess {

class Engine {
public:
    Board board;

    Engine();

    void reset();
    std::vector<Move> legalMoves() { return rules.generateLegalMoves(board); }
    std::vector<Move> legalMovesFrom(int from) { return rules.legalMovesFrom(board, from); }

    void playMove(const Move& m) { board.makeMove(m); }
    void undo() { board.undoMove(); }

    Move computeMove(int depth) { return search.findBestMove(board, depth); }

    bool inCheck(Color c) const { return rules.isKingInCheck(board, c); }

private:
    Rules rules;
    Search search;
};

}
