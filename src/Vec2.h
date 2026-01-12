#pragma once

#include "Globals.h"


namespace MINTGGGameEngine
{

/**
 * \brief A two-dimensional vector, used for e.g. positions or directions.
 *
 * It consists of an x and y coordinate (both real numbers). It can be used
 * to specify a point, as well as a direction or offset in 2D space.
 */
class Vec2
{
public:
    Vec2() : px(0), py(0) {}
    Vec2(const Vec2& o) : px(o.px), py(o.py) {}
    Vec2(float x, float y) : px(x), py(y) {}
    
    /// \name Coordinate access
    ///@{
    
    float x() const { return px; }
    float y() const { return py; }
    float operator[](size_t idx) const { return idx == 0 ? px : py; }
    
    void setX(float x) { px = x; }
    void setY(float y) { py = y; }
    void set(float x, float y) { px = x; py = y; }
    
    ///@}
    
    
    /// \name Operators
    ///@{
    
    Vec2& operator=(const Vec2& o) { px = o.px; py = o.py; return *this; }
    Vec2& operator+=(const Vec2& o) { px += o.px; py += o.py; return *this; }
    Vec2& operator-=(const Vec2& o) { px -= o.px; py -= o.py; return *this; }
    Vec2& operator*=(float s) { px *= s; py *= s; return *this; }
    Vec2& operator/=(float s) { return (*this *= 1.0f/s); }
    
    const Vec2 operator+(const Vec2& o) const { return Vec2(*this) += o; }
    const Vec2 operator-(const Vec2& o) const { return Vec2(*this) -= o; }
    const Vec2 operator*(float s) const { return Vec2(*this) *= s; }
    const Vec2 operator/(float s) const { return Vec2(*this) /= s; }
    const Vec2 operator-() const { return Vec2(-px, -py); }
    
    ///@}
    
    
    /// \name Other operations
    ///@{
    
    /**
     * \brief Calculate the dot product (alias scalar product) of two vectors.
     */
    float dot(const Vec2& o) const { return px*o.px + py*o.py; }
    
    /**
     * \brief Calculate the square of the length of this vector.
     */
    float lengthSq() const { return px*px + py*py; }
    
    /**
     * \brief Calculate the length of this vector.
     */
    float length() const { return sqrtf(lengthSq()); }
    
    /**
     * \brief Normalize this vector, i.e. make it length 1 without changing
     *      its direction.
     *
     * \return The vector's length prior to normalization.
     * \see normalized()
     */
    float normalize() { float l = length(); if (l < 1e-6f) return l; float il = 1.0f/l; px *= il; py *= il; return l; }
    
    /**
     * \brief Return a normalized version of this vector, i.e. one with length
     *      1, but with the same direction.
     *
     * Note that this method does not change the original vector, but returns
     * the normalized version as a copy.
     *
     * \return The normalized veftor.
     * \see normalize()
     */
    const Vec2 normalized() const { Vec2 r(*this); r.normalize(); return r; }
    
    float angle(const Vec2& o) const;
    
    ///@}
    
private:
    float px, py;
};

}
