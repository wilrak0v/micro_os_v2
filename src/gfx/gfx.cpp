#include "gfx.hpp"
#include <LovyanGFX.hpp>
#include <algorithm>

void Renderer::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    commands.push_back({RECTANGLE, x, y, w, h, color, nullptr, {x, y, w, h}});
}

void Renderer::fillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color) {
    uint16_t d = r * 2;
    uint16_t rx = (cx > r) ? cx - r : 0;
    uint16_t ry = (cy > r) ? cy - r : 0;
    GfxCommand command = {CIRCLE, cx, cy, r, 0, color, nullptr, {rx, ry, d, d}};
    commands.push_back(command);
}

void Renderer::drawLine(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t color) {
    uint16_t x = (startx < endx) ? startx : endx;
    uint16_t y = (starty < endy) ? starty : endy;
    uint16_t w = std::max(startx, endx) - x + 1;
    uint16_t h = std::max(starty, endy) - y + 1;
    GfxCommand command = {LINE, startx, starty, endx, endy, color, nullptr, {x, y, w, h}};
    commands.push_back(command);
}

void Renderer::drawText(const char *text, uint16_t x, uint16_t y, uint16_t font_size, uint16_t color) {
    lcd->setTextSize(font_size);
    uint16_t h = lcd->fontHeight();
    uint16_t w = lcd->textWidth(text);
    GfxCommand command = {TEXT, x, y, w, font_size, color, text, {x, y, w, h}};
    commands.push_back(command);
}

void Renderer::render() {
    if (commands.empty()) return;

    dirtyRects.clear();

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

    if (dirtyRects.size() > 1) {
        for (size_t i = 0; i < dirtyRects.size(); ++i) {
            for (size_t j = i + 1; j < dirtyRects.size(); ) {
                if (dirtyRects[i].intersects(dirtyRects[j])) {
                    dirtyRects[i].merge(dirtyRects[j]);
                    dirtyRects.erase(dirtyRects.begin() + j);
                } else {
                    ++j;
                }
            }
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
                    case LINE:
                        sprite.drawLine(cmd.x1 - rect.x, cmd.y1 - rect.y, cmd.x2 - rect.x, cmd.y2 - rect.y, cmd.color);
                        break;
                    case TEXT:
                        sprite.setTextColor(cmd.color);
                        sprite.setTextSize(cmd.y2);
                        sprite.drawString(cmd.text, rel_x, rel_y);
                        break;
                }
            }
        }

        sprite.pushSprite(rect.x, rect.y);

        sprite.deleteSprite();
    }
    commands.clear();
}