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

  class Pos {
  public:
    int x, y;

    Pos(int _x, int _y) : x{_x}, y{_y} {};
    Pos(const Vector2 &vec) : x{(int)vec.x}, y{(int)vec.y} {}
    Pos(const Vector2 &&vec) : x{(int)vec.x}, y{(int)vec.y} {}

    bool operator==(Pos &other) { return x == other.x && y == other.y; }
  };

  Tile tablero[8][8];
  bool turno_blanco = true;

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

  static inline Vector2 getTilePos(Pos tile, Vector2 pos, float size) noexcept {
    return pos + Vector2{(float)tile.x, (float)tile.y} * size;
  }

  static inline void HighlightMouse(Vector2 pos, float step) noexcept {
    const Vector2 mouse = (GetMousePosition() - pos) / step;
    if (Vector2Clamp(mouse, Vector2Zero(), {7.f, 7.f}) == mouse) {
      const Vector2 mouse_tile = getTilePos(mouse, pos, step);
      DrawRectangle(mouse_tile.x, mouse_tile.y, step, step,
                    ColorBrightness(LIGHT_TILE, -0.3f));
    }
  }

  static inline void DrawDarkTiles(Vector2 pos, float step) noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = y % 2; x < 8; x += 2) {
        const Vector2 tile_pos = getTilePos({x, y}, pos, step);
        DrawRectangle(tile_pos.x, tile_pos.y, step, step, DARK_TILE);
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
    DrawCircleV(center, radius, BLACK);
    DrawCircleV(center, radius - 4.f, color);
  }

  void DrawTiles(Vector2 pos, float step) noexcept {
    for (int y = 0; y < 8; y++) {
      for (int x = 1 - (y % 2); x < 8; x += 2) {
        const Vector2 tile_pos = getTilePos({x, y}, pos, step);
        DrawTile(tablero[y][x], Vector2AddValue(tile_pos, step * .5f),
                 step * .32f);
      }
    }
  }

  void Draw(Vector2 pos, float size) noexcept {
    DrawRectangle(pos.x, pos.y, size, size, LIGHT_TILE);
    const float step = size / 8.f;

    HighlightMouse(pos, step);
    DrawDarkTiles(pos, step);
    DrawBoardLines(pos, size);
    DrawTiles(pos, step);
  }

  static void DrawBoardLines(Vector2 pos, float size) noexcept {
    constexpr float thick = 4.f;
    constexpr float half_thick = thick / 2;
    const float step = size / 8.f;

    const Vector2 end = Vector2AddValue(pos, half_thick + size);
    for (int i = 0; i <= 8; i++) {
      const Vector2 off = Vector2AddValue(pos, step * i);
      DrawLineEx({off.x, pos.y}, {off.x, end.y}, thick, BLACK);
      DrawLineEx({pos.x, off.y}, {end.x, off.y}, thick, BLACK);
    };
  }
};

int main() {
  Damas damas;

  InitWindow(800, 700, "Damas");
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
