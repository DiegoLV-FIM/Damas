#include "raylib.h"
#include "raymath.h"
#include <array>
#include <optional>
#include <stdexcept>

namespace Colors {
const inline Color Background = BEIGE;
const inline Color LightTile = RAYWHITE;
const inline Color DarkTile = {45, 45, 45, 255};
const inline Color Lines = BLACK;
const inline Color LightPiece = DARKBLUE;
const inline Color DarkPiece = MAROON;
const inline Color PieceBorder = BLACK;
const inline Color Hover = Fade(BLACK, 0.30f);
const inline Color Selected = Fade(GOLD, 0.55f);
}; // namespace Colors

class Pos;

class Pos {
public:
  int x, y;

  bool operator==(const Pos &other) const {
    return x == other.x && y == other.y;
  }
  bool operator!=(const Pos &other) const { return !(*this == other); }

  Pos operator+(const Pos &other) const { return {x + other.x, y + other.y}; }
  Pos operator-(const Pos &other) const { return {x - other.x, y - other.y}; }
  Pos operator/(int scalar) const { return {x / scalar, y / scalar}; }

  bool inBounds(Pos hi, Pos lo) const {
    return x >= hi.x && x < lo.x && y >= hi.y && y < lo.y;
  };

  Vector2 toScreenPos(Vector2 origin, float size) const {
    return origin + Vector2{(float)x, (float)y} * size;
  }
};

struct Capture {
  Pos from;
  Pos move_to;
  Pos move_over;

  static Capture fromTo(Pos from, Pos to) {
    return {from, to, from + (to - from) / 2};
  }
};

class Board {
public:
  struct Piece {
    enum class Kind { Empty, Light, Dark };
    Kind color;
    Pos pos;
    bool is_queen = false;

    Piece(Kind color = Kind::Empty, Pos pos = {0, 0}, bool is_queen = false)
        : color(color), pos(pos), is_queen(is_queen) {}

    bool isLight() const { return color == Kind::Light; }
    bool isDark() const { return color == Kind::Dark; }

    bool isOponent(Piece::Kind other) const {
      return color != other && other != Piece::Kind::Empty;
    }

    void Draw(Vector2 center, float radius) const noexcept {
      if (color == Piece::Kind::Empty)
        return;

      Color color = isLight() ? Colors::LightPiece : Colors::DarkPiece;
      DrawCircleV(center, radius, Colors::PieceBorder);
      DrawCircleV(center, radius - 4.f, color);
      DrawCircleV(center, radius / 1.5f, Fade(BLACK, .2f));

      if (is_queen) {
        DrawCircleV(center, radius - 4.f, Fade(BLACK, 0.5f));
      }
    }
  };

  Board() noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        pieces[y][x] = {Piece::Kind::Empty, {x, y}, false};
        if ((x + y) % 2 != 1)
          continue;

