#include "MoveGen.h"
#include <algorithm>

namespace chess {

static bool onBoard(int sq) { return sq >= 0 && sq < 64; }

void MoveGen::addIfValid(const Board& b, int from, int to, std::vector<Move>& out) const {
    if (!onBoard(to)) return;
    Piece me = b.at(from);
    Piece t = b.at(to);
    if (t.isEmpty() || t.color != me.color) {
        Move m; m.from = from; m.to = to;
        out.push_back(m);
    }
}

void MoveGen::genPawn(const Board& b, int from, std::vector<Move>& out) const {
    Piece p = b.at(from);
    int f = fileOf(from);
    int r = rankOf(from);
    int dir = (p.color == Color::White) ? 8 : -8;
    int startRank = (p.color == Color::White) ? 1 : 6;
    int promoRank = (p.color == Color::White) ? 7 : 0;

    int one = from + dir;
    if (onBoard(one) && b.at(one).isEmpty()) {
        // промоция
        if (rankOf(one) == promoRank) {
            PieceType promos[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
            for (PieceType pr : promos) { Move m; m.from = from; m.to = one; m.promo = pr; out.push_back(m); }
        } else {
            Move m; m.from = from; m.to = one; out.push_back(m);
        }

        // двойной ход
        int two = from + 2 * dir;
        if (r == startRank && onBoard(two) && b.at(two).isEmpty()) {
            Move m2; m2.from = from; m2.to = two;
            out.push_back(m2);
        }
    }

    // взятия по диагонали
    int cap1 = from + dir + 1;
    int cap2 = from + dir - 1;

    if (f < 7 && onBoard(cap1)) {
        Piece t = b.at(cap1);
        if (!t.isEmpty() && t.color != p.color) {
            if (rankOf(cap1) == promoRank) {
                PieceType promos[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
                for (PieceType pr : promos) { Move m; m.from = from; m.to = cap1; m.promo = pr; out.push_back(m); }
            } else {
                Move m; m.from = from; m.to = cap1; out.push_back(m);
            }
        }
    }
    if (f > 0 && onBoard(cap2)) {
        Piece t = b.at(cap2);
        if (!t.isEmpty() && t.color != p.color) {
            if (rankOf(cap2) == promoRank) {
                PieceType promos[4] = {PieceType::Queen, PieceType::Rook, PieceType::Bishop, PieceType::Knight};
                for (PieceType pr : promos) { Move m; m.from = from; m.to = cap2; m.promo = pr; out.push_back(m); }
            } else {
                Move m; m.from = from; m.to = cap2; out.push_back(m);
            }
        }
    }

    // en passant
    if (b.epSquare != -1) {
        if (f < 7 && cap1 == b.epSquare) {
            Move m; m.from = from; m.to = cap1; m.isEnPassant = true; out.push_back(m);
        }
        if (f > 0 && cap2 == b.epSquare) {
            Move m; m.from = from; m.to = cap2; m.isEnPassant = true; out.push_back(m);
        }
    }
}

void MoveGen::genKnight(const Board& b, int from, std::vector<Move>& out) const {
    static const int off[8] = { 17, 15, 10, 6, -17, -15, -10, -6 };
    int ff = fileOf(from);
    int rr = rankOf(from);
    for (int k = 0; k < 8; ++k) {
        int to = from + off[k];
        if (!onBoard(to)) continue;
        int tf = fileOf(to), tr = rankOf(to);
        // защита от перелёта через край 
        int df = tf - ff; if (df < 0) df = -df;
        int dr = tr - rr; if (dr < 0) dr = -dr;
        if (!((df == 1 && dr == 2) || (df == 2 && dr == 1))) continue;
        addIfValid(b, from, to, out);
    }
}

void MoveGen::genKing(const Board& b, int from, std::vector<Move>& out) const {
    for (int dr = -1; dr <= 1; ++dr) {
        for (int df = -1; df <= 1; ++df) {
            if (df == 0 && dr == 0) continue;
            int tf = fileOf(from) + df;
            int tr = rankOf(from) + dr;
            if (tf < 0 || tf > 7 || tr < 0 || tr > 7) continue;
            addIfValid(b, from, toSq(tf, tr), out);
        }
    }

    // рокировка (только как псевдоход)
    Piece k = b.at(from);
    if (k.color == Color::White && rankOf(from) == 0 && fileOf(from) == 4) {
        if (b.wk && b.at(toSq(5,0)).isEmpty() && b.at(toSq(6,0)).isEmpty()) {
            Move m; m.from = from; m.to = toSq(6,0); m.isCastle = true; out.push_back(m);
        }
        if (b.wq && b.at(toSq(3,0)).isEmpty() && b.at(toSq(2,0)).isEmpty() && b.at(toSq(1,0)).isEmpty()) {
            Move m; m.from = from; m.to = toSq(2,0); m.isCastle = true; out.push_back(m);
        }
    }
    if (k.color == Color::Black && rankOf(from) == 7 && fileOf(from) == 4) {
        if (b.bk && b.at(toSq(5,7)).isEmpty() && b.at(toSq(6,7)).isEmpty()) {
            Move m; m.from = from; m.to = toSq(6,7); m.isCastle = true; out.push_back(m);
        }
        if (b.bq && b.at(toSq(3,7)).isEmpty() && b.at(toSq(2,7)).isEmpty() && b.at(toSq(1,7)).isEmpty()) {
            Move m; m.from = from; m.to = toSq(2,7); m.isCastle = true; out.push_back(m);
        }
    }
}

void MoveGen::genSliding(const Board& b, int from, const int* dirs, int dirCount, std::vector<Move>& out) const {
    Piece me = b.at(from);
    for (int i = 0; i < dirCount; ++i) {
        int d = dirs[i];
        int cur = from;
        while (true) {
            int next = cur + d;
            if (!onBoard(next)) break;

            // защита от перелёта по горизонтали
            int cf = fileOf(cur), nf = fileOf(next);
            if ((d == 1 || d == -1 || d == 9 || d == -9 || d == 7 || d == -7) && (nf - cf > 1 || cf - nf > 1))
                break;

            Piece t = b.at(next);
            if (t.isEmpty()) {
                Move m; m.from = from; m.to = next; out.push_back(m);
            } else {
                if (t.color != me.color) { Move m; m.from = from; m.to = next; out.push_back(m); }
                break;
            }
            cur = next;
        }
    }
}

std::vector<Move> MoveGen::generatePseudoLegal(const Board& b) const {
    std::vector<Move> out;
    out.reserve(64);

    for (int sq = 0; sq < 64; ++sq) {
        Piece p = b.at(sq);
        if (p.isEmpty()) continue;
        if (p.color != b.sideToMove) continue;

        switch (p.type) {
        case PieceType::Pawn:
            genPawn(b, sq, out);
            break;
        case PieceType::Knight:
            genKnight(b, sq, out);
            break;
        case PieceType::Bishop: {
            static const int dirs[4] = { 9, 7, -9, -7 };
            genSliding(b, sq, dirs, 4, out);
            break;
        }
        case PieceType::Rook: {
            static const int dirs[4] = { 8, -8, 1, -1 };
            genSliding(b, sq, dirs, 4, out);
            break;
        }
        case PieceType::Queen: {
            static const int dirs[8] = { 8, -8, 1, -1, 9, 7, -9, -7 };
            genSliding(b, sq, dirs, 8, out);
            break;
        }
        case PieceType::King:
            genKing(b, sq, out);
            break;
        default:
            break;
        }
    }
    return out;
}

}
