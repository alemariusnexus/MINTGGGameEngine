#pragma once

#include "Globals.h"


namespace MINTGGGameEngine
{

class Vec2
{
public:
    Vec2() : px(0), py(0) {}
    Vec2(const Vec2& o) : px(o.px), py(o.py) {}
    Vec2(float x, float y) : px(x), py(y) {}
    
    float x() const { return px; }
    float y() const { return py; }
    float operator[](size_t idx) const { return idx == 0 ? px : py; }
    
    void setX(float x) { px = x; }
    void setY(float y) { py = y; }
    void set(float x, float y) { px = x; py = y; }
    
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
    
    float dot(const Vec2& o) const { return px*o.px + py*o.py; }
    float lengthSq() const { return px*px + py*py; }
    float length() const { return sqrtf(lengthSq()); }
    float normalize() { float l = length(); if (l < 1e-6f) return l; float il = 1.0f/l; px *= il; py *= il; return l; }
    const Vec2 normalized() const { Vec2 r(*this); r.normalize(); return r; }
    
private:
    float px, py;
};

}
