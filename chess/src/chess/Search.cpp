#include "Search.h"
#include <algorithm>
#include <limits>

namespace chess {

static const int INF = 100000;


bool Search::isCaptureLike(const Board& b, const Move& m) const {
    if (m.isEnPassant) return true;
    Piece t = b.at(m.to);
    return !t.isEmpty();
}

Move Search::findBestMove(Board& b, int depth) {
    std::vector<Move> moves = rules.generateLegalMoves(b);
    if (moves.empty()) return Move{};

    // простое упорядочивание взятия вперёд
    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& c) {
        return isCaptureLike(b, a) > isCaptureLike(b, c);
    });

    int bestScore = -INF;
    Move best = moves[0];

    int alpha = -INF, beta = INF;
    for (const Move& m : moves) {
        b.makeMove(m);
        int score = -negamax(b, depth - 1, -beta, -alpha);
        b.undoMove();

        if (score > bestScore) {
            bestScore = score;
            best = m;
        }
        if (score > alpha) alpha = score;
    }
    return best;
}

int Search::negamax(Board& b, int depth, int alpha, int beta) {
    if (depth <= 0) {
        return eval.evaluateForSideToMove(b);
    }

    std::vector<Move> moves = rules.generateLegalMoves(b);

    if (moves.empty()) {
        // мат или пат
        bool inCheck = rules.isKingInCheck(b, b.sideToMove);
        if (inCheck) return -INF + 10;
        return 0;
    }

    std::stable_sort(moves.begin(), moves.end(), [&](const Move& a, const Move& c) {
        return isCaptureLike(b, a) > isCaptureLike(b, c);
    });

    int best = -INF;

    for (const Move& m : moves) {
        b.makeMove(m);
        int score = -negamax(b, depth - 1, -beta, -alpha);
        b.undoMove();

        if (score > best) best = score;
        if (score > alpha) alpha = score;
        if (alpha >= beta) break; // отсечение
    }

    return best;
}

}
