#pragma once

#include "SFML/Graphics.hpp"


class Ball
{
public:
    Ball(float x0, float y0, float dirX, float dirY, float r = 0.f, float speed = 0.f);
    float GetRadius() const;
    float GetX() const;
    float GetY() const;
    bool ProcessBorderCollision(int maxX, int maxY);
    bool Displace(Ball& ball);
    void Move(float deltaTime);
    void Collide(Ball& ball);
private:
    enum class BorderCollision {
        None, Left, Upper, Right, Bottom
    };
    BorderCollision FindBorderCollision(int maxX, int maxY) const;
    float DistanceBetweenCenters(const Ball& ball) const;
    bool Overlaps(const Ball& ball) const;

    sf::Vector2f p;
    sf::Vector2f dir;
    float r;
    float mass;
};
