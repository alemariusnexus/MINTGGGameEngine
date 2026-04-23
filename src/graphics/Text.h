#pragma once

#include "../Globals.h"
#include "Color.h"

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
private:
    struct Data
    {
        int16_t x;
        int16_t y;
        int size;
        Color color;
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
    Text(int16_t x = 0, int16_t y = 0, int size = 1, Color color = Color::BLACK, const std::string& text = "");
    Text(const Text& other) : d(other.d) {}
    
    int16_t getX() const { return d->x; }
    int16_t getY() const { return d->y; }
    int getSize() const { return d->size; }
    Color getColor() const { return d->color; }
    std::string getText() const { return d->text; }
    bool isVisible() const { return d->visible; }
    bool isWorldSpace() const { return d->worldSpace; }
    
    void setPosition(int16_t x, int16_t y) { d->x = x; d->y = y; }
    void setSize(int size) { d->size = size; }
    void setColor(const Color& color) { d->color = color; }
    void setText(const std::string& text) { d->text = text; }
#ifdef ARDUINO
    void setText(const String& text) { setText(std::string(text.c_str())); }
#endif
    void setVisible(bool visible) { d->visible = visible; }
    void setWorldSpace(bool worldSpace) { d->worldSpace = worldSpace; }
    
    bool operator==(const Text& other) const { return d == other.d; }
    bool operator!=(const Text& other) const { return d != other.d; }

private:
    std::shared_ptr<Data> d;
};

}
