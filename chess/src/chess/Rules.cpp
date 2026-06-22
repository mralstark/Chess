#include "Rules.h"

namespace chess {

static bool onBoard(int sq) { return sq >= 0 && sq < 64; }

bool Rules::isSquareAttacked(const Board& b, int sq, Color by) const {
    int sf = fileOf(sq), sr = rankOf(sq);

    // атака пешки
    if (by == Color::White) {
        // белая пешка бьёт вверх
        int p1 = sq - 7;
        int p2 = sq - 9;
        if (sf > 0 && onBoard(p2)) {
            Piece p = b.at(p2);
            if (!p.isEmpty() && p.color == by && p.type == PieceType::Pawn) return true;
        }
        if (sf < 7 && onBoard(p1)) {
            Piece p = b.at(p1);
            if (!p.isEmpty() && p.color == by && p.type == PieceType::Pawn) return true;
        }
    } else {
        // чёрная пешка бьёт вниз
        int p1 = sq + 7;
        int p2 = sq + 9;
        if (sf < 7 && onBoard(p2)) {
            Piece p = b.at(p2);
            if (!p.isEmpty() && p.color == by && p.type == PieceType::Pawn) return true;
        }
        if (sf > 0 && onBoard(p1)) {
            Piece p = b.at(p1);
            if (!p.isEmpty() && p.color == by && p.type == PieceType::Pawn) return true;
        }
    }

    // атака короля
    static const int kOff[8] = { 17, 15, 10, 6, -17, -15, -10, -6 };
    for (int i = 0; i < 8; ++i) {
        int from = sq + kOff[i];
        if (!onBoard(from)) continue;
        int df = fileOf(from) - sf; if (df < 0) df = -df;
        int dr = rankOf(from) - sr; if (dr < 0) dr = -dr;
        if (!((df == 1 && dr == 2) || (df == 2 && dr == 1))) continue;
        Piece p = b.at(from);
        if (!p.isEmpty() && p.color == by && p.type == PieceType::Knight) return true;
    }

    // атака коня
    for (int dr = -1; dr <= 1; ++dr) {
        for (int df = -1; df <= 1; ++df) {
            if (df == 0 && dr == 0) continue;
            int tf = sf + df, tr = sr + dr;
            if (tf < 0 || tf > 7 || tr < 0 || tr > 7) continue;
            int from = toSq(tf, tr);
            Piece p = b.at(from);
            if (!p.isEmpty() && p.color == by && p.type == PieceType::King) return true;
        }
    }

    // ладьи  иферзи горизонталь/вертикаль
    static const int rookDirs[4] = { 8, -8, 1, -1 };
    for (int d : rookDirs) {
        int cur = sq;
        while (true) {
            int nxt = cur + d;
            if (!onBoard(nxt)) break;
            int cf = fileOf(cur), nf = fileOf(nxt);
            if ((d == 1 || d == -1) && (nf - cf > 1 || cf - nf > 1)) break;

            Piece p = b.at(nxt);
            if (!p.isEmpty()) {
                if (p.color == by && (p.type == PieceType::Rook || p.type == PieceType::Queen)) return true;
                break;
            }
            cur = nxt;
        }
    }

    // слоны и ферзи диагональ
    static const int bishDirs[4] = { 9, 7, -9, -7 };
    for (int d : bishDirs) {
        int cur = sq;
        while (true) {
            int nxt = cur + d;
            if (!onBoard(nxt)) break;
            int cf = fileOf(cur), nf = fileOf(nxt);
            if (nf - cf > 1 || cf - nf > 1) break;

            Piece p = b.at(nxt);
            if (!p.isEmpty()) {
                if (p.color == by && (p.type == PieceType::Bishop || p.type == PieceType::Queen)) return true;
                break;
            }
            cur = nxt;
        }
    }

    return false;
}

bool Rules::isKingInCheck(const Board& b, Color kingColor) const {
    int ks = b.findKing(kingColor);
    if (ks < 0) return false; // если короля нет (не должно быть)
    return isSquareAttacked(b, ks, opposite(kingColor));
}

bool Rules::castlePassesAttack(const Board& b, const Move& m) const {
    Piece k = b.at(m.from);
    if (k.type != PieceType::King) return true;
    Color enemy = opposite(k.color);

    // если король сейчас под шахом
    if (isKingInCheck(b, k.color)) return false;

    // проверим клетки, через которые проходит король
    if (k.color == Color::White) {
        if (m.to == toSq(6,0)) { 
            if (isSquareAttacked(b, toSq(5,0), enemy)) return false;
            if (isSquareAttacked(b, toSq(6,0), enemy)) return false;
        } else {
            if (isSquareAttacked(b, toSq(3,0), enemy)) return false;
            if (isSquareAttacked(b, toSq(2,0), enemy)) return false;
        }
    } else {
        if (m.to == toSq(6,7)) { 
            if (isSquareAttacked(b, toSq(5,7), enemy)) return false;
            if (isSquareAttacked(b, toSq(6,7), enemy)) return false;
        } else { 
            if (isSquareAttacked(b, toSq(3,7), enemy)) return false;
            if (isSquareAttacked(b, toSq(2,7), enemy)) return false;
        }
    }
    return true;
}

std::vector<Move> Rules::generateLegalMoves(Board& b) const {
    MoveGen gen;
    std::vector<Move> pseudo = gen.generatePseudoLegal(b);
    std::vector<Move> legal;
    legal.reserve(pseudo.size());

    Color me = b.sideToMove;

    for (const Move& m : pseudo) {
        if (m.isCastle) {
            if (!castlePassesAttack(b, m)) continue;
        }
        b.makeMove(m);
        bool ok = !isKingInCheck(b, me);
        b.undoMove();
        if (ok) legal.push_back(m);
    }
    return legal;
}

std::vector<Move> Rules::legalMovesFrom(Board& b, int from) const {
    std::vector<Move> all = generateLegalMoves(b);
    std::vector<Move> out;
    for (auto& m : all) if (m.from == from) out.push_back(m);
    return out;
}

} 
