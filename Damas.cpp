#include "raylib.h"
#include "raymath.h"

constexpr Color BACKGROUND = BEIGE;

class Damas {
  static inline Color DARK_TILE = DARKGRAY;
  static inline Color LIGHT_TILE = RAYWHITE;

public:
  Damas() { }

  void Draw(Vector2 pos, float size) noexcept {
    DrawRectangle(pos.x, pos.y, size, size, LIGHT_TILE);
    float step = size / 8.f;
    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
        if ((x + y) % 2 != 0)
          continue;
        Vector2 rpos = Vector2Add(pos, {y * step, x * step});
        DrawRectangle(rpos.x, rpos.y, step, step, DARK_TILE);
      }
    }
    DrawBoardLines(pos, size);
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
  }
  CloseWindow();

  return 0;
}
