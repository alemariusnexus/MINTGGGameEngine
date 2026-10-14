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

#pragma once

#include "../Globals.h"
#include "Color.h"
#include "Font.h"

#include <memory>
#include <string>


namespace MINTGGGameEngine
{

/**
 * \brief Represents text visible on the screen.
 *
 * Text is defined by its content, position, size and color.
 *
 * Text rendering is currently not well defined, and thus very dependent on the
 * actual rendering library used in the background.
 */
class Text
{
public:
    enum class Anchor
    {
        TopLeft,
        TopCenter,
        TopRight,

        CenterLeft,
        CenterCenter,
        CenterRight,

        BottomLeft,
        BottomCenter,
        BottomRight
    };

    enum class HAlign
    {
        Left,
        Center,
        Right
    };

    struct TextMetrics
    {
        size_t numLines;
        size_t maxGlyphsPerLine;
    };

private:
    struct Data
    {
        int32_t x;
        int32_t y;
        Font font;
        uint16_t scaleX;
        uint16_t scaleY;
        uint8_t explosion;
        Color color;
        Color explosionColor;
        Anchor anchor;
        HAlign halign;
        std::string text;
        bool visible;
        bool worldSpace;
    };
    
public:
    /**
     * \brief Create a new text object.
     * 
     * \param x The x position of the top-left corner of the text's bounding
     *      rectangle.
     * \param y The y position of the top-left corner of the text's bounding
     *      rectangle.
     * \param size The size of the text.
     * \param color The text color.
     * \param text The text content.
     */
    Text (
        int32_t x = 0, int32_t y = 0,
        const Font& font = Font(),
        uint16_t scaleX = 1, uint16_t scaleY = 1,
        Color color = Color::BLACK,
        const std::string& text = ""
        );
    Text(const Text& other) : d(other.d) {}
    
    int32_t getX() const { return d->x; }
    int32_t getY() const { return d->y; }
    const Font& getFont() const { return d->font; }
    uint16_t getScaleX() const { return d->scaleX; }
    uint16_t getScaleY() const { return d->scaleY; }
    [[deprecated("Use getScaleX()/getScaleY() instead")]] uint16_t getScaleFactor() const { return getScaleX(); }
    uint8_t getExplosion() const { return d->explosion; }
    const Color& getColor() const { return d->color; }
    const Color& getExplosionColor() const { return d->color; }
    Anchor getAnchor() const { return d->anchor; }
    HAlign getHAlign() const { return d->halign; }
    const std::string& getText() const { return d->text; }
    bool isVisible() const { return d->visible; }
    bool isWorldSpace() const { return d->worldSpace; }
    
    Text& setPosition(int32_t x, int32_t y) { d->x = x; d->y = y; return *this; }
    Text& setFont(const Font& font) { d->font = font; return *this; }
    Text& setScale(uint16_t scaleX, uint16_t scaleY) { d->scaleX = scaleX; d->scaleY = scaleY; return *this; }
    Text& setScale(uint16_t scale) { setScale(scale, scale); return *this; }
    [[deprecated("Use setScale() instead")]] Text& setScaleFactor(uint16_t scale) { setScale(scale); return *this; }
    Text& setExplosion(uint8_t explosion) { d->explosion = explosion; return *this; }
    Text& setColor(const Color& color) { d->color = color; return *this; }
    Text& setExplosionColor(const Color& color) { d->explosionColor = color; return *this; }
    Text& setAnchor(Anchor anchor) { d->anchor = anchor; return *this; }
    Text& setHAlign(HAlign halign) { d->halign = halign; return *this; }
    Text& setText(const std::string& text) { d->text = text; return *this; }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    [[deprecated]] Text& setArduinoText(const String& text) { setText(std::string(text.c_str())); return *this; }
#endif
    Text& setVisible(bool visible) { d->visible = visible; return *this; }
    Text& setWorldSpace(bool worldSpace) { d->worldSpace = worldSpace; return *this; }

    void calcTextMetrics(TextMetrics* metrics) const;
    void calcTextBoundingRect (
        int32_t* x, int32_t* y,
        uint32_t* width, uint32_t* height,
        TextMetrics* fromMetrics = nullptr
        );

    void transformAnchorPosition (
        Anchor newAnchor,
        int32_t* outX, int32_t* outY,
        TextMetrics* metrics = nullptr
        ) const;

    Text& operator=(const Text& other);
    
    bool operator==(const Text& other) const { return d == other.d; }
    bool operator!=(const Text& other) const { return d != other.d; }

private:
    std::shared_ptr<Data> d;
};

}
