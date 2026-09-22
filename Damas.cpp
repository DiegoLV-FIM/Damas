#include "raylib.h"
#include "raymath.h"
#include <optional>

namespace Colors {
const inline Color Background = BEIGE;
const inline Color LightTile = RAYWHITE;
const inline Color DarkTile = {45, 45, 45, 255};
const inline Color Lines = BLACK;
const inline Color LightPiece = DARKBLUE;
const inline Color DarkPiece = MAROON;
const inline Color PieceBorder = BLACK;
const inline Color Hover = Fade(BLACK, 0.30f);
}; // namespace Colors

class Pos {
public:
  int x, y;

  bool operator==(const Pos &other) const {
    return x == other.x && y == other.y;
  }
};

static inline std::optional<Pos> getMouseTile(Vector2 pos,
                                              float tile_size) noexcept {
  const Vector2 mouse = (GetMousePosition() - pos) / tile_size;
  if (mouse.x < 0.f || mouse.x >= 8.f || mouse.y < 0.f || mouse.y >= 8.f)
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
  bool turno_blanco = true;

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
        const Vector2 tile_pos = getTilePos({x, y}, pos, tile_size);
        DrawPiece(pieces[y][x], Vector2AddValue(tile_pos, tile_size * .5f),
                  tile_size * .32f);
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

  void Draw(Vector2 pos, float size) noexcept {
    DrawRectangle(pos.x, pos.y, size, size, Colors::LightTile);
    const float tile_size = size / 8.f;
    HighlightMouse(pos, tile_size);

    for (int y = 0; y < 8; y++) {
      for (int x = y % 2; x < 8; x += 2) {
        DrawTile(pos, {x, y}, tile_size, Colors::DarkTile);
      }
    }

    DrawBoardLines(pos, size, tile_size);
    DrawAllPieces(pos, tile_size);
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
    BeginDrawing();
    ClearBackground(Colors::Background);
    DrawTimer(board_pos + Vector2{board_size / 2.f, board_size + 20.f}, 80,
              GetTime() - time_start);
    damas.Draw(board_pos, board_size);
    EndDrawing();
  }
  CloseWindow();

  return 0;
}
