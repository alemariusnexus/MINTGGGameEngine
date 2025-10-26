#pragma once

#include "Globals.h"
#include "Color.h"

#include <memory>


namespace MINTGGGameEngine
{

class Text
{
private:
    struct Data
    {
        int16_t x;
        int16_t y;
        int size;
        Color color;
        String text;
        bool visible;
    };
    
public:
    Text(int16_t x = 0, int16_t y = 0, int size = 1, Color color = Color::BLACK, String text = "");
    Text(const Text& other) : d(other.d) {}
    
    int16_t getX() const { return d->x; }
    int16_t getY() const { return d->y; }
    int getSize() const { return d->size; }
    Color getColor() const { return d->color; }
    String getText() const { return d->text; }
    bool isVisible() const { return d->visible; }
    
    void setPosition(int16_t x, int16_t y) { d->x = x; d->y = y; }
    void setSize(int size) { d->size = size; }
    void setColor(const Color& color) { d->color = color; }
    void setText(const String& text) { d->text = text; }
    void setVisible(bool visible) { d->visible = visible; }
    
    bool operator==(const Text& other) const { return d == other.d; }
    bool operator!=(const Text& other) const { return d != other.d; }

private:
    std::shared_ptr<Data> d;
};

}
