#include <raylib.h>

namespace Editor {
    void Render(Vector2 window) {
        DrawRectangle(window.x / 1.135f, 10, window.x / 8.8f, window.y - 20, (Color){50, 50, 50, 100});
    }
};