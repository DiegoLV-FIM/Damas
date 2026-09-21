#include "raylib.h"
#include "raymath.h"

constexpr Color BACKGROUND = BEIGE;

class Damas {
  static inline Color DARK_TILE = DARKGRAY;
  static inline Color LIGHT_TILE = RAYWHITE;

  enum class Tile {
    Empty,
    Light,
    QLight,
    Dark,
    QDark,
  };

  Tile tablero[8][8];

public:
  Damas() noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        tablero[y][x] = Tile::Empty;
        if ((x + y) % 2 == 1) {
          if (y < 3)
            tablero[y][x] = Tile::Dark;
          if (y >= 5)
            tablero[y][x] = Tile::Light;
        }
      }
    }
  }

  void Draw(Vector2 pos, float size) noexcept {
    DrawRectangle(pos.x, pos.y, size, size, LIGHT_TILE);
    float step = size / 8.f;
    for (int y = 0; y < 8; y++) {
      for (int x = y % 2; x < 8; x += 2) {
        Vector2 rpos =
            Vector2Add(pos, Vector2Scale({(float)x, (float)y}, step));
        DrawRectangle(rpos.x, rpos.y, step, step, DARK_TILE);
      }
    }

    DrawBoardLines(pos, size);

    for (int y = 0; y < 8; y++) {
      for (int x = 1 - (y % 2); x < 8; x += 2) {
        Vector2 rpos =
            Vector2Add(pos, Vector2Scale({(float)x, (float)y}, step));

        DrawTile(tablero[y][x], Vector2AddValue(rpos, step * .5f), step * .32f);
      }
    }
  }

  static void DrawTile(Tile tile, Vector2 center, float radius) noexcept {
    Color color;
    switch (tile) {
    case Tile::Empty:
      return;
    case Tile::Dark:
      color = MAROON;
      break;
    case Tile::Light:
      color = DARKBLUE;
      break;
    default:
      break;
    }
    DrawCircleV(center, radius, color);
  }

  static void DrawBoardLines(Vector2 pos, float size) noexcept {
    constexpr float thick = 4.f;
    constexpr float half_thick = thick / 2;
    float step = size / 8.f;

    Vector2 end = Vector2AddValue(pos, half_thick + size);
    for (int i = 0; i <= 8; i++) {
      Vector2 off = Vector2AddValue(pos, step * i);
      DrawLineEx({off.x, pos.y}, {off.x, end.y}, thick, BLACK);
      DrawLineEx({pos.x, off.y}, {end.x, off.y}, thick, BLACK);
    };
  }
};

int main() {
  Damas damas;

  InitWindow(800, 700, "Hola Mundo");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(BACKGROUND);
    damas.Draw(Vector2{100, 50}, 600);
    EndDrawing();
    // break;
  }
  CloseWindow();

  return 0;
}
