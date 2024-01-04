#pragma once

#include "Vec2.h"
#include <SFML/Graphics.hpp>

enum SpecialAbilityType
{
   INSTANT_DAMAGE,
   STATIC_BLACK_HOLE,
   MOVING_BLACK_HOLE,
   AI_COMPANION, 
};

class cTransform
{
public:
    Vec2 pos      = {0,0};
    Vec2 velocity = {0,0};
    int rotation  = 0;
    int angle     = 0;

    cTransform(){}
    cTransform(const Vec2& p, const Vec2& v, int a, int r = 0)
                : pos(p), velocity(v), angle(a), rotation(r){}
};

class cCollision
{
public:
    float radius = 10.f;

    cCollision(){}
    cCollision(float r): radius(r){}
};

class cShape 
{
public:
    sf::CircleShape circle;

    cShape(float radius, int points, sf::Color fillColor, sf::Color outlineColor, float thickness)
            : circle(radius, points)
    {
        circle.setFillColor(fillColor);
        circle.setOutlineColor(outlineColor);
        circle.setOutlineThickness(thickness);
        circle.setOrigin(radius, radius);
    }
};

class cScore
{
public:
    int score;

    cScore(int s): score(s){}
};

class cInput
{
public:
    bool up                     = false;
    bool down                   = false;
    bool left                   = false;
    bool right                  = false;
    bool shooting               = false;
    bool specialAbility         = false;
    int  specialAbilityIndex    = 0;        // since we will have more than one special ability

    cInput(){}
};

class cLifespan
{
public:
    int remaining;
    int total;

    cLifespan(int r, int t): remaining(r), total(t){}
};

class cSpecialAbility
{
public:
    SpecialAbilityType type = SpecialAbilityType::INSTANT_DAMAGE;
    int cooldownPeriod = 1000;
    int elapsed = 0; 

    cSpecialAbility(){}

    cSpecialAbility(SpecialAbilityType sat, int cooldown, int elap): type(sat), cooldownPeriod(cooldown),
                     elapsed(elap){}
};

class cGravity
{
public:
    float gravity = 20.f;
    float rangeRadius = 50.f;

    cGravity(){}

    cGravity(float g, float r): gravity(g), rangeRadius(r){}
};