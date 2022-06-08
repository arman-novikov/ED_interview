#include "Ball.h"
#include "MiddleAverageFilter.h"

#include <vector>
#include <unordered_map>
#include <iostream>

constexpr int WINDOW_X = 1024;
constexpr int WINDOW_Y = 768;
constexpr int MAX_BALLS = 300;
constexpr int MIN_BALLS = 299;

Math::MiddleAverageFilter<float,100> fpscounter;

static void draw_ball(sf::RenderWindow& window, const Ball& ball)
{
    sf::CircleShape gball;
    gball.setRadius(ball.GetRadius());
    gball.setPosition(ball.GetX(), ball.GetY());
    window.draw(gball);
}

static void draw_fps(sf::RenderWindow& window, float fps)
{
    char c[32];
    snprintf(c, 32, "FPS: %f", fps);
    std::string string(c);
    sf::String str(c);
    window.setTitle(str);
}

using ball_container_t = std::vector<Ball>;
using colliding_ball_container_t = std::unordered_map<Ball*, Ball*>;

static void validateCollisions(ball_container_t& balls,
    colliding_ball_container_t& colliding_balls)
{
    for (auto& ball : balls)
    {
        ball.ProcessBorderCollision(WINDOW_X, WINDOW_Y);
        for (auto& next : balls) {
            if (&next == &ball)
            {
                continue;
            }
            if (ball.Displace(next))
            {
                Ball* k, * v;
                if (&ball < &next)
                {
                    k = &ball;
                    v = &next;
                }
                else
                {
                    k = &next;
                    v = &ball;
                }
                auto iter = colliding_balls.find(k);
                if (iter == colliding_balls.end() || iter->second != v)
                {
                    colliding_balls[k] = v;
                }

            }
        }
    }
}

static ball_container_t random_balls()
{
    srand(static_cast<unsigned int>(time(NULL)));
    const auto ball_rand = rand() % (MAX_BALLS - MIN_BALLS) + MIN_BALLS;
    ball_container_t balls{};
    balls.reserve(ball_rand);

    // randomly initialize balls
    for (int i = 0; i < ball_rand; i++)
    {
        balls.emplace_back(
            static_cast<float>(rand() % (WINDOW_X - 50)),
            static_cast<float>(rand() % (WINDOW_Y - 50)),
            static_cast<float>((-5 + (rand() % 10)) / 3.f),
            static_cast<float>((-5 + (rand() % 10)) / 3.f),
            static_cast<float>(5 + rand() % 5),
            static_cast<float>(i == 0 ? 1000.f : 30.0f + rand() % 30)
        );
    }
    return balls;
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(WINDOW_X, WINDOW_Y), "ball collision demo");
    // window.setFramerateLimit(60);
    sf::Clock clock;
    ball_container_t balls = random_balls();
    float lastime = clock.restart().asSeconds();

    while (window.isOpen())
    {

        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        float current_time = clock.getElapsedTime().asSeconds();
        float deltaTime = current_time - lastime;
        fpscounter.push(1.0f / (current_time - lastime));
        lastime = current_time;

        colliding_ball_container_t colliding_balls{};

        for (auto& ball : balls)
        {
            ball.Move(deltaTime);
        }

        validateCollisions(balls, colliding_balls);

        for (auto& [b1, b2] : colliding_balls)
        {
           b1->Collide(*b2);
        }

        window.clear();
        for (auto& ball : balls)
        {
            draw_ball(window, ball);
        }

		draw_fps(window, fpscounter.getAverage());
		window.display();
    }
    return 0;
}
