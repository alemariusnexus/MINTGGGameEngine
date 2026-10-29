/*
 * This file is part of MINTGGGameEngine.
 *
 * Copyright 2026 David Lerch
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "../Globals.h"

#include <cmath>

#include "Vec2.h"


namespace MINTGGGameEngine
{

/**
 * \brief A three-dimensional vector, used for e.g. positions or directions.
 *
 * It consists of x, y and z coordinates (all real numbers). It can be used
 * to specify a point, as well as a direction or offset in 3D space.
 */
class Vec3
{
public:
    Vec3() : px(0), py(0), pz(0) {}
    Vec3(const Vec3& o) : px(o.px), py(o.py), pz(o.pz) {}
    Vec3(const Vec2& xy, float z) : px(xy.x()), py(xy.y()), pz(z) {}
    Vec3(float x, float y, float z) : px(x), py(y), pz(z) {}
    
    /// \name Coordinate access
    ///@{
    
    float x() const { return px; }
    float y() const { return py; }
    float z() const { return pz; }
    float operator[](size_t idx) const { return idx == 0 ? px : (idx == 1 ? py : pz); }
    
    void setX(float x) { px = x; }
    void setY(float y) { py = y; }
    void setZ(float z) { pz = z; }
    void set(float x, float y, float z) { px = x; py = y; pz = z; }
    
    ///@}
    
    
    /// \name Operators
    ///@{
    
    Vec3& operator=(const Vec3& o) { px = o.px; py = o.py; pz = o.pz; return *this; }
    Vec3& operator+=(const Vec3& o) { px += o.px; py += o.py; pz += o.pz; return *this; }
    Vec3& operator-=(const Vec3& o) { px -= o.px; py -= o.py; pz -= o.pz; return *this; }
    Vec3& operator*=(float s) { px *= s; py *= s; pz *= s; return *this; }
    Vec3& operator/=(float s) { return (*this *= 1.0f/s); }
    
    const Vec3 operator+(const Vec3& o) const { return Vec3(*this) += o; }
    const Vec3 operator-(const Vec3& o) const { return Vec3(*this) -= o; }
    const Vec3 operator*(float s) const { return Vec3(*this) *= s; }
    const Vec3 operator/(float s) const { return Vec3(*this) /= s; }
    const Vec3 operator-() const { return Vec3(-px, -py, -pz); }
    
    ///@}
    
    
    /// \name Other operations
    ///@{
    
    /**
     * \brief Calculate the dot product (alias scalar product) of two vectors.
     */
    float dot(const Vec3& o) const { return px*o.px + py*o.py + pz*o.pz; }
    
    /**
     * \brief Calculate the square of the length of this vector.
     */
    float lengthSq() const { return px*px + py*py + pz*pz; }
    
    /**
     * \brief Calculate the length of this vector.
     */
    float length() const { return std::sqrt(lengthSq()); }
    
    /**
     * \brief Normalize this vector, i.e. make it length 1 without changing
     *      its direction.
     *
     * \return The vector's length prior to normalization.
     * \see normalized()
     */
    float normalize()
            { float l = length(); if (l < 1e-6f) return l; float il = 1.0f/l; px *= il; py *= il; pz *= il; return l; }
    
    /**
     * \brief Return a normalized version of this vector, i.e. one with length
     *      1, but with the same direction.
     *
     * Note that this method does not change the original vector, but returns
     * the normalized version as a copy.
     *
     * \param outLen If not null, this will be set to the vector's length prior
     *      to normalization
     * \return The normalized vector.
     * \see normalize()
     */
    const Vec3 normalized(float* outLen = nullptr) const
            { Vec3 r(*this); float l = r.normalize(); if (outLen) *outLen = l; return r; }
    
    /**
     * \brief Calculate the angle between this vector and another.
     *
     * \param o The other vector.
     * \return The angle between the vectors. Radians in range [0, pi]
     */
    float angle(const Vec3& o) const;
    
    ///@}
    
private:
    float px, py, pz;
};

inline Vec3 operator*(float s, const Vec3& v) { return v * s; }

}
