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
    for (const auto& rect : dirtyRects) {
        lcd->setClipRect(rect.x, rect.y, rect.w, rect.h);

        lcd->startWrite();

        for (const auto& cmd: commands) {
            if (rect.intersects(cmd.bounds)) {
                switch(cmd.type) {
                    case RECTANGLE:
                        lcd->fillRect(cmd.x1, cmd.y1, cmd.x2, cmd.y2, cmd.color);
                        break;
                    case CIRCLE:
                        lcd->fillCircle(cmd.x1, cmd.y1, cmd.x2, cmd.color);
                        break;
                }
            }
        }

        lcd->endWrite();
    }
    lcd->setClipRect(0, 0, 320, 240);
    commands.clear();
}