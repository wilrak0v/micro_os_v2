#pragma once

#include <cstdint>
#include <vector>
#include <LovyanGFX.hpp>

enum CommandType {
    RECTANGLE,
    CIRCLE,
    LINE,
    TEXT,
};

struct Rect {
    uint16_t x, y;
    uint16_t w, h;

    bool intersects(const Rect& o) const {
        return !(o.x >= x + w || o.x + o.w <= x || o.y >= y + h || o.y + o.h <= y);
    }

    void merge(const Rect& o) {
        uint16_t x1 = (x < o.x) ? x : o.x;
        uint16_t y1 = (y < o.y) ? y : o.y;

        uint16_t w1 = x + w;
        uint16_t w2 = o.x + o.w;
        uint16_t x2 = (w1 > w2) ? w1 : w2;

        uint16_t h1 = y + h;
        uint16_t h2 = o.y + o.h;
        uint16_t y2 = (h1 > h2) ? h1 : h2;

        x = x1;
        y = y1;
        w = x2 - x1;
        h = y2 - y1;
    }
};

struct GfxCommand {
    CommandType type;
    uint16_t x1, y1, x2, y2;
    uint16_t color;
    const char *text;
    Rect bounds;
};

class Renderer {
private:
    LGFX_Device *lcd;
    std::vector<GfxCommand> commands;
    std::vector<Rect> dirtyRects;
public:
    Renderer(LGFX_Device *lcd) : lcd{lcd} {}

    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    
    void fillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);

    void drawLine(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t color);

    void drawText(const char *text, uint16_t x, uint16_t y, uint16_t font_size, uint16_t color);

    void render();
};