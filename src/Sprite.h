#pragma once

#include "Globals.h"
#include "Bitmap.h"
#include "Color.h"
#include "Screen.h"

namespace MINTGGGameEngine
{

/**
 * \brief The visual representation of a GameObject.
 *
 * Currently, four types of sprites are supported.
 *
 * The Null type is an invisible sprite. This is useful is an object should
 * function purely as an invisible collider.
 *
 * The Rect type is an axis-aligned rectangle with a solid color.
 *
 * The Circle type is a circle with a solid color.
 *
 * The Bitmap type is a rectangular bitmap, i.e. an array of color pixel values.
 *
 * \see Bitmap
 */
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
    /**
     * \brief Create a Null sprite, i.e. one that is invisible.
     */
    Sprite() : type(Type::Null) {}
    Sprite(const Sprite& other);
    ~Sprite() {};
    
    /**
     * \brief Return the width of the sprite in pixels.
     */
    uint16_t getWidth() const;
    
    /**
     * \brief Return the height of the sprite in pixels.
     */
    uint16_t getHeight() const;
    
    /**
     * \brief Return the type of sprite.
     */
    Type getType() const { return type; }
    
    /**
     * \brief Return the bitmap behind the sprite.
     *
     * Note that sprites of types other than Bitmap will return an invalid
     * bitmap for this method.
     *
     * \return The bitmap, or an invalid bitmap for non-Bitmap type sprites.
     */
    Bitmap getBitmap() const { return (type == Type::Bitmap) ? bitmap : Bitmap(); }

    /**
     * \brief Draw the given bitmap on a screen.
     */
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
