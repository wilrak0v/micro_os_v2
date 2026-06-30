#include "gfx.hpp"
#include <LovyanGFX.hpp>

void Renderer::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    commands.push_back({RECTANGLE, x, y, w, h, color, nullptr, {x, y, w, h}});
}

void Renderer::fillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color) {
    uint16_t d = r * 2;
    GfxCommand command = {CIRCLE, cx, cy, r, 0, color, nullptr, {cx - r, cy - r, d, d}};
    commands.push_back(command);
}

void Renderer::drawText(const char *text, uint16_t x, uint16_t y, uint16_t font_size, uint16_t color) {
    uint16_t h = lcd->fontHeight();
    uint16_t w = lcd->textWidth(text);
    GfxCommand command = {TEXT, x, y, w, h, color, text, {x, y, w, h}};
    commands.push_back(command);
}

void Renderer::render() {
    if (commands.empty()) return;

    std::vector<Rect> dirtyRects;

    // Compute the rects
    for (const auto& cmd : commands) {
        bool merged = false;
        for (auto& dr : dirtyRects) {
            if (dr.intersects(cmd.bounds)) {
                dr.merge(cmd.bounds);
                merged = true;
                break;
            }
        }

        if (!merged) {
            dirtyRects.push_back(cmd.bounds);
        }
    }

    // Render the rects
    lgfx::LGFX_Sprite sprite(lcd);
    for (const auto& rect : dirtyRects) {
        if (!sprite.createSprite(rect.w, rect.h)) continue;

        for (const auto& cmd: commands) {
            if (rect.intersects(cmd.bounds)) {
                int16_t rel_x = cmd.x1 - rect.x;
                int16_t rel_y = cmd.y1 - rect.y;
                switch(cmd.type) {
                    case RECTANGLE:
                        sprite.fillRect(rel_x, rel_y, cmd.x2, cmd.y2, cmd.color);
                        break;
                    case CIRCLE:
                        sprite.fillCircle(rel_x, rel_y, cmd.x2, cmd.color);
                        break;
                }
            }
        }

        sprite.pushSprite(rect.x, rect.y);

        sprite.deleteSprite();
    }
    commands.clear();
}