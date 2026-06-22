#pragma once
#include <vector>
#include <string>
#include "Types.h"

namespace chess {

struct UndoInfo {
    Move m;
    Piece moved;     // какая фигура ходила
    Piece captured;  // что съели 
    bool wk, wq, bk, bq;
    int epSquare;
    int halfmoveClock;
    int fullmoveNumber;
};

class Board {
public:
    Piece cells[64]{};

    Color sideToMove = Color::White;
    bool wk = true, wq = true, bk = true, bq = true; // права рокировки
    int epSquare = -1; // -1 если нет
    int halfmoveClock = 0;
    int fullmoveNumber = 1;

    std::vector<UndoInfo> history;

    void setStartPos();

    Piece at(int sq) const { return cells[sq]; }
    void set(int sq, Piece p) { cells[sq] = p; }

    void makeMove(const Move& m);
    void undoMove();

    int findKing(Color c) const;
};

} 
