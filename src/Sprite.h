#pragma once

#include "Globals.h"
#include "Bitmap.h"
#include "Color.h"
#include "Screen.h"

namespace MINTGGGameEngine
{

class Sprite
{
public:
    enum class Type
    {
        Null,
        Rect,
        Circle,
        Bitmap
    };

public:
    static Sprite createRect(int w, int h, const Color& color, bool filled = true);
    static Sprite createCircle(int r, const Color& color, bool filled = true);
    static Sprite createBitmap(const Bitmap& bitmap);

public:
    Sprite() : type(Type::Null) {}
    Sprite(const Sprite& other);
    ~Sprite() {};
    
    uint16_t getWidth() const;
    uint16_t getHeight() const;
    
    Type getType() const { return type; }
    Bitmap getBitmap() const { return (type == Type::Bitmap) ? bitmap : Bitmap(); }

    void draw(Screen& screen, float x, float y, FlipDir flipDir) const;

    Sprite& operator=(const Sprite& other);

private:
    Sprite(Type type) : type(type) {}

public:
    Type type;
    union {
        struct {
            int w;
            int h;
            Color color;
            bool filled;
        } rect;

        struct {
            int r;
            Color color;
            bool filled;
        } circle;
    };
    Bitmap bitmap; // Don't put this in the enum because of it's non-trivial destructor
};

}
