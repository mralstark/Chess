#pragma once
#include <cstdint>

namespace chess {

enum class Color : uint8_t { White = 0, Black = 1 };
enum class PieceType : uint8_t { None = 0, Pawn, Knight, Bishop, Rook, Queen, King };

inline Color opposite(Color c) { return (c == Color::White) ? Color::Black : Color::White; }

struct Piece {
    PieceType type = PieceType::None;
    Color color = Color::White; 

    bool isEmpty() const { return type == PieceType::None; }
};

struct Move {
    int from = -1;
    int to = -1;
    PieceType promo = PieceType::None;
    bool isCastle = false;
    bool isEnPassant = false;

    bool isValid() const { return from >= 0 && to >= 0; }
};

inline int fileOf(int sq) { return sq & 7; }
inline int rankOf(int sq) { return sq >> 3; }
inline int toSq(int file, int rank) { return rank * 8 + file; }

}
