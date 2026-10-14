/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2025-2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "Text.h"

#include <algorithm>
#include <cassert>

#include "../util/Log.h"


LOG_USE_TAG("Text")


namespace MINTGGGameEngine
{

Text::Text (
    int32_t x, int32_t y,
    const Font& font,
    uint16_t scaleX, uint16_t scaleY,
    Color color, const std::string& text
)
    : d(std::make_shared<Data>())
{
    d->x = x;
    d->y = y;
    d->font = font;
    d->scaleX = scaleX;
    d->scaleY = scaleY;
    d->explosion = 0;
    d->color = color;
    d->explosionColor = Color::WHITE;
    d->anchor = Anchor::TopLeft;
    d->halign = HAlign::Left;
    d->text = text;
    d->visible = true;
    d->worldSpace = false;
}

void Text::calcTextMetrics(TextMetrics* metrics) const
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

void Text::calcTextBoundingRect (
    int32_t* x, int32_t* y,
    uint32_t* width, uint32_t* height,
    TextMetrics* fromMetrics
) {
    TextMetrics localMetrics;
    if (!fromMetrics) {
        calcTextMetrics(&localMetrics);
        fromMetrics = &localMetrics;
    }

    if (x) {
        *x = getX();
    }
    if (y) {
        *y = getY();
    }
    if (width) {
        *width = fromMetrics->maxGlyphsPerLine * d->scaleX * d->font.getGlyphWidth();
    }
    if (height) {
        *height = fromMetrics->numLines * d->scaleY * d->font.getGlyphHeight();
    }
}

void Text::transformAnchorPosition (
    Anchor newAnchor,
    int32_t* outX, int32_t* outY,
    TextMetrics* metrics
) const {
    if (!d->font  ||  newAnchor == d->anchor) {
        *outX = d->x;
        *outY = d->y;
    } else {
        TextMetrics localMetrics;
        if (!metrics) {
            metrics = &localMetrics;
            calcTextMetrics(metrics);
        }

        const uint16_t effectiveScaleX = d->scaleX;
        const uint16_t effectiveScaleY = d->scaleY;

        // TODO: Handle explosion

        auto w = static_cast<int32_t>(metrics->maxGlyphsPerLine * d->font.getGlyphWidth() * effectiveScaleX);
        auto h = static_cast<int32_t>(metrics->numLines * d->font.getGlyphHeight() * effectiveScaleY);

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
            tlX = d->x;
            tlY = d->y;
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

Text& Text::operator=(const Text& other)
{
    if (&other != this) {
        d = other.d;
    }
    return *this;
}

}
