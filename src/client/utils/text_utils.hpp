#pragma once
#include <raylib.h>

namespace utils {

inline void DrawTextCentered(const char *text, float cx, float y, int fontSize, Color color) {
    int w = MeasureText(text, fontSize);
    DrawText(text, (int)(cx - w * 0.5f), (int)y, fontSize, color);
}

} // namespace utils
