#include "Gui.h"
#include <algorithm>
#include <string>
#include <vector>

static sf::Color colLight() { return sf::Color(240, 217, 181); }
static sf::Color colDark()  { return sf::Color(181, 136,  99); }
static sf::Color colHL()    { return sf::Color(80,  200, 120, 120); }
static sf::Color colSel()   { return sf::Color(60,  120, 240, 140); }

Gui::Gui(chess::Engine& e)
: engine(e), window(sf::VideoMode(8*80, 8*80), "Chess (student) - SFML")
{
    window.setFramerateLimit(60);

    // Пытаемся включить Unicode-фигуры. Ключевой момент: SFML хранит строку в UTF-32,
    // но мы можем задавать символы по Unicode-кодпойнтам (U+2654..U+265F), что эквивалентно UTF-8 вводу.
    // Для отображения нужен шрифт с этими глифами.
    std::vector<std::string> candidates;

    // 1) Если пользователь положит рядом свой шрифт — используем его в первую очередь.
    // (можно создать папку assets/fonts и положить туда любой ttf с поддержкой шахматных символов)
    candidates.push_back("assets/fonts/DejaVuSans.ttf");
    candidates.push_back("assets/fonts/NotoSansSymbols2-Regular.ttf");
    candidates.push_back("assets/fonts/SegoeUISymbol.ttf");

#if defined(_WIN32)
    // 2) Windows системные шрифты
    candidates.push_back("C:/Windows/Fonts/seguisym.ttf");  // Segoe UI Symbol (обычно есть)
    candidates.push_back("C:/Windows/Fonts/arialuni.ttf");  // Arial Unicode MS (если установлен)
    candidates.push_back("C:/Windows/Fonts/segoeui.ttf");   // иногда тоже содержит
#else
    // 2) Linux (частая установка)
    candidates.push_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    candidates.push_back("/usr/share/fonts/TTF/DejaVuSans.ttf");
#endif

    for (const auto& path : candidates) {
        if (pieceFont.loadFromFile(path)) {
            // Проверим, что глиф короля реально есть.
            const unsigned int testSize = 64;
            auto g = pieceFont.getGlyph(0x2654, testSize, false);
            if (g.advance > 0.f) {
                useUnicodePieces = true;
                break;
            }
        }
    }
}

bool Gui::isHumanTurn() const {
    return engine.board.sideToMove == chess::Color::White;
}

void Gui::resetSelection() {
    selectedSq = -1;
    movesFromSelected.clear();
}

int Gui::mouseToSquare(int mx, int my) const {
    int f = mx / cell;
    int r = my / cell;
    if (f < 0 || f > 7 || r < 0 || r > 7) return -1;
    int boardRank = 7 - r; // чтобы верх экрана был 8-я горизонталь
    return chess::toSq(f, boardRank);
}

void Gui::handleMouseClick(int mx, int my) {
    int sq = mouseToSquare(mx, my);
    if (sq < 0) return;

    if (!isHumanTurn()) return; // ждём ход компьютера

    chess::Piece p = engine.board.at(sq);

    if (selectedSq == -1) {
        // выбираем свою фигуру
        if (!p.isEmpty() && p.color == chess::Color::White) {
            selectedSq = sq;
            movesFromSelected = engine.legalMovesFrom(selectedSq);
        }
        return;
    }

    // если уже выбрано
    if (sq == selectedSq) {
        resetSelection();
        return;
    }

    // если кликнули по своей фигуре - смена выбора
    if (!p.isEmpty() && p.color == chess::Color::White) {
        selectedSq = sq;
        movesFromSelected = engine.legalMovesFrom(selectedSq);
        return;
    }

    // пытаемся сделать ход
    auto it = std::find_if(movesFromSelected.begin(), movesFromSelected.end(),
        [&](const chess::Move& m){ return m.to == sq; });

    if (it != movesFromSelected.end()) {
        engine.playMove(*it);
        resetSelection();
        maybeComputerMove();
    }
}

void Gui::maybeComputerMove() {
    if (engine.board.sideToMove != chess::Color::Black) return;
    // "студенческий" синхронный ход — глубина маленькая
    chess::Move m = engine.computeMove(3);
    if (m.isValid()) engine.playMove(m);
}

void Gui::handleEvent(const sf::Event& ev) {
    if (ev.type == sf::Event::Closed) window.close();

    if (ev.type == sf::Event::KeyPressed) {
        if (ev.key.code == sf::Keyboard::R) {
            engine.reset();
            resetSelection();
        }
        if (ev.key.code == sf::Keyboard::Z) { // undo (два раза чтобы откатить и компа)
            engine.undo();
            engine.undo();
            resetSelection();
        }
    }

    if (ev.type == sf::Event::MouseButtonPressed && ev.mouseButton.button == sf::Mouse::Left) {
        handleMouseClick(ev.mouseButton.x, ev.mouseButton.y);
    }
}

