#pragma once

#include "Globals.h"
#include "Bitmap.h"
#include "Collider.h"
#include "Color.h"
#include "Screen.h"
#include "Sprite.h"
#include "Vec2.h"

#include <memory>


namespace MINTGGGameEngine
{

/**
 * \brief Represents a single object in the game (e.g. player, enemy, bullet).
 *
 * A GameObject can have a Sprite (if it's a graphical object) and a Collider
 * (if it should support collision with other GameObjects).
 *
 * Each object has a position on the screen, specified by an (x,y) coordinate
 * pair (setPosition(), getPosition()). These coordinates don't need to be
 * integers, although they will be rounded to the nearest integer when
 * displaying the object on screen. The position is taken as the top-left corner
 * of the bounding rectangle of the object for drawing and collision purposes.
 *
 * A GameObject can be flipped (setFlipDir()) horizontally or vertically, which
 * applies to both its Sprite and its Collider.
 *
 * A move direction can be set for the GameObject (setMoveDirection()). This
 * alone **does not make the object move**, but merely stores a direction for
 * later use by the caller. It can also be moved in this direction via
 * move(float).
 *
 * GameObjects can be hidden via setVisible(), and the order in which they are
 * drawn on the screen can be changed with setZOrder(). The latter can be
 * important when GameObjects overlap. The sprite and collider can be changed
 * on-the-fly, e.g. for animation purposes (setSprite(), setCollider()).
 *
 * This class uses a shared pointer to store its data. Copying is therefore
 * cheap, and all copies still refer to the same single GameObject.
 *
 * \see Sprite
 * \see Collider
 */
class GameObject
{
private:
    struct Data
    {
        float x;
        float y;
        Vec2 moveDir;
        FlipDir flipDir;
        Sprite sprite;
        Collider collider;
        uint64_t tags;
        uint16_t zOrder; // Higher is in front
        bool visible;
    };

public:
    /// \name Creating common GameObjects
    ///@{

    /**
     * \brief Create a GameObject with a circle as sprite and collider.
     * 
     * \param x x coordinate of the top-left corner of the bounding rectangle
     *      around the circle. It is **not the center coordinate**!
     * \param y y coordinate of the top-left corner of the bounding rectangle
     *      around the circle. It is **not the center coordinate**!
     * \param r Radius of the circle.
     * \param color Color of the circle.
     * \param filled true to fill the circle with its color, false to draw only
     *      the outlines of the circle.
     * \param collider true to give the object a circular collider, false to
     *      give it no collider (e.g. for making purely visual objects without
     *      collision).
     * \return The new GameObject. Game.spawnObject() can be called with it.
     * \see createColliderCircle()
     */
    static GameObject createCircle(float x, float y, float r, const Color& color, bool filled = true, bool collider = true);
    
    /**
     * \brief Create a GameObject with a rectangle as sprite and collider.
     * 
     * \param x x coordinate of the top-left corner of the rectangle.
     * \param y y coordinate of the top-left corner of the rectangle.
     * \param w Width of the rectangle.
     * \param h Height of the rectangle.
     * \param color Color of the circle.
     * \param filled true to fill the rectangle with its color, false to draw only
     *      the outlines of the rectangle.
     * \param collider true to give the object a rectangular collider, false to
     *      give it no collider (e.g. for making purely visual objects without
     *      collision).
     * \return The new GameObject. Game.spawnObject() can be called with it.
     * \see createColliderRect()
     */
    static GameObject createRect(float x, float y, float w, float h, const Color& color, bool filled = true, bool collider = true);
    
    /**
     * \brief Create a GameObject with a Bitmap as sprite and a rectangular
     *      collider.
     * 
     * \param x x coordinate of the top-left corner of the bounding rectangle
     *      around the bitmap.
     * \param y y coordinate of the top-left corner of the bounding rectangle
     *      around the bitmap.
     * \param bitmap The bitmap to use as a sprite.
     * \param collider true to give the object a rectangular collider, false to
     *      give it no collider (e.g. for making purely visual objects without
     *      collision).
     * \return The new GameObject. Game.spawnObject() can be called with it.
     * \see Bitmap
     * \see Sprite
     */
    static GameObject createBitmap(float x, float y, const Bitmap& bitmap, bool collider = true);
    
    
    /**
     * \brief Create a GameObject with a circle as collider and no sprite.
     * 
     * \param x x coordinate of the top-left corner of the bounding rectangle
     *      around the circle. It is **not the center coordinate**!
     * \param y y coordinate of the top-left corner of the bounding rectangle
     *      around the circle. It is **not the center coordinate**!
     * \param r Radius of the circle.
     * \return The new GameObject. Game.spawnObject() can be called with it.
     * \see createCircle()
     */
    static GameObject createColliderCircle(float x, float y, float r);
    
