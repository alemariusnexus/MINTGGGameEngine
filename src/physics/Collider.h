#pragma once

#include "../Globals.h"
#include "../graphics/Color.h"
#include "../util/Vec2.h"

#include <vector>


namespace MINTGGGameEngine
{

class GameObject;
class RayCastHit;
class DrawSurface;

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
     * \param corner The coordinates of the top-left corner of the circle's bounding rectangle (**not the center!**)
     * \param r The radius of the circle.
     * \return The collider.
     */
    static Collider createCircle(const Vec2& corner, float r)
            { return createCircle(corner.x(), corner.y(), r); }
    
    /**
     * \brief Create a circular collider.
     *
     * \param cornerX x coordinate of the top-lft corner of the circle's bounding rectangle (**not the center!**)
     * \param cornerY y coordinate of the top-lft corner of the circle's bounding rectangle (**not the center!**)
     * \param r The radius of the circle.
     * \return The collider.
     */
    static Collider createCircle(float cornerX, float cornerY, float r)
    {
        Collider c(Type::Circle);
        c.circle.x = cornerX;
        c.circle.y = cornerY;
        c.circle.r = r;
        return c;
    }
            
    /**
     * \brief Create a rectangular collider.
     * 
     * \param corner The coordinates of the top-left corner of the rectangle.
     * \param size The width and height of the rectangle.
     */
    static Collider createRect(const Vec2& corner, const Vec2& size)
            { return createRect(corner.x(), corner.y(), size.x(), size.y()); }

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

    Collider& operator=(const Collider& o);
    
    float getWidth() const;
    float getHeight() const;

    /**
     * \brief Convert all coordinates to the world coordinate system.
     *
     * This will apply the affine transforms to the collider.
     *
     * \param px The x offset of the collider.
     * \param py The y offset of the collider.
     * \param scaleX The x scale factor.
     * \param scaleY The y scale factor.
     * \param rotation The rotation.
     * \param flip The flip direction (does not matter for the currently
     *      supported collider types).
     */
    Collider toWorld(float px, float py, float scaleX, float scaleY, Rotation rotation, FlipDir flip) const;
    
    /**
     * \brief Convert all coordinates to the world coordinate system.
     *
     * This will apply translation and flip to the collider.
     *
     * \param pos The offset of the collider.
     * \param scale The scale factors.
     * \param rotation The rotation.
     * \param flip The flip direction (does not matter for the currently
     *      supported collider types).
     */
    Collider toWorld(const Vec2& pos, const Vec2& scale, Rotation rotation, FlipDir flip) const
            { return toWorld(pos.x(), pos.y(), scale.x(), scale.y(), rotation, flip); }
    
    /**
     * \brief Returns a copy of the collider shrunken by the given amount.
     *
     * Shrinking occurs towards the center of the collider, so shrinking changes its position as well.
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
     * \brief Draws the outline of the collider to a surface.
     *
     * \param offset Offset to draw the collider at. Scale factor will NOT be applied to this.
     */
    void debugDraw (
        DrawSurface& surface,
        const Color& color,
        const Vec2& offset,
        float scaleX, float scaleY
        );

    /**
     * \brief Checks whether this is a valid collider (i.e. not a Null one).
     */
    operator bool() const { return type != Type::Null; }
    
    /**
     * \brief Calculate intersection of a ray with this collider.
     *
     * This is considered an internal method. Use Game::castRay() instead.
     */
    size_t castRay (
            std::vector<RayCastHit>& hits,
            const Vec2& start, const Vec2& direction, float length,
            const GameObject& gameObject
            ) const;

private:
    Collider(Type type) : type(type) {}

    Vec2 getCenterPosition() const;

    static bool collidesCircleCircle(const Collider& a, const Collider& b);
    static bool collidesRectRect(const Collider& a, const Collider& b);
    static bool collidesCircleRect(const Collider& c, const Collider& r);

private:
    Type type;
    union {
        struct {
            float x; // Corner coordinate, NOT center!
            float y; // Corner coordinate, NOT center!
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