        if (y < 3)
          pieces[y][x].color = Piece::Kind::Dark;
        else if (y >= 5)
          pieces[y][x].color = Piece::Kind::Light;
      }
    }
  }

  void Draw(Vector2 pos, float size, std::optional<Pos> highlight,
            std::optional<Pos> selected) const noexcept {
    DrawRectangle(pos.x, pos.y, size, size, Colors::LightTile);
    const float tile_size = size / 8.f;

    if (highlight) {
      DrawTile(pos, *highlight, tile_size, Colors::Hover);
    }

    for (int y = 0; y < 8; y++) {
      for (int x = y % 2; x < 8; x += 2) {
        DrawTile(pos, {x, y}, tile_size, Colors::DarkTile);
      }
    }

    if (selected) {
      DrawTile(pos, *selected, tile_size, Colors::Selected);
    }

    DrawBoardLines(pos, size, tile_size);
    DrawAllPieces(pos, tile_size);
  }

  const Piece &operator[](Pos tile) const noexcept {
    return pieces[tile.y][tile.x];
  }

  bool hasCaptures() const noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 1 - (y % 2); x < 8; x += 2) {
        const Pos from = {x, y};
        if (get(from).color != turn())
          continue;

        if (hasCaptures(from))
          return true;
      }
    }
    return false;
  }

  bool hasCaptures(Pos from) const noexcept {
    if (!from.inBounds({0, 0}, {8, 8}))
      return false;

    for (int dx : {-1, 1}) {
      for (int dy : {-1, 1}) {
        const Pos move = {dx, dy};
        const Pos over = from + move;
        const Pos to = over + move;
        if (validCapture({from, to, over})) {
          return true;
        }
      }
    }

    return false;
  }

  bool validCapture(Capture c) const noexcept {
    if (!c.from.inBounds({0, 0}, {8, 8}) ||
        !c.move_to.inBounds({0, 0}, {8, 8}) ||
        !c.move_over.inBounds({0, 0}, {8, 8}))
      return false;

    const Piece piece = get(c.from);
    if (std::abs(c.move_to.x - c.from.x) != 2)
      return false;

    const int dy = c.move_to.y - c.from.y;
    if (!piece.is_queen) {
      if (piece.isLight() && dy != -2)
        return false;
      else if (piece.isDark() && dy != 2)
        return false;

    } else if (std::abs(dy) != 2) {
      return false;
    }

    const Piece over = get(c.move_over);
    const Piece to = get(c.move_to);

    if (to.color != Piece::Kind::Empty)
      return false;

    return piece.isOponent(over.color);
  }

  void capture(Capture c) {
    if (!validCapture(c))
      throw std::invalid_argument("Invalid capture");

    Piece &over_piece = get(c.move_over);
    moveUnchecked(c.from, c.move_to);
    over_piece.color = Piece::Kind::Empty;
    over_piece.is_queen = false;

    if (!hasCaptures(c.move_to)) {
      white_move = !white_move;
    }
  }

  bool validMove(Pos from, Pos to) const noexcept {
    if (hasCaptures())
      return false;

    if (!from.inBounds({0, 0}, {8, 8}) || !to.inBounds({0, 0}, {8, 8}))
      return false;

    if (get(from).color != turn())
      return false;

    Piece p = get(from);
    if (p.color == Piece::Kind::Empty)
      return false;

    if (!to.inBounds({0, 0}, {8, 8}))
      return false;

    if (get(to).color != Piece::Kind::Empty)
      return false;

    int dx = to.x - from.x;
    if (std::abs(dx) != 1)
      return false;

    int dy = to.y - from.y;
    if (p.is_queen)
      return std::abs(dy) == 1;

    if (p.isLight())
      return dy == -1; // white advances toward y == 0
    else if (p.isDark())
      return dy == 1; // black advances toward y == 7

    return false;
  }

  void move(Pos from, Pos to) {
    if (hasCaptures())
      throw std::invalid_argument("Must capture if possible");

    if (!validMove(from, to))
      throw std::invalid_argument("Invalid move");

    moveUnchecked(from, to);
    white_move = !white_move;
  }

  Piece::Kind turn() const noexcept {
    return white_move ? Piece::Kind::Light : Piece::Kind::Dark;
  }