    /**
     * \brief Create a GameObject with a rectangle as collider and no sprite.
     * 
     * \param x x coordinate of the top-left corner of the rectangle.
     * \param y y coordinate of the top-left corner of the rectangle.
     * \param w Width of the rectangle.
     * \param h Height of the rectangle.
     * \return The new GameObject. Game.spawnObject() can be called with it.
     * \see createRect()
     */
    static GameObject createColliderRect(float x, float y, float w, float h);
    
    ///@}

public:
    /**
     * \brief Create a new GameObject.
     *
     * The Sprite and Collider can later be changed with setSprite() and
     * setCollider().
     *
     * \param x x coordinate of the GameObject (top-left corner of the bounding
     *      rectangle).
     * \param y y coordinate of the GameObject (top-left corner of the bounding
     *      rectangle).
     * \param sprite The sprite to use.
     * \param collider The collider to use.
     * \see Sprite
     * \see Collider
     * \see createCircle()
     * \see createRect()
     * \see createBitmap()
     */
    GameObject(float x = 0.0f, float y = 0.0f, const Sprite& sprite = Sprite(), const Collider& collider = Collider());
    
    /**
     * \brief Copy constructor.
     *
     * Note that this class uses shared pointers, so this will **not** create
     * an actual copy in the game. Copying is cheap.
     *
     * \param other The object to copy.
     */
    GameObject(const GameObject& other) : d(other.d) {}


    /// \name Positioning
    ///@{
    
    /**
     * \brief Return the x coordinate of the top-left corner of the bounding
     *      rectangle.
     *
     * \return x coordinate.
     * \see getY()
     * \see getPosition().
     */
    float getX() const { return d->x; }
    
    /**
     * \brief Set the x coordinate of the top-left corner of the bounding
     *      rectangle.
     *
     * \param x x coordinate.
     * \see setY()
     * \see setPosition()
     */
	void setX(float x) { d->x = x; }
    
    /**
     * \brief Return the y coordinate of the top-left corner of the bounding
     *      rectangle.
     *
     * \return y coordinate.
     * \see getX()
     * \see getPosition().
     */
    float getY() const { return d->y; }
    
    /**
     * \brief Set the y coordinate of the top-left corner of the bounding
     *      rectangle.
     *
     * \param y y coordinate.
     * \see setX()
     * \see setPosition()
     */
	void setY(float y) { d->y = y; }
    
    /**
     * \brief Return the coordinates of the top-left corner of the bounding
     *      rectangle.
     *
     * \return Coordinates.
     * \see getX()
     * \see getY()
     */
	Vec2 getPosition() const { return Vec2(getX(), getY()); }
    
    /**
     * \brief Set the position of the top-left corner of the bounding rectangle.
     *
     * \param x x coordinate.
     * \param y y coordinate.
     * \see setX()
     * \see setY()
     */
    void setPosition(float x, float y) { d->x = x; d->y = y; }
    
    /**
     * \brief Set the position of the top-left corner of the bounding rectangle.
     *
     * \param o Coordinates.
     * \see setX()
     * \see setY()
     */
    void setPosition(const Vec2& p) { setPosition(p.x(), p.y()); }
    
    Vec2 getCenterPosition(bool useSprite = false) const;
    
    /**
     * \brief Return the object's movement direction.
     *
     * \return Movement direction. It is always normalized to length 1, i.e.
     *      does not include movement speed.
     * \see setMoveDirection()
     * \see move(float)
     */
    Vec2 getMoveDirection() const { return d->moveDir; }
    
    /**
     * \brief Set the movement direction of the object.
     *
     * Note that just setting this does **not** cause any actual movement in the
     * object. To move the object, a method like move(float) must be manually
     * called as well.
     *
     * \param dir The movement direction. It will automatically be normalized
     *      to length 1, i.e. movement speed can **not** be set with this
     *      method.
     * \see getMoveDirection()
     * \see move(float)
     */
    void setMoveDirection(const Vec2& dir) { d->moveDir = dir.normalized(); }
    
