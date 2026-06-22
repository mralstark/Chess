#pragma once
#include <SFML/Graphics.hpp>
#include "../chess/Engine.h"

class Gui {
public:
    Gui(chess::Engine& e);
    void run();

private:
    chess::Engine& engine;
    sf::RenderWindow window;

    // Unicode-шахматы: рисуем фигуры как UTF-8/Unicode символы (♔♕♖♗♘♙ / ♚♛♜♝♞♟).
    // Если шрифт не найден/не поддерживает глифы — автоматически откатываемся на «рисование формами».
    sf::Font pieceFont;
    bool useUnicodePieces = false;

    int cell = 80;
    int selectedSq = -1;
    std::vector<chess::Move> movesFromSelected;

    void handleEvent(const sf::Event& ev);
    void handleMouseClick(int mx, int my);

    int mouseToSquare(int mx, int my) const;

    void draw();
    void drawBoard();
    void drawHighlights();
    void drawPieces();
    void drawOnePiece(const chess::Piece& p, int sq);

    // простая отрисовка фигур формами
    void drawPawn(sf::Vector2f center, float s, bool white);
    void drawKnight(sf::Vector2f center, float s, bool white);
    void drawBishop(sf::Vector2f center, float s, bool white);
    void drawRook(sf::Vector2f center, float s, bool white);
    void drawQueen(sf::Vector2f center, float s, bool white);
    void drawKing(sf::Vector2f center, float s, bool white);

    void resetSelection();
    bool isHumanTurn() const; // человек играет белыми
    void maybeComputerMove();
};