void Gui::run() {
    while (window.isOpen()) {
        sf::Event ev;
        while (window.pollEvent(ev)) handleEvent(ev);

        draw();
        window.display();
    }
}

void Gui::draw() {
    window.clear(sf::Color::Black);
    drawBoard();
    drawHighlights();
    drawPieces();
}

void Gui::drawBoard() {
    for (int r = 0; r < 8; ++r) {
        for (int f = 0; f < 8; ++f) {
            sf::RectangleShape rect(sf::Vector2f((float)cell, (float)cell));
            rect.setPosition((float)(f*cell), (float)(r*cell));
            bool light = ((r + f) % 2 == 0);
            rect.setFillColor(light ? colLight() : colDark());
            window.draw(rect);
        }
    }
}

void Gui::drawHighlights() {
    if (selectedSq == -1) return;

    // подсветка выбранной
    int sf = chess::fileOf(selectedSq);
    int sr = chess::rankOf(selectedSq);
    sf::RectangleShape sel(sf::Vector2f((float)cell, (float)cell));
    sel.setPosition((float)(sf*cell), (float)((7-sr)*cell));
    sel.setFillColor(colSel());
    window.draw(sel);

    // подсветка ходов
    for (auto& m : movesFromSelected) {
        int tf = chess::fileOf(m.to);
        int tr = chess::rankOf(m.to);
        sf::RectangleShape hl(sf::Vector2f((float)cell, (float)cell));
        hl.setPosition((float)(tf*cell), (float)((7-tr)*cell));
        hl.setFillColor(colHL());
        window.draw(hl);
    }
}

static sf::Color pieceFill(bool white) { return white ? sf::Color(235,235,235) : sf::Color(45,45,45); }
static sf::Color pieceLine(bool white) { return white ? sf::Color(30,30,30) : sf::Color(220,220,220); }

void Gui::drawPawn(sf::Vector2f c, float s, bool white) {
    sf::CircleShape head(s*0.22f);
    head.setOrigin(head.getRadius(), head.getRadius());
    head.setPosition(c.x, c.y - s*0.12f);
    head.setFillColor(pieceFill(white));
    head.setOutlineColor(pieceLine(white));
    head.setOutlineThickness(2.f);
    window.draw(head);

    sf::RectangleShape base(sf::Vector2f(s*0.45f, s*0.22f));
    base.setOrigin(base.getSize().x/2, base.getSize().y/2);
    base.setPosition(c.x, c.y + s*0.15f);
    base.setFillColor(pieceFill(white));
    base.setOutlineColor(pieceLine(white));
    base.setOutlineThickness(2.f);
    window.draw(base);
}

void Gui::drawRook(sf::Vector2f c, float s, bool white) {
    sf::RectangleShape tower(sf::Vector2f(s*0.45f, s*0.5f));
    tower.setOrigin(tower.getSize().x/2, tower.getSize().y/2);
    tower.setPosition(c.x, c.y);
    tower.setFillColor(pieceFill(white));
    tower.setOutlineColor(pieceLine(white));
    tower.setOutlineThickness(2.f);
    window.draw(tower);

    // "зубцы"
    for (int i = -1; i <= 1; ++i) {
        sf::RectangleShape cren(sf::Vector2f(s*0.12f, s*0.1f));
        cren.setOrigin(cren.getSize().x/2, cren.getSize().y/2);
        cren.setPosition(c.x + i*s*0.14f, c.y - s*0.3f);
        cren.setFillColor(pieceFill(white));
        cren.setOutlineColor(pieceLine(white));
        cren.setOutlineThickness(2.f);
        window.draw(cren);
    }
}

void Gui::drawKnight(sf::Vector2f c, float s, bool white) {
    // просто "голова коня" треугольником + круг
    sf::CircleShape body(s*0.23f);
    body.setOrigin(body.getRadius(), body.getRadius());
    body.setPosition(c.x, c.y + s*0.08f);
    body.setFillColor(pieceFill(white));
    body.setOutlineColor(pieceLine(white));
    body.setOutlineThickness(2.f);
    window.draw(body);

    sf::ConvexShape head;
    head.setPointCount(3);
    head.setPoint(0, sf::Vector2f(c.x - s*0.18f, c.y - s*0.05f));
    head.setPoint(1, sf::Vector2f(c.x + s*0.22f, c.y - s*0.15f));
    head.setPoint(2, sf::Vector2f(c.x + s*0.05f, c.y + s*0.2f));
    head.setFillColor(pieceFill(white));
    head.setOutlineColor(pieceLine(white));
    head.setOutlineThickness(2.f);
    window.draw(head);
}

void Gui::drawBishop(sf::Vector2f c, float s, bool white) {
    sf::CircleShape body(s*0.25f);
    body.setOrigin(body.getRadius(), body.getRadius());
    body.setPosition(c.x, c.y);
    body.setFillColor(pieceFill(white));
    body.setOutlineColor(pieceLine(white));
    body.setOutlineThickness(2.f);
    window.draw(body);

    sf::RectangleShape slash(sf::Vector2f(s*0.06f, s*0.4f));
    slash.setOrigin(slash.getSize().x/2, slash.getSize().y/2);
    slash.setPosition(c.x, c.y);
    slash.setRotation(25.f);
    slash.setFillColor(pieceLine(white));
    window.draw(slash);
}

