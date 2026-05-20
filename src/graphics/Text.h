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
        uint16_t scaleFactor;
        Color color;
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
        uint16_t scaleFactor = 1,
        Color color = Color::BLACK,
        const std::string& text = ""
        );
    Text(const Text& other) : d(other.d) {}
    
    int32_t getX() const { return d->x; }
    int32_t getY() const { return d->y; }
    const Font& getFont() const { return d->font; }
    uint16_t getScaleFactor() const { return d->scaleFactor; }
    const Color& getColor() const { return d->color; }
    Anchor getAnchor() const { return d->anchor; }
    HAlign getHAlign() const { return d->halign; }
    const std::string& getText() const { return d->text; }
    bool isVisible() const { return d->visible; }
    bool isWorldSpace() const { return d->worldSpace; }
    
    void setPosition(int32_t x, int32_t y) { d->x = x; d->y = y; }
    void setFont(const Font& font) { d->font = font; }
    void setScaleFactor(uint16_t scaleFactor) { d->scaleFactor = scaleFactor; }
    void setColor(const Color& color) { d->color = color; }
    void setAnchor(Anchor anchor) { d->anchor = anchor; }
    void setHAlign(HAlign halign) { d->halign = halign; }
    void setText(const std::string& text) { d->text = text; }
#ifdef MINTGGGAMEENGINE_PORT_ARDUINO
    void setText(const String& text) { setText(std::string(text.c_str())); }
#endif
    void setVisible(bool visible) { d->visible = visible; }
    void setWorldSpace(bool worldSpace) { d->worldSpace = worldSpace; }

    void getTextMetrics(TextMetrics* metrics) const;

    void transformAnchorPosition (
        Anchor newAnchor,
        int32_t* outX, int32_t* outY,
        TextMetrics* metrics = nullptr
        ) const;
    
    bool operator==(const Text& other) const { return d == other.d; }
    bool operator!=(const Text& other) const { return d != other.d; }

private:
    std::shared_ptr<Data> d;
};

}
