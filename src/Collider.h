#pragma once

#include "Globals.h"
#include "Color.h"


namespace MINTGGGameEngine
{

class Screen;

/**
 * \brief A two-dimensional shape used for collision checking.
 *
 * Currently, only circles and axis-aligned rectangles are supported as
 * collider types, and collision can be checked between any combination of these
 * types. Null colliders are also supported, which never collide with anything.
 *
 * For convenience, a shrink value can be specified in every collision check.
 * This value causes all involved colliders to shrink by the given (possibly
 * fractional) amount of pixels at all of their edges for the purposes of
 * collision calculations. A small shrink can be useful to avoid detecting
 * objects that perfectly touch at their edges to be considered colliding. A
 * negative shrink value can also be used to expand the colliders.
 */
class Collider
{
public:
    enum class Type
    {
        Null,
        Circle,
        Rect
    };

public:
    /**
     * \brief Create a circular collider.
     *
     * \param cx The center x coordinate of the circle.
     * \param cy The center y coordinate of the circle.
     * \param r The radius of the circle.
     * \return The collider.
     */
    static Collider createCircle(float cx, float cy, float r)
    {
        Collider c(Type::Circle);
        c.circle.cx = cx;
        c.circle.cy = cy;
        c.circle.r = r;
        return c;
    }

    /**
     * \brief Create a rectangular collider.
     * 
     * \param x The x coordinate of the top-left corner of the rectangle.
     * \param y The y coordinate of the top-left corner of the rectangle.
     * \param w The width of the rectangle.
     * \param h The height of the rectangle.
     */
    static Collider createRect(float x, float y, float w, float h)
    {
        Collider c(Type::Rect);
        c.rect.x = x;
        c.rect.y = y;
        c.rect.w = w;
        c.rect.h = h;
        return c;
    }

public:
    /**
     * \brief Create a null collider, which never collides with anything.
     */
    Collider() : type(Type::Null) {}
    Collider(const Collider& o);

    /**
     * \brief Convert all coordinates to the world coordinate system.
     *
     * This will apply translation and flip to the collider.
     *
     * \param px The x offset of the collider.
     * \param py The y offset of the collider.
     * \param flip The flip direction (does not matter for the currently
     *      supported collider types).
     */
    Collider toWorld(float px, float py, FlipDir flip) const;
    
    /**
     * \brief Returns a copy of the collider shrunken by the given amount.
     */
    Collider shrunk(float shrink) const;

    /**
     * \brief Check whether this collider collides with another.
     *
     * \param other The other collider.
     * \param shrink The shrink to apply to both colliders.
     */
    bool collides(const Collider& other, float shrink = 0.0f) const;

    /**
     * \brief Draws the outline of the collider to a screen.
     */
    void debugDraw(Screen& screen, const Color& color);

    /**
     * \brief Checks whether this is a valid collider (i.e. not a Null one).
     */
    operator bool() const { return type != Type::Null; }

private:
    Collider(Type type) : type(type) {}

    static bool collidesCircleCircle(const Collider& a, const Collider& b);
    static bool collidesRectRect(const Collider& a, const Collider& b);
    static bool collidesCircleRect(const Collider& c, const Collider& r);

private:
    Type type;
    union {
        struct {
            float cx;
            float cy;
            float r;
        } circle;

        struct {
            float x;
            float y;
            float w;
            float h;
        } rect;
    };
};

}
