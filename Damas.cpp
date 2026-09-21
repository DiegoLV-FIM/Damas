#include "raylib.h"

int main() {
  InitWindow(800, 600, "Hola Mundo");
  while (!WindowShouldClose()) {
    BeginDrawing();
    ClearBackground(DARKGRAY);
    EndDrawing();
  }
  CloseWindow();
  
  return 0;
}
