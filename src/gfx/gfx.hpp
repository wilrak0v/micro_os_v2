#pragma once

#include <cstdint>
#include <vector>
#include <LovyanGFX.hpp>

enum CommandType {
    RECTANGLE,
    FILLED_RECTANGLE,
    ROUND_RECTANGLE,
    FILLED_ROUND_RECTANGLE,

    CIRCLE,
    LINE,
    TEXT,
    BITMAP,
    BITMAP_SCALED,
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
    uint16_t size;
    union {
        const uint16_t *bitmap;
        const char *text;
        uint16_t radius;
    };
    Rect bounds;

    GfxCommand(CommandType t, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color, Rect b)
        : type(t), x1(x1), y1(y1), x2(x2), y2(y2), color(color), radius(0), bounds(b) {}

    GfxCommand(CommandType t, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t radius, uint16_t color, Rect b)
        : type(t), x1(x1), y1(y1), x2(x2), y2(y2), color(color), radius(radius), bounds(b) {}

    GfxCommand(CommandType t, uint16_t x, uint16_t y, uint16_t font_w, uint16_t font_size, uint16_t color, const char* text, Rect b)
        : type{t}, x1{x}, y1{y}, x2{font_w}, y2{font_size}, color{color}, text{text}, bounds{b} {}

    GfxCommand(CommandType t, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* bitmap, Rect b)
        : type{t}, x1{x}, y1{y}, x2{w}, y2{h}, color{0}, bitmap{bitmap}, bounds{b} {}

    GfxCommand(CommandType t, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t scale, uint16_t color, const uint16_t* bitmap, Rect b)
        : type{t}, x1{x}, y1{y}, x2{w}, y2{h}, size{scale}, color{color}, bitmap{bitmap}, bounds{b} {}
};

class Renderer {
private:
    LGFX_Device *lcd;
    std::vector<GfxCommand> commands;
    std::vector<Rect> dirtyRects;
public:
    Renderer(LGFX_Device *lcd) : lcd{lcd} {}

    void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
    void fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color);
    
    void fillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color);

    void drawLine(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t color);

    void drawText(const char *text, uint16_t x, uint16_t y, uint16_t font_size, uint16_t color);

    void drawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap);
    void drawScaledBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t scale, uint16_t color, const uint16_t *bitmap);


    void render();
};