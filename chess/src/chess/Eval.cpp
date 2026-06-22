#include "Eval.h"

namespace chess {

static int pieceValue(PieceType t) {
    switch (t) {
    case PieceType::Pawn:   return 100;
    case PieceType::Knight: return 320;
    case PieceType::Bishop: return 330;
    case PieceType::Rook:   return 500;
    case PieceType::Queen:  return 900;
    case PieceType::King:   return 0;
    default: return 0;
    }
}

int Eval::evaluateWhitePOV(const Board& b) const {
    int score = 0;

    // материал
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = b.at(sq);
        if (p.isEmpty()) continue;
        int v = pieceValue(p.type);
        if (p.color == Color::White) score += v;
        else score -= v;
    }

    // очень простой бонус за центр (пешки/фигуры ближе к центру)
    for (int sq = 0; sq < 64; ++sq) {
        Piece p = b.at(sq);
        if (p.isEmpty()) continue;
        int f = fileOf(sq), r = rankOf(sq);
        int df = (f < 4) ? (3 - f) : (f - 4);
        int dr = (r < 4) ? (3 - r) : (r - 4);
        int centerBonus = 6 - (df + dr); // ближе к центру => больше
        if (centerBonus < 0) centerBonus = 0;
        if (p.color == Color::White) score += centerBonus;
        else score -= centerBonus;
    }

    return score;
}

int Eval::evaluateForSideToMove(const Board& b) const {
    int w = evaluateWhitePOV(b);
    return (b.sideToMove == Color::White) ? w : -w;
}

}