    /**
     * \brief Set the movement direction of the object.
     *
     * \see setMoveDirection(const Vec2& dir)
     */
    void setMoveDirection(float x, float y) { setMoveDirection(Vec2(x, y)); }
    
    /**
     * \brief Move the object by the given amount.
     *
     * This just adds the parameters to the object's current position.
     *
     * \param dx x coordiante offset.
     * \param dy y coordiante offset.
     * \see setPosition()
     */
    void move(float dx, float dy) { setPosition(d->x + dx, d->y + dy); }
    
    /**
     * \brief Move the object by the given amount.
     *
     * This just adds the parameters to the object's current position.
     *
     * \param v Coordinate offset.
     * \see setPosition()
     */
    void move(const Vec2& v) { move(v.x(), v.y()); }
    
    /**
     * \brief Move the object by the given amount along its current movement
     *      direction.
     *
     * This will use the direction defined by setMoveDirection().
     *
     * \param speed The length to move along the movement direction.
     * \see move(const Vec2&)
     * \see setMoveDirection()
     * \see setPosition()
     */
    void move(float speed) { move(d->moveDir * speed); }
    
    bool isLeftOf(const GameObject& other, bool useSprite = false) const;
    bool isRightOf(const GameObject& other, bool useSprite = false) const;
    bool isAbove(const GameObject& other, bool useSprite = false) const;
    bool isBelow(const GameObject& other, bool useSprite = false) const;
    
    float getWidth(bool useSprite = false) const;
    float getHeight(bool useSprite = false) const;
    Vec2 getSize(bool useSprite = false) const
            { return Vec2(getWidth(useSprite), getHeight(useSprite)); }
    
    ///@}
    
    
    /// \name Miscellaneous
    ///@{
    
    /**
     * \brief Return the direction that the object is flipped.
     *
     * \return The flip direction. See FlipDir for possible options.
     */
    FlipDir getFlipDir() const { return d->flipDir; }
    
    /**
     * \brief Sets the axes along which the object should be flipped.
     *
     * This applies to both the Sprite and the Collider.
     *
     * \param flipDir The direction to flip the object.
     */
    void setFlipDir(FlipDir flipDir) { d->flipDir = flipDir; }
    
    /**
     * \brief Return the order in which to draw this object on the screen.
     *
     * The default is ZOrderNormal.
     *
     * \return The Z order.
     */
    uint16_t getZOrder() const { return d->zOrder; }
    
    /**
     * \brief Set the drawing order of this object.
     *
     * See ZOrder for what this is used for, as well as possible values.
     *
     * \param zorder The Z order.
     * \see ZOrder
     */
    void setZOrder(uint16_t zorder = ZOrderNormal) { d->zOrder = zorder; }
    
    /**
     * \brief Return whether the object is currently visible.
     *
     * \return true if visible, false otherwise. Note that a GameObject with no
     *      sprite is not automatically considered invisible.
     */
    bool isVisible() const { return d->visible; }
    
    /**
     * \brief Show or hide the object.
     *
     * This only influences the Sprite, not the Collider.
     *
     * \param visible true if visible, false if hidden.
     */
    void setVisible(bool visible) { d->visible = visible; }
    
    /**
     * \brief Return the object's visual sprite.
     *
     * \return The current sprite.
     * \see Sprite
     */
    Sprite getSprite() const { return d->sprite; }
    
    /**
     * \brief Set the visual sprite for the object.
     *
     * \param sprite The new sprite.
     */
    void setSprite(const Sprite& sprite) { d->sprite = sprite; }
    
    /**
     * \brief Return the object's collider, used for collision checking.
     *
     * The collider is returned in local coordinates, i.e. relative to the
     * GameObject's own position.
     *
     * \return The current collider.
     * \see getWorldCollider()
     * \see Collider
     */
    Collider getCollider() const { return d->collider; }
    
    /**
     * \brief Return the object's collider in world coordinates.
     *
     * Unlike getCollider(), this returns the collider in world coordinates,
     * i.e. with the GameObject's own position and flip direction already
     * baked in.
     *
     * \see getCollider()
     * \see Collider
     */
    Collider getWorldCollider() const;
    
