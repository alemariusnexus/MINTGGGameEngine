#include "Text.h"

#include <algorithm>


namespace MINTGGGameEngine
{

Text::Text (
    int32_t x, int32_t y,
    const Font& font,
    uint16_t scaleFactor,
    Color color, const std::string& text
)
    : d(std::make_shared<Data>())
{
    d->x = x;
    d->y = y;
    d->font = font;
    d->scaleFactor = scaleFactor;
    d->color = color;
    d->anchor = Anchor::TopLeft;
    d->halign = HAlign::Left;
    d->text = text;
    d->visible = true;
    d->worldSpace = false;
}

void Text::getTextMetrics(TextMetrics* metrics) const
{
    if (d->text.empty()) {
        metrics->numLines = 0;
        metrics->maxGlyphsPerLine = 0;
        return;
    }

    size_t numLines = 1;
    size_t maxGlyphsPerLine = 0;

    size_t curGlyphsPerLine = 0;
    for (char c : d->text) {
        if (c == '\n') {
            numLines++;
            maxGlyphsPerLine = std::max(maxGlyphsPerLine, curGlyphsPerLine);
            curGlyphsPerLine = 0;
        } else {
            curGlyphsPerLine++;
        }
    }
    maxGlyphsPerLine = std::max(maxGlyphsPerLine, curGlyphsPerLine);

    metrics->numLines = numLines;
    metrics->maxGlyphsPerLine = maxGlyphsPerLine;
}

void Text::transformAnchorPosition (
    Anchor newAnchor,
    int32_t* outX, int32_t* outY,
    TextMetrics* metrics
) const {
    if (newAnchor == d->anchor) {
        *outX = d->x;
        *outY = d->y;
    } else {
        TextMetrics localMetrics;
        if (!metrics) {
            metrics = &localMetrics;
            getTextMetrics(metrics);
        }

        int32_t w = metrics->maxGlyphsPerLine * d->font.getGlyphWidth() * d->scaleFactor;
        int32_t h = metrics->numLines * d->font.getGlyphHeight() * d->scaleFactor;

        // First convert to top-left position
        int32_t tlX, tlY;
        switch (d->anchor) {
        case Anchor::TopLeft:
            tlX = d->x;
            tlY = d->y;
            break;
        case Anchor::TopCenter:
            tlX = d->x - w/2;
            tlY = d->y;
            break;
        case Anchor::TopRight:
            tlX = d->x - w;
            tlY = d->y;
            break;

        case Anchor::CenterLeft:
            tlX = d->x;
            tlY = d->y - h/2;
            break;
        case Anchor::CenterCenter:
            tlX = d->x - w/2;
            tlY = d->y - h/2;
            break;
        case Anchor::CenterRight:
            tlX = d->x - w;
            tlY = d->y - h/2;
            break;

        case Anchor::BottomLeft:
            tlX = d->x;
            tlY = d->y - h;
            break;
        case Anchor::BottomCenter:
            tlX = d->x - w/2;
            tlY = d->y - h;
            break;
        case Anchor::BottomRight:
            tlX = d->x - w;
            tlY = d->y - h;
            break;

        default:
            assert(false);
            *outX = d->x;
            *outY = d->y;
        }

        // Then convert to new position
        switch (newAnchor) {
        case Anchor::TopLeft:
            *outX = tlX;
            *outY = tlY;
            break;
        case Anchor::TopCenter:
            *outX = tlX + w/2;
            *outY = tlY;
            break;
        case Anchor::TopRight:
            *outX = tlX + w;
            *outY = tlY;
            break;

        case Anchor::CenterLeft:
            *outX = tlX;
            *outY = tlY + h/2;
            break;
        case Anchor::CenterCenter:
            *outX = tlX + w/2;
            *outY = tlY + h/2;
            break;
        case Anchor::CenterRight:
            *outX = tlX + w;
            *outY = tlY + h/2;
            break;

        case Anchor::BottomLeft:
            *outX = tlX;
            *outY = tlY + h;
            break;
        case Anchor::BottomCenter:
            *outX = tlX + w/2;
            *outY = tlY + h;
            break;
        case Anchor::BottomRight:
            *outX = tlX + w;
            *outY = tlY + h;
            break;

        default:
            assert(false);
            *outX = d->x;
            *outY = d->y;
        }
    }
}

}
