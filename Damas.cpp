#include "raylib.h"
#include "raymath.h"
#include <optional>
#include <vector>

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

class Pos {
public:
  int x, y;

  bool operator==(const Pos &other) const {
    return x == other.x && y == other.y;
  }

  bool operator!=(const Pos &other) const { return !(*this == other); }

  bool inBounds(Pos hi, Pos lo) const {
    return x >= hi.x && x < lo.x && y >= hi.y && y < lo.y;
  };
};

static inline std::optional<Pos> getMouseTile(Vector2 board_pos,
                                              float tile_size) noexcept {
  const Vector2 mouse = (GetMousePosition() - board_pos) / tile_size;
  const Pos tile = {(int)mouse.x, (int)mouse.y};
  if (!tile.inBounds({0, 0}, {8, 8}))
    return std::nullopt;
  return Pos{(int)mouse.x, (int)mouse.y};
}

static inline Vector2 getTilePos(Pos tile, Vector2 pos, float size) noexcept {
  return pos + Vector2{(float)tile.x, (float)tile.y} * size;
}

class Damas {
  enum class Piece {
    Empty,
    Light,
    QLight,
    Dark,
    QDark,
  };

  Piece pieces[8][8];
  bool white_move = true;
  std::optional<Pos> selected;

  Piece &getPiece(Pos tile) { return pieces[tile.y][tile.x]; }

  static bool isLightPiece(Piece p) {
    return p == Piece::Light || p == Piece::QLight;
  }

  static bool isDarkPiece(Piece p) {
    return p == Piece::Dark || p == Piece::QDark;
  }

  static bool isOpponent(Piece p, bool is_white) {
    return is_white ? isDarkPiece(p) : isLightPiece(p);
  };

  struct Capture {
    Pos move_to;
    Pos captures;
  };

  std::vector<Capture> getCaptures(Pos from) {
    std::vector<Capture> captures;
    const Piece p = getPiece(from);
    if (p == Piece::Empty)
      return {};

    const bool is_white = isLightPiece(p);
    const int dy_dir = is_white ? -1 : 1;

    for (int dx : {-1, 1}) {
      const Pos over = {from.x + dx, from.y + dy_dir};
      const Pos to = {over.x + dx, over.y + dy_dir};

      if (!to.inBounds({0, 0}, {8, 8}))
        continue;

      const Piece over_piece = getPiece(over);
      if (isOpponent(over_piece, is_white) && getPiece(to) == Piece::Empty) {
        captures.push_back({to, over});
      }
    }

    return captures;
  }

  bool isValidMove(Pos from, Pos to) {
    Piece p = getPiece(from);
    if (p == Piece::Empty)
      return false;
    if (!to.inBounds({0, 0}, {8, 8}))
      return false;
    if (getPiece(to) != Piece::Empty)
      return false;

    const int dx = to.x - from.x;
    const int dy = to.y - from.y;
    if (std::abs(dx) != 1)
      return false;

    if (isLightPiece(p))
      return dy == -1; // white advances toward y == 0
    if (isDarkPiece(p))
      return dy == 1; // black advances toward y == 7
    return false;
  }

  void HandleClick(Vector2 board_pos, float board_size) noexcept {
    const auto tile = getMouseTile(board_pos, board_size / 8.f);
    if (!tile)
      return;

    Piece &piece = getPiece(*tile);
    if (selected) {
      Piece &sel_piece = getPiece(*selected);
      if (auto captures = getCaptures(*selected); !captures.empty()) {
        for (const auto &capture : captures) {
          if (capture.move_to != *tile)
            continue;

          piece = sel_piece;
          sel_piece = Piece::Empty;

          Piece &captured = getPiece(capture.captures);
          captured = Piece::Empty;

          selected = *tile;
          if (getCaptures(*selected).empty()) {
            white_move = !white_move;
            selected = std::nullopt;
          }
        }
        return;
      }

      if (isValidMove(*selected, *tile)) {
        piece = sel_piece;
        sel_piece = Piece::Empty;
        white_move = !white_move;
        selected = std::nullopt;
        return;
      }
    }

    switch (piece) {
    case Piece::Empty:
      break;
    case Piece::Light:
    case Piece::QLight:
      if (white_move) {
        selected = tile;
      }
      break;
    case Piece::Dark:
    case Piece::QDark:
      if (!white_move) {
        selected = tile;
      }
      break;
    }
  }

  static inline void DrawTile(Vector2 origin, Pos tile, float tile_size,
                              Color color) noexcept {
    const Vector2 tile_pos = getTilePos(tile, origin, tile_size);
    DrawRectangle(tile_pos.x, tile_pos.y, tile_size, tile_size, color);
  }

  static inline void HighlightMouse(Vector2 pos, float tile_size) noexcept {
    if (auto tile = getMouseTile(pos, tile_size)) {
      DrawTile(pos, *tile, tile_size, Colors::Hover);
    }
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

  static void DrawPiece(Piece tile, Vector2 center, float radius) noexcept {
    Color color;
    switch (tile) {
    case Piece::Empty:
      return;
    case Piece::QLight:
    case Piece::Light:
      color = Colors::LightPiece;
      break;
    case Piece::QDark:
    case Piece::Dark:
      color = Colors::DarkPiece;
      break;
    }
    DrawCircleV(center, radius, Colors::PieceBorder);
    DrawCircleV(center, radius - 4.f, color);
  }

  void DrawAllPieces(Vector2 pos, float tile_size) noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 1 - (y % 2); x < 8; x += 2) {
        const Pos tile = {x, y};
        const Vector2 tile_pos = getTilePos(tile, pos, tile_size);
        const Vector2 center = Vector2AddValue(tile_pos, tile_size * .5f);
        DrawPiece(getPiece(tile), center, tile_size * .32f);
      }
    }
  }

public:
  Damas() noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        pieces[y][x] = Piece::Empty;
        if ((x + y) % 2 != 1)
          continue;

        if (y < 3)
          pieces[y][x] = Piece::Dark;
        else if (y >= 5)
          pieces[y][x] = Piece::Light;
      }
    }
  }

  void DrawBoard(Vector2 pos, float size) noexcept {
    DrawRectangle(pos.x, pos.y, size, size, Colors::LightTile);
    const float tile_size = size / 8.f;
    HighlightMouse(pos, tile_size);

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

  void Update(Vector2 board_pos, float board_size) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      HandleClick(board_pos, board_size);
    }
  }
};

void DrawTimer(Vector2 pos, int font_size, float time) {
  const auto time_text =
      TextFormat("%02d:%02d", (int)time / 60, (int)time % 60);
  const int text_len = MeasureText(time_text, font_size);
  DrawText(time_text, pos.x - text_len / 2.f, pos.y, font_size, BLACK);
}

int main() {
  Damas damas;

  InitWindow(800, 800, "Damas");
  SetTargetFPS(60);

  const double time_start = GetTime();
  while (!WindowShouldClose()) {
    const Vector2 board_pos = {100.f, 50.f};
    const float board_size = 600.f;
    damas.Update(board_pos, board_size);
    BeginDrawing();
    ClearBackground(Colors::Background);
    const Vector2 timer_pos =
        board_pos + Vector2{board_size / 2.f, board_size + 20.f};
    DrawTimer(timer_pos, 80, GetTime() - time_start);
    damas.DrawBoard(board_pos, board_size);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}
