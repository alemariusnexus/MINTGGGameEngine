#include "Sprite.h"


namespace MINTGGGameEngine
{
    
Sprite::Sprite(const Sprite& other)
        : type(other.type)
{
    if (type == Type::Null) {
        // Nothing to do
    } else if (type == Type::Rect) {
        rect.w = other.rect.w;
        rect.h = other.rect.h;
        rect.color = other.rect.color;
        rect.filled = other.rect.filled;
    } else if (type == Type::Circle) {
        circle.r = other.circle.r;
        circle.color = other.circle.color;
        circle.filled = other.circle.filled;
    } else if (type == Type::Bitmap) {
        bitmap = other.bitmap;
    } else {
        assert(false);
    }
}

Sprite Sprite::createRect(int w, int h, const Color& color, bool filled)
{
    Sprite s(Type::Rect);
    s.rect.w = w;
    s.rect.h = h;
    s.rect.color = color;
    s.rect.filled = filled;
    return s;
}

Sprite Sprite::createCircle(int r, const Color& color, bool filled)
{
    Sprite s(Type::Circle);
    s.circle.r = r;
    s.circle.color = color;
    s.circle.filled = filled;
    return s;
}

Sprite Sprite::createBitmap(const Bitmap& bitmap)
{
    Sprite s(Type::Bitmap);
    s.bitmap = bitmap;
    return s;
}

uint16_t Sprite::getWidth() const
{
    if (type == Type::Rect) {
        return rect.w;
    } else if (type == Type::Circle) {
        return 2*circle.r;
    } else if (type == Type::Bitmap) {
        return bitmap.getWidth();
    }
    return 0;
}

uint16_t Sprite::getHeight() const
{
    if (type == Type::Rect) {
        return rect.h;
    } else if (type == Type::Circle) {
        return 2*circle.r;
    } else if (type == Type::Bitmap) {
        return bitmap.getHeight();
    }
    return 0;
}

void Sprite::draw(Screen& screen, float x, float y, FlipDir flipDir) const
{
    if (type == Type::Rect) {
        screen.drawRect(roundf(x), roundf(y), rect.w, rect.h, rect.color, rect.filled);
    } else if (type == Type::Circle) {
        float cx = x + circle.r;
        float cy = y + circle.r;
        screen.drawCircle(roundf(cx), roundf(cy), circle.r, circle.color, circle.filled);
    } else if (type == Type::Bitmap) {
        if (bitmap) {
            screen.drawBitmap(roundf(x), roundf(y), bitmap, flipDir);
        }
    }
}

Sprite& Sprite::operator=(const Sprite& other)
{
    type = other.type;
    if (other.type == Type::Null) {
        // Nothing to do
    } else if (type == Type::Rect) {
        rect.w = other.rect.w;
        rect.h = other.rect.h;
        rect.color = other.rect.color;
        rect.filled = other.rect.filled;
    } else if (type == Type::Circle) {
        circle.r = other.circle.r;
        circle.color = other.circle.color;
        circle.filled = other.circle.filled;
    } else if (type == Type::Bitmap) {
        bitmap = other.bitmap;
    } else {
        assert(false);
    }
    return *this;
}

}
