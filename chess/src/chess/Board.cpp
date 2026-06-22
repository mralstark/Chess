#include "Board.h"
#include <cstring>

namespace chess {

static Piece makePiece(PieceType t, Color c) {
    Piece p; p.type = t; p.color = c; return p;
}

void Board::setStartPos() {
    for (int i = 0; i < 64; ++i) cells[i] = Piece{};
    history.clear();

    set(toSq(0,0), makePiece(PieceType::Rook,   Color::White));
    set(toSq(1,0), makePiece(PieceType::Knight, Color::White));
    set(toSq(2,0), makePiece(PieceType::Bishop, Color::White));
    set(toSq(3,0), makePiece(PieceType::Queen,  Color::White));
    set(toSq(4,0), makePiece(PieceType::King,   Color::White));
    set(toSq(5,0), makePiece(PieceType::Bishop, Color::White));
    set(toSq(6,0), makePiece(PieceType::Knight, Color::White));
    set(toSq(7,0), makePiece(PieceType::Rook,   Color::White));
    for (int f = 0; f < 8; ++f) set(toSq(f,1), makePiece(PieceType::Pawn, Color::White));

    set(toSq(0,7), makePiece(PieceType::Rook,   Color::Black));
    set(toSq(1,7), makePiece(PieceType::Knight, Color::Black));
    set(toSq(2,7), makePiece(PieceType::Bishop, Color::Black));
    set(toSq(3,7), makePiece(PieceType::Queen,  Color::Black));
    set(toSq(4,7), makePiece(PieceType::King,   Color::Black));
    set(toSq(5,7), makePiece(PieceType::Bishop, Color::Black));
    set(toSq(6,7), makePiece(PieceType::Knight, Color::Black));
    set(toSq(7,7), makePiece(PieceType::Rook,   Color::Black));
    for (int f = 0; f < 8; ++f) set(toSq(f,6), makePiece(PieceType::Pawn, Color::Black));

    sideToMove = Color::White;
    wk = wq = bk = bq = true;
    epSquare = -1;
    halfmoveClock = 0;
    fullmoveNumber = 1;
}

int Board::findKing(Color c) const {
    for (int i = 0; i < 64; ++i) {
        if (!cells[i].isEmpty() && cells[i].type == PieceType::King && cells[i].color == c)
            return i;
    }
    return -1;
}

static void disableCastlingForRookMove(Board& b, int from, int to) {
    if (from == toSq(0,0) || to == toSq(0,0)) b.wq = false;
    if (from == toSq(7,0) || to == toSq(7,0)) b.wk = false;
    if (from == toSq(0,7) || to == toSq(0,7)) b.bq = false;
    if (from == toSq(7,7) || to == toSq(7,7)) b.bk = false;
}

void Board::makeMove(const Move& m) {
    UndoInfo u{};
    u.m = m;
    u.moved = cells[m.from];
    u.captured = Piece{};
    u.wk = wk; u.wq = wq; u.bk = bk; u.bq = bq;
    u.epSquare = epSquare;
    u.halfmoveClock = halfmoveClock;
    u.fullmoveNumber = fullmoveNumber;

    Piece moving = cells[m.from];
    Piece target = cells[m.to];

    epSquare = -1;

    bool isCapture = false;
    // взятие на проходе
    if (m.isEnPassant) {
        int capSq = (moving.color == Color::White) ? (m.to - 8) : (m.to + 8);
        u.captured = cells[capSq];
        cells[capSq] = Piece{};
        isCapture = true;
    } else if (!target.isEmpty()) {
        u.captured = target;
        isCapture = true;
    }

    // обновляем права рокировки
    if (moving.type == PieceType::King) {
        if (moving.color == Color::White) { wk = false; wq = false; }
        else { bk = false; bq = false; }
    }
    if (moving.type == PieceType::Rook) {
        disableCastlingForRookMove(*this, m.from, m.to);
    }
    if (isCapture && u.captured.type == PieceType::Rook) {
        disableCastlingForRookMove(*this, m.from, m.to);
    }

    // ход королём при рокировке двигает и ладью
    if (m.isCastle) {
        cells[m.from] = Piece{};
        cells[m.to] = moving;

        if (moving.color == Color::White) {
            if (m.to == toSq(6,0)) { // O-O
                Piece rook = cells[toSq(7,0)];
                cells[toSq(7,0)] = Piece{};
                cells[toSq(5,0)] = rook;
            } else { // O-O-O
                Piece rook = cells[toSq(0,0)];
                cells[toSq(0,0)] = Piece{};
                cells[toSq(3,0)] = rook;
            }
        } else {
            if (m.to == toSq(6,7)) { // O-O
                Piece rook = cells[toSq(7,7)];
                cells[toSq(7,7)] = Piece{};
                cells[toSq(5,7)] = rook;
            } else { // O-O-O
                Piece rook = cells[toSq(0,7)];
                cells[toSq(0,7)] = Piece{};
                cells[toSq(3,7)] = rook;
            }
        }
    } else {
        cells[m.from] = Piece{};
        cells[m.to] = moving;
    }

    // промоция (меняем тип пешки)
    if (moving.type == PieceType::Pawn && m.promo != PieceType::None) {
        cells[m.to].type = m.promo;
    }

    if (moving.type == PieceType::Pawn) {
        int dr = rankOf(m.to) - rankOf(m.from);
        if (dr == 2) epSquare = m.from + 8;
        if (dr == -2) epSquare = m.from - 8;
    }

    if (moving.type == PieceType::Pawn || isCapture) halfmoveClock = 0;
    else halfmoveClock++;

    if (sideToMove == Color::Black) fullmoveNumber++;

    sideToMove = opposite(sideToMove);

    history.push_back(u);
}

void Board::undoMove() {
    if (history.empty()) return;
    UndoInfo u = history.back();
    history.pop_back();

    // вернуть сторону, счётчики и права
    sideToMove = opposite(sideToMove);
    wk = u.wk; wq = u.wq; bk = u.bk; bq = u.bq;
    epSquare = u.epSquare;
    halfmoveClock = u.halfmoveClock;
    fullmoveNumber = u.fullmoveNumber;

    Move m = u.m;
    Piece moving = u.moved;

    // откатываем рокировку отдельно
    if (m.isCastle) {
        cells[m.from] = moving;
        cells[m.to] = Piece{};

        if (moving.color == Color::White) {
            if (m.to == toSq(6,0)) {
                Piece rook = cells[toSq(5,0)];
                cells[toSq(5,0)] = Piece{};
                cells[toSq(7,0)] = rook;
            } else {
                Piece rook = cells[toSq(3,0)];
                cells[toSq(3,0)] = Piece{};
                cells[toSq(0,0)] = rook;
            }
        } else {
            if (m.to == toSq(6,7)) {
                Piece rook = cells[toSq(5,7)];
                cells[toSq(5,7)] = Piece{};
                cells[toSq(7,7)] = rook;
            } else {
                Piece rook = cells[toSq(3,7)];
                cells[toSq(3,7)] = Piece{};
                cells[toSq(0,7)] = rook;
            }
        }
        return;
    }

    cells[m.from] = moving;

    cells[m.to] = Piece{};

    if (m.isEnPassant) {
        int capSq = (moving.color == Color::White) ? (m.to - 8) : (m.to + 8);
        cells[capSq] = u.captured;
    } else if (!u.captured.isEmpty()) {
        cells[m.to] = u.captured;
    }
}

}
