#pragma once

#include <vector>
#include <cmath>
#include <stdexcept>
#include <iostream>


class Vec2{
public:
    float x = 0;
    float y = 0;

    Vec2(){}
    Vec2(float xin, float yin);
    Vec2(const Vec2& vec);

    Vec2 operator + (const Vec2& rhs) const;
    Vec2 operator - (const Vec2& rhs) const;
    Vec2 operator * (float scale) const;
    Vec2 operator / (float scale) const;

    float operator *(const Vec2& rhs) const;    // dot product

    bool operator == (const Vec2& rhs) const;
    bool operator != (const Vec2& rhs) const;

    void operator += (const Vec2& rhs);
    void operator -= (const Vec2& rhs);
    void operator *= (float scale);
    void operator /= (float scale);

    friend std::ostream& operator << (std::ostream& os, const Vec2& vec);

    float distance (const Vec2& dest) const;
    float length() const;
    void normalize();
};