void Gui::drawQueen(sf::Vector2f c, float s, bool white) {
    sf::CircleShape body(s*0.25f);
    body.setOrigin(body.getRadius(), body.getRadius());
    body.setPosition(c.x, c.y + s*0.05f);
    body.setFillColor(pieceFill(white));
    body.setOutlineColor(pieceLine(white));
    body.setOutlineThickness(2.f);
    window.draw(body);

    for (int i = -1; i <= 1; ++i) {
        sf::CircleShape crown(s*0.08f);
        crown.setOrigin(crown.getRadius(), crown.getRadius());
        crown.setPosition(c.x + i*s*0.12f, c.y - s*0.22f);
        crown.setFillColor(pieceFill(white));
        crown.setOutlineColor(pieceLine(white));
        crown.setOutlineThickness(2.f);
        window.draw(crown);
    }
}

void Gui::drawKing(sf::Vector2f c, float s, bool white) {
    sf::CircleShape body(s*0.25f);
    body.setOrigin(body.getRadius(), body.getRadius());
    body.setPosition(c.x, c.y + s*0.05f);
    body.setFillColor(pieceFill(white));
    body.setOutlineColor(pieceLine(white));
    body.setOutlineThickness(2.f);
    window.draw(body);

    sf::RectangleShape v(sf::Vector2f(s*0.06f, s*0.30f));
    v.setOrigin(v.getSize().x/2, v.getSize().y/2);
    v.setPosition(c.x, c.y - s*0.22f);
    v.setFillColor(pieceLine(white));
    window.draw(v);

    sf::RectangleShape h(sf::Vector2f(s*0.22f, s*0.06f));
    h.setOrigin(h.getSize().x/2, h.getSize().y/2);
    h.setPosition(c.x, c.y - s*0.22f);
    h.setFillColor(pieceLine(white));
    window.draw(h);
}

void Gui::drawPieces() {
    for (int sq = 0; sq < 64; ++sq) {
        chess::Piece p = engine.board.at(sq);
        if (p.isEmpty()) continue;
        drawOnePiece(p, sq);
    }
}

static sf::Uint32 unicodePiece(const chess::Piece& p) {
    // Unicode chess symbols (white: U+2654..U+2659, black: U+265A..U+265F)
    // Order: King, Queen, Rook, Bishop, Knight, Pawn
    const bool white = (p.color == chess::Color::White);
    const sf::Uint32 base = white ? 0x2654 : 0x265A;
    switch (p.type) {
    case chess::PieceType::King:   return base + 0;
    case chess::PieceType::Queen:  return base + 1;
    case chess::PieceType::Rook:   return base + 2;
    case chess::PieceType::Bishop: return base + 3;
    case chess::PieceType::Knight: return base + 4;
    case chess::PieceType::Pawn:   return base + 5;
    default: return 0;
    }
}

void Gui::drawOnePiece(const chess::Piece& p, int sq) {
    int f = chess::fileOf(sq);
    int r = chess::rankOf(sq);
    float x = f * (float)cell + cell * 0.5f;
    float y = (7 - r) * (float)cell + cell * 0.5f;
    sf::Vector2f c(x, y);
    float s = (float)cell;

    bool white = (p.color == chess::Color::White);

    if (useUnicodePieces) {
        sf::Text t;
        t.setFont(pieceFont);

        // Подгоняем размер под клетку. Юникод-иконки обычно «крупнее» форм, поэтому чуть меньше.
        t.setCharacterSize((unsigned int)(cell * 0.78f));

        sf::String str;
        sf::Uint32 cp = unicodePiece(p);
        if (cp != 0) str += cp;
        t.setString(str);

        // Цвета делаем контрастными, чтобы на любой клетке было читаемо.
        t.setFillColor(white ? sf::Color(245,245,245) : sf::Color(25,25,25));
        t.setOutlineColor(white ? sf::Color(30,30,30) : sf::Color(230,230,230));
        t.setOutlineThickness(2.f);

        // Центрируем по реальным границам глифа
        sf::FloatRect b = t.getLocalBounds();
        t.setOrigin(b.left + b.width/2.f, b.top + b.height/2.f);
        t.setPosition(c);

        window.draw(t);
        return;
    }

    switch (p.type) {
    case chess::PieceType::Pawn:   drawPawn(c, s, white); break;
    case chess::PieceType::Knight: drawKnight(c, s, white); break;
    case chess::PieceType::Bishop: drawBishop(c, s, white); break;
    case chess::PieceType::Rook:   drawRook(c, s, white); break;
    case chess::PieceType::Queen:  drawQueen(c, s, white); break;
    case chess::PieceType::King:   drawKing(c, s, white); break;
    default: break;
    }
}
