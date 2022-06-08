#include "Ball.h"

Ball::Ball(float x0, float y0, float dirX, float dirY, float r, float speed):
	p{x0, y0}, dir{dirX, dirY }, r{r}, mass{r*r*3.14f}
{
    const float hypot = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    const float ratio = speed / hypot;
    dir.x *= ratio;
    dir.y *= ratio;
}


bool Ball::ProcessBorderCollision(int maxX, int maxY)
{
    switch (FindBorderCollision(maxX, maxY))
    {
    case BorderCollision::Left:
        dir.x *= -1.f;
        p.x += r - p.x;
        break;
    case BorderCollision::Right:
        p.x = maxX - 2 * r;
        dir.x *= -1.f;
        break;
    case BorderCollision::Upper:
        p.y += r - p.y;
        dir.y *= -1.f;
        break;
    case BorderCollision::Bottom:
        dir.y *= -1.f;
        p.y = maxY - 2 * r;
        break;
    case BorderCollision::None:
        return false;
    }
    return true;
}

bool Ball::Overlaps(const Ball& ball) const
{
    const float x = (p.x - ball.p.x) * (p.x - ball.p.x);
    const float y = (p.y - ball.p.y) * (p.y - ball.p.y);
    const float squared_r = (r + ball.r) * (r + ball.r);

    return x + y <= squared_r;
}

float Ball::DistanceBetweenCenters(const Ball& ball) const
{
    const float x = (p.x - ball.p.x) * (p.x - ball.p.x);
    const float y = (p.y - ball.p.y) * (p.y - ball.p.y);
    return std::sqrtf(x + y);
}

bool Ball::Displace(Ball& ball)
{
    if (!Overlaps(ball))
    {
        return false;
    }
    const float dist = DistanceBetweenCenters(ball);
    const float each_overlap = (dist - (r + ball.r)) * 0.5f;
    const float deltaX = each_overlap * (p.x - ball.p.x) / dist;
    const float deltaY = each_overlap * (p.y - ball.p.y) / dist;

    p.x -= deltaX;
    p.y -= deltaY;
    ball.p.x += deltaX;
    ball.p.y += deltaY;

    return true;
}

void Ball::Move(float deltaTime)
{
    p.x += dir.x * deltaTime;
    p.y += dir.y * deltaTime;
}

void Ball::Collide(Ball& ball)
{
    const float dist = DistanceBetweenCenters(ball);
    const float nx = (ball.p.x - p.x) / dist;
    const float ny = (ball.p.y - p.y) / dist;
    const float kx = (dir.x - ball.dir.x);
    const float ky = (dir.y - ball.dir.y);
    const float p = 2.0f * (nx * kx + ny * ky) / (mass + ball.mass);
    dir.x = dir.x - p * ball.mass * nx;
    dir.y = dir.y - p * ball.mass * ny;
    ball.dir.x = ball.dir.x + p * mass * nx;
    ball.dir.y = ball.dir.y + p * mass * ny;
}

Ball::circumscribed_square_points_t Ball::GetCircSquare() const
{
    circumscribed_square_points_t res{4};
    res[0] = { p.x - r, p.y - r };
    res[1] = { p.x + r, p.y - r };
    res[2] = { p.x + r, p.y + r };
    res[3] = { p.x - r, p.y + r };
    return res;
}

bool Ball::ContainedIn(const sf::Vector2f& s, const sf::Vector2f& e) const
{
    const auto circ_square = GetCircSquare();
    for (const auto& p : circ_square)
    {
        if (p.x >= s.x && p.x <= e.x && p.y >= s.y && p.y <= e.y)
        {
            return true;
        }
    }
    return false;
}

float Ball::GetRadius() const
{
    return r;
}

float Ball::GetX() const
{
    return p.x;
}

float Ball::GetY() const
{
    return p.y;
}

Ball::BorderCollision Ball::FindBorderCollision(int maxX, int maxY) const
{
    if (p.x - r <= 0)
    {
        return BorderCollision::Left;
    }
    else if (p.x + 2 * r >= maxX)
    {
        return BorderCollision::Right;
    }
    else if (p.y + 2 * r >= maxY)
    {
        return BorderCollision::Bottom;
    }
    else if (p.y <= r )
    {
        return BorderCollision::Upper;
    }
    return BorderCollision::None;
}