private:
  Piece &get(Pos tile) noexcept { return pieces[tile.y][tile.x]; }
  const Piece &get(Pos tile) const noexcept { return pieces[tile.y][tile.x]; }
  void moveUnchecked(Pos from, Pos to) noexcept {
    Piece &from_piece = get(from);
    Piece &to_piece = get(to);

    to_piece.color = from_piece.color;
    to_piece.is_queen = from_piece.is_queen;
    if (from_piece.color == Piece::Kind::Light && to.y == 0) {
      to_piece.is_queen = true;
    } else if (from_piece.color == Piece::Kind::Dark && to.y == 7) {
      to_piece.is_queen = true;
    }

    from_piece.color = Piece::Kind::Empty;
    from_piece.is_queen = false;
  }

  std::array<std::array<Piece, 8>, 8> pieces;
  bool white_move = true;

  static inline void DrawTile(Vector2 origin, Pos tile, float tile_size,
                              Color color) noexcept {
    const Vector2 tile_pos = tile.toScreenPos(origin, tile_size);
    DrawRectangle(tile_pos.x, tile_pos.y, tile_size, tile_size, color);
  }

  static void DrawBoardLines(Vector2 pos, float size,
                             float tile_size) noexcept {
    constexpr float thick = 4.f;
    constexpr float half_thick = thick / 2;

    const Vector2 start = Vector2SubtractValue(pos, half_thick);
    const Vector2 end = Vector2AddValue(pos, half_thick + size);
    for (int i = 0; i <= 8; i++) {
      const Vector2 off = Vector2AddValue(pos, tile_size * i);
      DrawLineEx({off.x, start.y}, {off.x, end.y}, thick, Colors::Lines);
      DrawLineEx({start.x, off.y}, {end.x, off.y}, thick, Colors::Lines);
    };
  }

  void DrawAllPieces(Vector2 pos, float tile_size) const noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 1 - (y % 2); x < 8; x += 2) {
        const Pos tile = {x, y};
        const Vector2 tile_pos = tile.toScreenPos(pos, tile_size);
        const Vector2 center = Vector2AddValue(tile_pos, tile_size * .5f);
        get(tile).Draw(center, tile_size * .32f);
      }
    }
  }
};

std::optional<Pos> getMousePos(Vector2 board_pos, float tile_size) noexcept {
  const Vector2 mouse = (GetMousePosition() - board_pos) / tile_size;
  const Pos tile = {(int)mouse.x, (int)mouse.y};
  if (!tile.inBounds({0, 0}, {8, 8})) {
    return std::nullopt;
  }
  return tile;
}

class Damas {
  Board board;
  std::optional<Pos> selected;

  bool HandleMove(Pos to) noexcept {
    const Pos from = *selected;
    const Capture capture = Capture::fromTo(from, to);
    if (board.validCapture(capture)) {
      board.capture(capture);

      if (board.hasCaptures(to)) {
        selected = to;
      } else {
        selected = std::nullopt;
      }

      return true;
    }

    if (!board.validMove(*selected, to))
      return false;

    board.move(*selected, to);
    selected = std::nullopt;
    return true;
  }

  void HandleClick(Vector2 board_pos, float board_size) noexcept {
    const auto tile = getMousePos(board_pos, board_size / 8.f);
    if (!tile)
      return;

    if (selected && HandleMove(*tile))
      return;

    const Board::Piece &piece = board[*tile];
    if (board.turn() == piece.color) {
      selected = tile;
    }
  }

public:
  void Draw(Vector2 board_pos, float board_size) const noexcept {
    board.Draw(board_pos, board_size, getMousePos(board_pos, board_size / 8.f),
               selected);
  }

  void Update(Vector2 board_pos, float board_size) noexcept {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      HandleClick(board_pos, board_size);
    }
  }
};

void DrawTimer(Vector2 pos, int font_size, float time) noexcept {
  const auto time_text =
      TextFormat("%02d:%02d", (int)time / 60, (int)time % 60);
  const int text_len = MeasureText(time_text, font_size);
  DrawText(time_text, pos.x - text_len / 2.f, pos.y, font_size, BLACK);
}

int main() {
  Damas damas;

  InitWindow(800, 800, "Damas");
  SetTargetFPS(60);

  const Vector2 board_pos = {100.f, 50.f};
  const float board_size = 600.f;
  const Vector2 timer_pos =
      board_pos + Vector2{board_size / 2.f, board_size + 20.f};

  const double time_start = GetTime();
  while (!WindowShouldClose()) {
    damas.Update(board_pos, board_size);
    BeginDrawing();
    ClearBackground(Colors::Background);
    DrawTimer(timer_pos, 80, GetTime() - time_start);
    damas.Draw(board_pos, board_size);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}
