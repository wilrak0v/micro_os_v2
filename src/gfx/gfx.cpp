#include "gfx.hpp"
#include <LovyanGFX.hpp>
#include <algorithm>

void Renderer::drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    commands.emplace_back(RECTANGLE, x, y, w, h, color, nullptr, Rect{x, y, w, h});
}

void Renderer::fillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    commands.emplace_back(FILLED_RECTANGLE, x, y, w, h, color, nullptr, Rect{x, y, w, h});
}

void Renderer::drawRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) {
    commands.emplace_back(ROUND_RECTANGLE, x, y, w, h, radius, color, Rect{x, y, w, h});
}

void Renderer::fillRoundRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t radius, uint16_t color) {
    commands.emplace_back(FILLED_ROUND_RECTANGLE, x, y, w, h, radius, color, Rect{x, y, w, h});
}

void Renderer::fillCircle(uint16_t cx, uint16_t cy, uint16_t r, uint16_t color) {
    uint16_t d = r * 2;
    uint16_t rx = (cx > r) ? cx - r : 0;
    uint16_t ry = (cy > r) ? cy - r : 0;
    commands.emplace_back(CIRCLE, cx, cy, r, 0, color, Rect{rx, ry, d, d});
}

void Renderer::drawLine(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint16_t color) {
    uint16_t x = (startx < endx) ? startx : endx;
    uint16_t y = (starty < endy) ? starty : endy;
    uint16_t w = std::max(startx, endx) - x + 1;
    uint16_t h = std::max(starty, endy) - y + 1;
    commands.emplace_back(LINE, startx, starty, endx, endy, color, Rect{x, y, w, h});
}

void Renderer::drawText(const char *text, uint16_t x, uint16_t y, uint16_t font_size, uint16_t color) {
    lcd->setTextSize(font_size);
    uint16_t h = lcd->fontHeight();
    uint16_t w = lcd->textWidth(text);
    commands.emplace_back(TEXT, x, y, w, font_size, color, text, Rect{x, y, w, h});
}

void Renderer::drawBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *bitmap) {
    commands.emplace_back(BITMAP, x, y, w, h, bitmap, Rect{x, y, w, h});
}

void Renderer::drawScaledBitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t scale, uint16_t color, const uint16_t *bitmap) {
    uint16_t rectw = static_cast<uint16_t>(w * scale);
    uint16_t recth = static_cast<uint16_t>(h * scale);
    commands.emplace_back(BITMAP_SCALED, x, y, w, h, scale, color, bitmap, Rect{x, y, rectw, recth});
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
                        sprite.drawRect(rel_x, rel_y, cmd.x2, cmd.y2, cmd.color);
                        break;
                    case FILLED_RECTANGLE:
                        sprite.fillRect(rel_x, rel_y, cmd.x2, cmd.y2, cmd.color);
                        break;
                    case ROUND_RECTANGLE:
                        sprite.drawRoundRect(rel_x, rel_y, cmd.x2, cmd.y2, cmd.radius, cmd.color);
                        break;
                    case FILLED_ROUND_RECTANGLE:
                        sprite.fillRoundRect(rel_x, rel_y, cmd.x2, cmd.y2, cmd.radius, cmd.color);
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
                    case BITMAP:
                        sprite.pushImage(rel_x, rel_y, cmd.x2, cmd.y2, (const uint16_t*)cmd.bitmap);
                        break;
                    case BITMAP_SCALED:
                        sprite.pushImageRotateZoom(
                            (float)rel_x, (float)rel_y,
                            0.0f, 0.0f,
                            0.0f,
                            cmd.size, cmd.size,
                            cmd.x2, cmd.y2,
                            (const uint16_t*)cmd.bitmap,
                            cmd.color
                        );
                        break;
                }
            }
        }

        sprite.pushSprite(rect.x, rect.y);

        sprite.deleteSprite();
    }
    commands.clear();
}