#include "Vec2.h"

Vec2::Vec2(float xin, float yin): x(xin), y(yin)
{

}

Vec2::Vec2(const Vec2& vec): x(vec.x), y(vec.y)
{

}

Vec2 Vec2::operator+(const Vec2& rhs) const 
{
    return Vec2(this->x + rhs.x, this->y + rhs.y);
} 

Vec2 Vec2::operator-(const Vec2& rhs) const 
{
    return Vec2(this->x - rhs.x, this->y - rhs.y);
} 

Vec2 Vec2::operator*(float scale) const
{
    return Vec2(this->x * scale, this->y * scale);
}

Vec2 Vec2::operator/(float scale) const 
{
    if(scale == 0)
        throw std::runtime_error("Vec2: Division By Zero");
    return Vec2(this->x / scale, this->y / scale);
}

float Vec2::operator*(const Vec2& rhs) const
{
    return (this->x * rhs.x + this->y + rhs.y);
}

bool Vec2::operator==(const Vec2& rhs) const 
{
    return ((this->x == rhs.x) && (this->y == rhs.y));
}

bool Vec2::operator!=(const Vec2& rhs) const 
{
    return ((this->x != rhs.x) || (this->y != rhs.y));
}

void Vec2::operator+=(const Vec2& rhs) 
{
    this->x += rhs.x;
    this->y += rhs.y;
}

std::ostream& operator<<(std::ostream& os, const Vec2& vec)
{
    os << vec.x << " "  << vec.y;
    return os;
}

void Vec2::operator-=(const Vec2& rhs)
{
    this->x -= rhs.x;
    this->y = rhs.y;
}

void Vec2::operator*=(float scale)
{
    this->x *= scale;
    this->y *= scale;
}

void Vec2::operator/=(float scale)
{
    if(scale == 0)
        throw std::runtime_error("Vec2: Division by Zero");
    this->x /= scale;
    this->y /= scale;
}

float Vec2::distance(const Vec2& dest) const
{
    float dx = dest.x - this->x;
    float dy = dest.y - this->y;

    return std::sqrt(dx * dx + dy * dy);
}

float Vec2::length() const
{
    return distance(Vec2(0,0));
}

void Vec2::normalize()
{
    float l = this->length();
    (*this) /= l;
}