    /**
     * \brief Set the collider for the object.
     *
     * \param collider The new collider.
     */
    void setCollider(const Collider collider) { d->collider = collider; }
    
    /**
     * \brief Draw the object on the given screen.
     *
     * This method **should not be called by the user**. It is automatically
     * called by Game.draw().
     *
     * \param screen The screen to draw on.
     * \param offset Position offset, e.g. to apply camera transform.
     * \see Game.draw()
     */
    void draw(Screen& screen, const Vec2& offset = Vec2()) const;

    /**
     * \brief Check whether this GameObjects collides with another.
     *
     * \param other The GameObject to check against.
     * \param shrink The amount to shrink both colliders before checking. See
     *      Collider class for details.
     * \return true if they collide, false otherwise.
     */
    bool collides(const GameObject& other, float shrink = 0.0f) const;
    
    ///@}
    
    
    
    /// \name Tags
    ///@{
    
    /**
     * \brief Enable the given tag on the object.
     * 
     * \param tag The tag to enable. It must be a bit flag.
     * \return The GameObject itself (for method chaining).
     */
    GameObject& setTag(uint64_t tag) { d->tags |= tag; return *this; }
    
    /**
     * \brief Disable the given tag on the object.
     * 
     * \param tag The tag to disable. It must be a bit flag.
     * \return The GameObject itself (for method chaining).
     */
    GameObject& unsetTag(uint64_t tag) { d->tags &= ~tag; return *this; }
    
    /**
     * \brief Check whether the object has the given tag.
     *
     * It is actually an alias for hasAnyTag().
     *
     * \param tag The tag to check for.
     * \return true if it has the tag, false otherwise.
     * \see hasAllTags()
     * \see hasAnyTags()
     * \see setTag()
     * \see unsetTag()
     */
    bool hasTag(uint64_t tag) const { return hasAnyTags(tag); }
    
    /**
     * \brief Check whether the object has all the given tags.
     *
     * \param tags The tags to check for, in binary OR combination.
     * \return true if it has all of the tags, false otherwise.
     * \see hasTag()
     * \see hasAnyTags()
     * \see setTag()
     * \see unsetTag()
     */
    bool hasAllTags(uint64_t tags) const { return (d->tags & tags) == tags; }
    
    /**
     * \brief Check whether the object has any of the given tags.
     *
     * \param tags The tags to check for, in binary OR combination.
     * \return true if it has any of the tags, false otherwise.
     * \see hasTag()
     * \see hasAllTags()
     * \see setTag()
     * \see unsetTag()
     */
    bool hasAnyTags(uint64_t tags) const { return (d->tags & tags) != 0; }
    
    ///@}
    
    
    /// \name Operators
    ///@{

    /**
     * \brief Assignment operator.
     *
     * Since this class uses a shared pointer, this operation is cheap.
     */
    GameObject& operator=(const GameObject& other) { d = other.d; return *this; };

    /**
     * \brief Checks whether two GameObjects are equal (i.e. the same).
     * 
     * A GameObject is only equal to itself, not to any other GameObjects (even
     * if they have all the same attributes).
     *
     * \param other The GameObject to compare against.
     * \return true if the same, false otherwise.
     */
    bool operator==(const GameObject& other) const { return d == other.d; }
    
    /**
     * \brief Checks whether two GameObjects are not equal (i.e. not the same).
     * 
     * A GameObject is only equal to itself, not to any other GameObjects (even
     * if they have all the same attributes).
     *
     * \param other The GameObject to compare against.
     * \return true if not the same, false otherwise.
     */
    bool operator!=(const GameObject& other) const { return d != other.d; }
    
    /**
     * \brief Does a pointer comparison on two GameObjects.
     */
    bool operator<(const GameObject& other) const { return d < other.d; }
    
    /**
     * \brief Does a pointer comparison on two GameObjects.
     */
    bool operator<=(const GameObject& other) const { return d <= other.d; }
    
    /**
     * \brief Does a pointer comparison on two GameObjects.
     */
    bool operator>(const GameObject& other) const { return d > other.d; }
    
    /**
     * \brief Does a pointer comparison on two GameObjects.
     */
    bool operator>=(const GameObject& other) const { return d >= other.d; }
    
    ///@}

private:
    std::shared_ptr<Data> d;
};

}
